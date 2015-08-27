
/* Functions to add,find and remove an entry from the memory list (head:_memory_head , lock:_memory_head_lock)
 */

#include "popcorn.h"

memory_t* _memory_head = NULL;

DEFINE_RAW_SPINLOCK(_memory_head_lock);

void add_memory_entry(memory_t* entry) {
	memory_t* curr;
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_memory_head_lock,flags);

	if (!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);
}

int add_memory_entry_with_check(memory_t* entry) {
	memory_t* curr;
	memory_t* prev;
	unsigned long flags;

	if (!entry) {
		return -1;
	}

	raw_spin_lock_irqsave(&_memory_head_lock,flags);

	if (!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		prev= NULL;
		do{
			if ( (curr->tgroup_home_cpu == entry->tgroup_home_cpu
						&& curr->tgroup_home_id == entry->tgroup_home_id)) {

				raw_spin_unlock_irqrestore(&_memory_head_lock,flags);
				return -1;

			}
			prev=curr;
			curr= curr->next;
		}
		while (curr->next != NULL) ;

		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);
	return 0;
}

memory_t* find_memory_entry(int cpu, int id) {
	memory_t* curr = NULL;
	memory_t* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock,flags);
	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);
	return ret;
}

struct mm_struct* find_dead_mapping(int cpu, int id) {
	memory_t* curr = NULL;
	struct mm_struct* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock,flags);
	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr->mm;
			break;
		}
		curr = curr->next;
	}


	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);

	return ret;
}

memory_t* find_and_remove_memory_entry(int cpu, int id) {
	memory_t* curr = NULL;
	memory_t* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock,flags);

	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	if (ret) {
		if (_memory_head == ret) {
			_memory_head = ret->next;
		}

		if (ret->next) {
			ret->next->prev = ret->prev;
		}

		if (ret->prev) {
			ret->prev->next = ret->next;
		}

		ret->prev = NULL;
		ret->next = NULL;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);

	return ret;
}

void remove_memory_entry(memory_t* entry) {
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_memory_head_lock,flags);

	if (_memory_head == entry) {
		_memory_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_memory_head_lock,flags);
}
