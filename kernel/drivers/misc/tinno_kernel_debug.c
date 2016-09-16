/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#define DEV_NAME "TINNO,kernel"
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/export.h>
#include <linux/syscore_ops.h>
#include <linux/ftrace.h>
#include <linux/rtc.h>


struct work_struct w_work;
static void work_func(struct work_struct *work)
{
         
	struct task_struct *p;
	struct timespec ts;
	struct rtc_time tm;	
	
	//printk("----- work_func-----\n");
	if(nr_iowait() > 5)
	{
		printk("----- iowait log start -----\n");
		for_each_process(p)
		{
			if(p->in_iowait == 1)
			{
				getnstimeofday(&ts);
				rtc_time_to_tm(ts.tv_sec, &tm);
				printk("!!!WARNING!!! iowait %ld, process pid: %d, process name: %s, %d-%02d-%02d %02d:%02d:%02d.%09lu UTC\n",nr_iowait(),p->pid, p->comm,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
			}
		}
		printk("----- iowait log end -----\n");
	}
	 msleep(1000);

	 schedule_work(&w_work);   
}
EXPORT_SYMBOL(w_work);

static __init int work_init(void)
{
	INIT_WORK(&w_work,work_func);
	schedule_work(&w_work);
	return 0;    
}
static void work_exit(void)
{
	cancel_work_sync(&w_work);
}
module_init(work_init);
module_exit(work_exit);


MODULE_DESCRIPTION(DEV_NAME);
MODULE_LICENSE("GPL v2");
