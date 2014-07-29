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
#include <linux/cpufreq.h>

static struct work_struct offline_cpu_work;
static struct work_struct online_cpu_work;

static bool suspend = false;
static bool suspend_max_freq = 800000;
module_param(suspend_max_freq, uint, 0644);

static int cpufreq_callback(struct notifier_block *nfb,
		unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;

	if (event != CPUFREQ_ADJUST)
		return 0;

	cpufreq_verify_within_limits(policy,
		policy->cpuinfo.min_freq,
		suspend ? suspend_max_freq : policy->cpuinfo.max_freq);

	return 0;
}

static struct notifier_block cpufreq_notifier_block = {
	.notifier_call = cpufreq_callback,
};

static void suspend_work_fn(struct work_struct *work)
{
	int cpu;

	suspend = true;

	for_each_online_cpu(cpu)
	{
		cpufreq_update_policy(cpu);

		if (!cpu)
			continue;

		cpu_down(cpu);
	}
}

static void resume_work_fn(struct work_struct *work)
{
	int cpu;

	suspend = false;

	for_each_online_cpu(cpu)
	{
		cpufreq_update_policy(cpu);

		if (!cpu)
			continue;

		cpu_up(cpu);
	}
}

static void u8500_hotplug_suspend(struct early_suspend *handler)
{
	schedule_work(&suspend_work);
}

static void u8500_hotplug_resume(struct early_suspend *handler)
{
	schedule_work(&resume_work);
}

static struct early_suspend u8500_hotplug_early_suspend = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = u8500_hotplug_suspend,
	.resume = u8500_hotplug_resume,
};

static int u8500_hotplug_init(void)
{
	INIT_WORK(&suspend_work, suspend_work_fn);
	INIT_WORK(&resume_work, resume_work_fn);

	register_early_suspend(&u8500_hotplug_early_suspend);

	cpufreq_register_notifier(&cpufreq_notifier_block, CPUFREQ_POLICY_NOTIFIER);
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
