// SPDX-License-Identifier: GPL-2.0
/*
 * hello.c - "Hello, world!" Linux kernel module (lab 6, debugging).
 *
 * Based on the module from lab 4 with two intentional bugs:
 *  - BUG_ON() is used instead of returning -EINVAL for count > 10;
 *  - for the 5th greeting kmalloc() is "forced" to return NULL, so the
 *    list element is dereferenced through a NULL pointer (kernel oops).
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/bug.h>

MODULE_AUTHOR("Vladyslav Musiienko, IO-44");
MODULE_DESCRIPTION("Hello, world with BUG_ON() and kmalloc() fault injection");
MODULE_LICENSE("Dual BSD/GPL");

static uint count = 1;
module_param(count, uint, 0444);
MODULE_PARM_DESC(count,
		 "How many times to print \"Hello, world!\" (default 1; >10 - BUG_ON)");

/* Number of the greeting for which kmalloc() "returns" NULL */
#define HELLO_FAIL_AT 5

struct hello_event {
	struct list_head list;
	ktime_t time;
};

static LIST_HEAD(hello_list);

static int __init hello_init(void)
{
	uint i;

	BUG_ON(count > 10);

	if (count == 0 || (count >= 5 && count <= 10))
		pr_warn("count=%u is 0 or between 5 and 10, continuing\n",
			count);

	for (i = 1; i <= count; i++) {
		struct hello_event *ev;

		/* Fault injection: as if kmalloc() returned 0 */
		if (i == HELLO_FAIL_AT)
			ev = NULL;
		else
			ev = kmalloc(sizeof(*ev), GFP_KERNEL);

		ev->time = ktime_get();
		list_add_tail(&ev->list, &hello_list);
		pr_info("Hello, world!\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	struct hello_event *ev, *tmp;

	list_for_each_entry_safe(ev, tmp, &hello_list, list) {
		pr_info("event time: %lld ns\n",
			(long long)ktime_to_ns(ev->time));
		list_del(&ev->list);
		kfree(ev);
	}
	pr_info("Goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);
