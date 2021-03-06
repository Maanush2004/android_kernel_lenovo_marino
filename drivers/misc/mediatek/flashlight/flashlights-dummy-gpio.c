/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>

#include "flashlight-core.h"
#include "flashlight-dt.h"
  static unsigned int f_duty = 7;  //modify lct tianyaping 100mA 20170407
 module_param(f_duty,int,0644);
  static unsigned int t_duty = 5;  //modify lct tianyaping  75mA 20170407
 module_param(t_duty,int,0644);
   static unsigned int count = 8;
 module_param(count,int,0644);
    static unsigned int sleep = 50;
 module_param(sleep,int,0644);
/* define device tree */
/* TODO: modify temp device tree name */
#ifndef DUMMY_DTNAME
#define DUMMY_DTNAME "mediatek,flashlights_dummy_gpio"
#endif

/* TODO: define driver name */
#define DUMMY_NAME "flashlights-dummy-gpio"

/* define registers */
/* TODO: define register */
enum
{
	e_DutyNum = 16,
};
static int flashDuty[e_DutyNum]=     {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1}; //lenovo.sw huangsh4 change for 1A flash current
/* define mutex and work queue */
static DEFINE_MUTEX(dummy_mutex);
static struct work_struct dummy_work;
static int g_duty=-1;
/* define pinctrl */
/* TODO: define pinctrl */
#define DUMMY_PINCTRL_PIN_XXX 0
#define DUMMY_PINCTRL_PINSTATE_LOW 0
#define DUMMY_PINCTRL_PINSTATE_HIGH 1
#define DUMMY_PINCTRL_STATE_XXX_HIGH "xxx_high"
#define DUMMY_PINCTRL_STATE_XXX_LOW  "xxx_low"
static struct pinctrl *dummy_pinctrl;
static struct pinctrl_state *dummy_xxx_high;
static struct pinctrl_state *dummy_xxx_low;

/* define usage count */
static int use_count;



/******************************************************************************
 * Pinctrl configuration
 *****************************************************************************/
static int dummy_pinctrl_init(struct platform_device *pdev)
{
	int ret = 0;

	/* get pinctrl */
	dummy_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(dummy_pinctrl)) {
		pr_err("Failed to get flashlight pinctrl.\n");
		ret = PTR_ERR(dummy_pinctrl);
	}

	/* TODO: Flashlight XXX pin initialization */
	dummy_xxx_high = pinctrl_lookup_state(dummy_pinctrl, DUMMY_PINCTRL_STATE_XXX_HIGH);
	if (IS_ERR(dummy_xxx_high)) {
		pr_err("Failed to init (%s)\n", DUMMY_PINCTRL_STATE_XXX_HIGH);
		ret = PTR_ERR(dummy_xxx_high);
	}
	dummy_xxx_low = pinctrl_lookup_state(dummy_pinctrl, DUMMY_PINCTRL_STATE_XXX_LOW);
	if (IS_ERR(dummy_xxx_low)) {
		pr_err("Failed to init (%s)\n", DUMMY_PINCTRL_STATE_XXX_LOW);
		ret = PTR_ERR(dummy_xxx_low);
	}

	return ret;
}

static int dummy_pinctrl_set(int pin, int state)
{
	int ret = 0;

	if (IS_ERR(dummy_pinctrl)) {
		pr_err("pinctrl is not available\n");
		return -1;
	}

	switch (pin) {
	case DUMMY_PINCTRL_PIN_XXX:
		if (state == DUMMY_PINCTRL_PINSTATE_LOW && !IS_ERR(dummy_xxx_low))
			pinctrl_select_state(dummy_pinctrl, dummy_xxx_low);
		else if (state == DUMMY_PINCTRL_PINSTATE_HIGH && !IS_ERR(dummy_xxx_high))
			pinctrl_select_state(dummy_pinctrl, dummy_xxx_high);
		else
			pr_err("set err, pin(%d) state(%d)\n", pin, state);
		break;
	default:
		pr_err("set err, pin(%d) state(%d)\n", pin, state);
		break;
	}
	pr_debug("pin(%d) state(%d)\n", pin, state);

	return ret;
}


/******************************************************************************
 * dummy operations
 *****************************************************************************/
