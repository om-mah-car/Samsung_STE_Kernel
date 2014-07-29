/*
 * arch/arm/mach-ux500/u8500_hotplug.c
 *
 * Copyright (c) 2014, Zhao Wei Liew <zhaoweiliew@gmail.com>. 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/earlysuspend.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/init.h>

static struct work_struct offline_cpu_work;
static struct work_struct online_cpu_work;

static void offline_cpu_work_fn(struct work_struct *work)
{
	cpu_down(1);
}

static void online_cpu_work_fn(struct work_struct *work)
{
	cpu_up(1);
}

static void u8500_hotplug_suspend(struct early_suspend *handler)
{
	schedule_work(&offline_cpu_work);
}

static void u8500_hotplug_resume(struct early_suspend *handler)
{
	schedule_work(&online_cpu_work);
}

static struct early_suspend u8500_hotplug_early_suspend = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = u8500_hotplug_suspend,
	.resume = u8500_hotplug_resume,
};

static int u8500_hotplug_init(void)
{
	INIT_WORK(&offline_cpu_work, offline_cpu_work_fn);
	INIT_WORK(&online_cpu_work, online_cpu_work_fn);

	register_early_suspend(&u8500_hotplug_early_suspend);
}

late_initcall(u8500_hotplug_init);

static void u8500_hotplug_exit(void)
{
	unregister_early_suspend(&u8500_hotplug_early_suspend);
}

module_exit(u8500_hotplug_exit);

MODULE_AUTHOR("Zhao Wei Liew <zhaoweiliew@gmail.com>");
MODULE_DESCRIPTION("Hotplug driver for U8500");
MODULE_LICENSE("GPL");
