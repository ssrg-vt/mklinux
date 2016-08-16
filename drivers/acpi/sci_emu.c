/*
 *  Code to emulate SCI interrupt for Hotplug node insertion/removal
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <acpi/acpi_drivers.h>

#include "internal.h"

#include "acpica/accommon.h"
#include "acpica/acnamesp.h"
#include "acpica/acevents.h"

#define _COMPONENT		ACPI_SYSTEM_COMPONENT
ACPI_MODULE_NAME("sci_emu");
MODULE_LICENSE("GPL");

static struct dentry *sci_notify_dentry;

static void sci_notify_client(char *acpi_name, u32 event)
{
	struct acpi_namespace_node *node;
	acpi_status status, status1;
	acpi_handle hlsb, hsb;
	union acpi_operand_object *obj_desc;

	status = acpi_get_handle(NULL, "\\_SB", &hsb);
	status1 = acpi_get_handle(hsb, acpi_name, &hlsb);
	if (ACPI_FAILURE(status) || ACPI_FAILURE(status1)) {
		pr_err(PREFIX
	"acpi getting handle to <\\_SB.%s> failed inside notify_client\n",
			acpi_name);
		return;
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		pr_err(PREFIX "Acquiring acpi namespace mutext failed\n");
		return;
	}

	node = acpi_ns_validate_handle(hlsb);
	if (!node) {
		acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
		pr_err(PREFIX "Mapping handle to node failed\n");
		return;
	}

	/*
	 * Check for internal object and make sure there is a handler
	 * registered for this object
	 */
	obj_desc = acpi_ns_get_attached_object(node);
	if (obj_desc) {
		if (((event <= ACPI_MAX_SYS_NOTIFY) &&
		    obj_desc->common_notify.notify_list[ACPI_SYSTEM_HANDLER_LIST]) ||
		    (((event > ACPI_MAX_SYS_NOTIFY) &&
		    obj_desc->common_notify.notify_list[ACPI_DEVICE_HANDLER_LIST]))) {
			/*
			 * Release the lock and queue the item for later
			 * exectuion
			 */
			acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
			status = acpi_ev_queue_notify_request(node, event);
			if (ACPI_FAILURE(status))
				pr_err(PREFIX "acpi_ev_queue_notify_request failed\n");
			else
				pr_info(PREFIX "Notify event is queued\n");
			return;
		}
	} else {
		pr_info(PREFIX "Notify handler not registered for this device\n");
	}

	acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
	return;
}

static ssize_t sci_notify_write(struct file *file, const char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	u32 event;
	char *name1 = NULL;
	char *name2 = NULL;
	const char *delim = " ";
	char *temp_buf = NULL;
	char *temp_buf_addr = NULL;

	temp_buf = kmalloc(count+1, GFP_ATOMIC);
	if (!temp_buf) {
		pr_warn(PREFIX "sci_notify_wire: Memory allocation failed\n");
		return count;
	}
	temp_buf[count] = '\0';
	temp_buf_addr = temp_buf;
	if (copy_from_user(temp_buf, user_buf, count))
		goto out;

	name1 = strsep(&temp_buf, delim);
	name2 = strsep(&temp_buf, delim);

	if (name1 && name2) {
		ssize_t ret;
		unsigned long val;

		ret = kstrtoul(name2, 10, &val);
		if (ret) {
			pr_warn(PREFIX "unknown event\n");
			goto out;
		}

		event = (u32)val;
	} else {
		pr_warn(PREFIX "unknown device\n");
		goto out;
	}

	pr_info(PREFIX "ACPI device name is <%s>, event code is <%d>\n",
		name1, event);

	sci_notify_client(name1, event);

out:
	kfree(temp_buf_addr);
	return count;
}

static const struct file_operations sci_notify_fops = {
	.write = sci_notify_write,
};

static int __init acpi_sci_notify_init(void)
{
	if (acpi_debugfs_dir == NULL)
		return -ENOENT;

	sci_notify_dentry = debugfs_create_file("sci_notify", S_IWUSR,
				acpi_debugfs_dir, NULL, &sci_notify_fops);
	if (sci_notify_dentry == NULL)
		return -ENODEV;

	return 0;
}

device_initcall(acpi_sci_notify_init);
