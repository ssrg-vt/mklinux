
#include "popcorn.h"

count_answers_t* _count_head = NULL;

DEFINE_RAW_SPINLOCK(_count_head_lock);

void push_data(data_header_t** phead, raw_spinlock_t* spinlock,
		data_header_t* entry) {
	unsigned long flags;
	data_header_t* head;

	if (!entry) {
		return;
	}
	entry->prev = NULL;

	raw_spin_lock_irqsave(spinlock, flags);

	head= *phead;

	if (!head) {
		entry->next = NULL;
		*phead = entry;
	} else {
		entry->next = head;
		head->prev = entry;
		*phead = entry;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);
}

data_header_t* pop_data(data_header_t** phead, raw_spinlock_t* spinlock) {
	data_header_t* ret = NULL;
	data_header_t* head;
	unsigned long flags;

	raw_spin_lock_irqsave(spinlock, flags);

	head= *phead;
	if (head) {
		ret = head;
		if (head->next){
			head->next->prev = NULL;
		}
		*phead = head->next;
		ret->next = NULL;
		ret->prev = NULL;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}

int count_data(data_header_t** phead, raw_spinlock_t* spinlock) {
	int ret = 0;
	unsigned long flags;
	data_header_t* head;
	data_header_t* curr;

	raw_spin_lock_irqsave(spinlock, flags);

	head= *phead;

	curr = head;
	while (curr) {
		ret++;
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}

/* Functions to add,find and remove an entry from the count list (head:_count_head , lock:_count_head_lock)
 */

void add_count_entry(count_answers_t* entry) {
	count_answers_t* curr;
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	if (!_count_head) {
		_count_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _count_head;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);
}

count_answers_t* find_count_entry(int cpu, int id) {
	count_answers_t* curr = NULL;
	count_answers_t* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	curr = _count_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);

	return ret;
}

void remove_count_entry(count_answers_t* entry) {

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	if (_count_head == entry) {
		_count_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);
}