/* flashlight enable function */
static int dummy_enable(int level)
{
	int pin = 0;
	int i =0;
	if(g_duty== 0)//torch
	{
	for (i =1;i<=flashDuty[t_duty];i++)
	{
		//dummy_pinctrl_set(pin, 1);
		udelay(count);
		pr_debug("Suny FL_enable torch i = %d ,g_duty=%d line=%d\n",i,g_duty,__LINE__);
		 dummy_pinctrl_set(pin, 0);
		  udelay(sleep);
		 dummy_pinctrl_set(pin, 1);
			}
	} else {
	for (i =1;i<=flashDuty[f_duty];i++)
	{
		//dummy_pinctrl_set(pin, 1);
		udelay(count);
		pr_debug("Suny FL_enable flash i = %d ,g_duty=%d line=%d\n",i,g_duty,__LINE__);
		 dummy_pinctrl_set(pin, 0);
		  udelay(sleep);
		 dummy_pinctrl_set(pin, 1);
			}
		}
		pr_debug("Suny FL_enable flash g_duty=%d line=%d\n",g_duty,__LINE__);
	/* TODO: wrap enable function */

	return 0;
}

/* flashlight disable function */
static int dummy_disable(void)
{
	int pin = 0, state = 0;

	/* TODO: wrap disable function */

	return dummy_pinctrl_set(pin, state);
}

/* set flashlight level */
static int dummy_set_level(int level)
{
	int pin = 0, state = 0;

	/* TODO: wrap set level function */
	g_duty=level;
	pr_debug(" Suny FL_dim_duty=%d line=%d\n",level,__LINE__);
	return dummy_pinctrl_set(pin, state);
}

/* flashlight init */
static int dummy_init(void)
{
	int pin = 0, state = 0;

	/* TODO: wrap init function */

	return dummy_pinctrl_set(pin, state);
}

/* flashlight uninit */
static int dummy_uninit(void)
{
	int pin = 0, state = 0;

	/* TODO: wrap uninit function */

	return dummy_pinctrl_set(pin, state);
}

/******************************************************************************
 * Timer and work queue
 *****************************************************************************/
static struct hrtimer fl_timer;
static unsigned int fl_timeout_ms;

static void dummy_work_disable(struct work_struct *data)
{
	pr_debug("work queue callback\n");
	dummy_disable();
}

static enum hrtimer_restart fl_timer_func(struct hrtimer *timer)
{
	schedule_work(&dummy_work);
	return HRTIMER_NORESTART;
}
static void sys_torch_brightness_set(struct led_classdev *led_cdev,enum led_brightness brightness)
{
	printk("--[%s][%d]------%d--\n",__FUNCTION__,__LINE__,brightness);
	if (brightness ==1)
	dummy_enable(brightness);
	else
	dummy_disable();
	return;
}

/******************************************************************************
 * Flashlight operations
 *****************************************************************************/
static int dummy_ioctl(unsigned int cmd, unsigned long arg)
{
	struct flashlight_user_arg *fl_arg;
	int ct_index;
	ktime_t ktime;

	fl_arg = (struct flashlight_user_arg *)arg;
	ct_index = fl_get_ct_index(fl_arg->ct_id);
	if (flashlight_ct_index_verify(ct_index)) {
		fl_err("Failed with error index\n");
		return -EINVAL;
	}

	switch (cmd) {
	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		fl_dbg("FLASH_IOC_SET_TIME_OUT_TIME_MS(%d): %d\n",
				ct_index, (int)fl_arg->arg);
		fl_timeout_ms = fl_arg->arg;
		break;

	case FLASH_IOC_SET_DUTY:
		fl_dbg("FLASH_IOC_SET_DUTY(%d): %d\n",
				ct_index, (int)fl_arg->arg);
		dummy_set_level(fl_arg->arg);
		break;

	case FLASH_IOC_SET_STEP:
		fl_dbg("FLASH_IOC_SET_STEP(%d): %d\n",
				ct_index, (int)fl_arg->arg);
		break;

	case FLASH_IOC_SET_ONOFF:
		fl_dbg("FLASH_IOC_SET_ONOFF(%d): %d\n",
				ct_index, (int)fl_arg->arg);
		if (fl_arg->arg == 1) {
			if (fl_timeout_ms) {
				ktime = ktime_set(fl_timeout_ms / 1000,
						(fl_timeout_ms % 1000) * 1000000);
				hrtimer_start(&fl_timer, ktime, HRTIMER_MODE_REL);
			}
			dummy_enable((int)fl_arg->arg);
		} else {
			dummy_disable();
			hrtimer_cancel(&fl_timer);
		}
		break;
	default:
		fl_info("No such command and arg(%d): (%d, %d)\n",
				ct_index, _IOC_NR(cmd), (int)fl_arg->arg);
		return -ENOTTY;
	}

	return 0;
}

static int dummy_open(void *pArg)
{
	/* Actual behavior move to set driver function since power saving issue */
	return 0;
}

static int dummy_release(void *pArg)
{
	/* uninit chip and clear usage count */
	mutex_lock(&dummy_mutex);
	use_count--;
	if (!use_count)
		dummy_uninit();
	if (use_count < 0)
		use_count = 0;
	mutex_unlock(&dummy_mutex);

	fl_dbg("Release: %d\n", use_count);

	return 0;
}

static int dummy_set_driver(int scenario)
{
	/* init chip and set usage count */
	mutex_lock(&dummy_mutex);
	if (!use_count)
		dummy_init();
	use_count++;
	mutex_unlock(&dummy_mutex);

	fl_dbg("Set driver: %d\n", use_count);

	return 0;
}

static ssize_t dummy_strobe_store(struct flashlight_arg arg)
{
	dummy_set_driver(FLASHLIGHT_SCENARIO_CAMERA);
	dummy_set_level(arg.level);
	dummy_enable(arg.level);
	msleep(arg.dur);
	dummy_disable();
	dummy_release(NULL);

	return 0;
}

static struct flashlight_operations dummy_ops = {
	dummy_open,
	dummy_release,
	dummy_ioctl,
	dummy_strobe_store,
	dummy_set_driver
};


/******************************************************************************
 * Platform device and driver
 *****************************************************************************/
static int dummy_chip_init(void)
{
	/* NOTE: Chip initialication move to "set driver" operation for power saving issue.
	 * dummy_init();
	 */

	return 0;
}

static int dummy_probe(struct platform_device *dev)
{
	int err;
	static struct led_classdev torch_cdev;
	fl_dbg("Probe start.\n");

	/* init pinctrl */
	if (dummy_pinctrl_init(dev)) {
		fl_dbg("Failed to init pinctrl.\n");
		err = -EFAULT;
		goto err;
	}

	/* init work queue */
	INIT_WORK(&dummy_work, dummy_work_disable);

	/* init timer */
	hrtimer_init(&fl_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	fl_timer.function = fl_timer_func;
	fl_timeout_ms = 100;

	/* init chip hw */
	dummy_chip_init();

	/* register flashlight operations */
	if (flashlight_dev_register(DUMMY_NAME, &dummy_ops)) {
		err = -EFAULT;
		goto err;
	}

	/* clear usage count */
	use_count = 0;
	
	    //create some sys node.
   	 torch_cdev.name="torch";
   	 torch_cdev.brightness_set=sys_torch_brightness_set;
  	 led_classdev_register(&(dev->dev),&torch_cdev);
	fl_dbg("Probe done.\n");

	return 0;
err:
	return err;
}

static int dummy_remove(struct platform_device *dev)
{
	fl_dbg("Remove start.\n");

	/* flush work queue */
	flush_work(&dummy_work);

	/* unregister flashlight operations */
	flashlight_dev_unregister(DUMMY_NAME);

	fl_dbg("Remove done.\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id dummy_gpio_of_match[] = {
	{.compatible = DUMMY_DTNAME},
	{},
};
MODULE_DEVICE_TABLE(of, dummy_gpio_of_match);
#else
static struct platform_device dummy_gpio_platform_device[] = {
	{
		.name = DUMMY_NAME,
		.id = 0,
		.dev = {}
	},
	{}
};
MODULE_DEVICE_TABLE(platform, dummy_gpio_platform_device);
#endif

static struct platform_driver dummy_platform_driver = {
	.probe = dummy_probe,
	.remove = dummy_remove,
	.driver = {
		.name = DUMMY_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = dummy_gpio_of_match,
#endif
	},
};

static int __init flashlight_dummy_init(void)
{
	int ret;

	pr_debug("Init start.\n");

#ifndef CONFIG_OF
	ret = platform_device_register(&dummy_gpio_platform_device);
	if (ret) {
		pr_err("Failed to register platform device\n");
		return ret;
	}
#endif

	ret = platform_driver_register(&dummy_platform_driver);
	if (ret) {
		pr_err("Failed to register platform driver\n");
		return ret;
	}

	pr_debug("Init done.\n");

	return 0;
}

static void __exit flashlight_dummy_exit(void)
{
	pr_debug("Exit start.\n");

	platform_driver_unregister(&dummy_platform_driver);

	pr_debug("Exit done.\n");
}

module_init(flashlight_dummy_init);
module_exit(flashlight_dummy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Simon Wang <Simon-TCH.Wang@mediatek.com>");
MODULE_DESCRIPTION("MTK Flashlight DUMMY GPIO Driver");

