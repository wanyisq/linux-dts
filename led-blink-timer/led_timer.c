#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/timer.h>
#include <linux/semaphore.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_NAME "timer_led"
#define LED_NUM	1
#define TIMER_CNT 1
#define CLOSE_CMD	(0x01)
#define OPEN_CMD	(0x02)
#define SETPERIOD	(0x03)

#define LEDON	1
#define LEDOFF	0

struct timer_dev
{
	int major;
	int minor;
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *node;
	int led_gpio;
	struct timer_list timer;
	int timeperiod;
	spinlock_t lock;
};
struct timer_dev timerdev;


static int led_init(void)
{
	int ret = 0;

	timerdev.node = of_find_node_by_path("/wan-leds");
	if(timerdev.node == NULL){
		printk("find node none\n");
		return -EINVAL;
	}else
	{
		printk("fine node\n");
	}
	
	timerdev.led_gpio = of_get_named_gpio(timerdev.node, "leds0", 0);
	if(timerdev.led_gpio < 0){
		printk("timerdev gpio not found\n");
		return -EINVAL;
	}
	printk("timerdev gpio is %d\n", timerdev.led_gpio);
	ret = gpio_direction_output(timerdev.led_gpio, 0);
	if(ret < 0){
		printk("can't set timerdev gpio\n");
	}
	return ret;
}
static ssize_t timer_open(struct inode *node, struct file *filp)
{
	int ret;

	filp->private_data = &timerdev;
	timerdev.timeperiod = 1000;
	ret = led_init();
	if(ret < 0){
		return ret;
	}
	return 0;
}

static long timer_unlock_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct timer_dev *dev = (struct timer_dev *)filp->private_data;
	int timerperiod = 0;
	unsigned long flags = 0;

	switch(cmd)
	{
		case CLOSE_CMD:
			del_timer_sync(&dev->timer);
			printk("timer ioctl close timer\n");
		break;

		case OPEN_CMD:
			spin_lock_irqsave(&dev->lock, flags);
			timerperiod = dev->timeperiod;
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(timerperiod));
			printk("timer ioctl open timer\n");
		break;

		case SETPERIOD:
			spin_lock_irqsave(&dev->lock, flags);
			dev->timeperiod = arg;
			printk("input timer period is %ld\n", arg);
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(arg));
			printk("timer ioctl setperiod\n");
		break;

		default: 
		break;
	}
	return 0;
} 
static struct file_operations timer_ops = {
	.owner = THIS_MODULE,
	.open = timer_open,
	.unlocked_ioctl = timer_unlock_ioctl,
};

void timer_callback(unsigned long arg)
{
	struct timer_dev *dev = (struct timer_dev *)arg;
	static int status = 0;

	status = !status;
	gpio_set_value(dev->led_gpio, status);
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod));
	printk("timer callback timestamp %ld, period is %d\n", jiffies, dev->timeperiod);
}

static int __init timer_init(void)
{
	spin_lock_init(&timerdev.lock);
	if(timerdev.major){
		timerdev.devid = MKDEV(timerdev.major, 0);
		register_chrdev_region(timerdev.devid, TIMER_CNT, DRIVER_NAME);
	}else
	{
		alloc_chrdev_region(&timerdev.devid, 0, TIMER_CNT, DRIVER_NAME);
		timerdev.major = MAJOR(timerdev.devid);
		timerdev.minor = MINOR(timerdev.devid);
	}
	printk("timerdev major %d, minor %d\n", timerdev.major, timerdev.minor);

	timerdev.cdev.owner = THIS_MODULE;
	cdev_init(&timerdev.cdev, &timer_ops);
	cdev_add(&timerdev.cdev, timerdev.devid, TIMER_CNT);

	timerdev.class = class_create(THIS_MODULE, "timerdev class");
	if(IS_ERR(timerdev.class)){
		printk("timerdev class %d\n", IS_ERR(timerdev.class));
		return PTR_ERR(timerdev.class);
	}
	timerdev.device = device_create(timerdev.class, NULL, timerdev.devid, NULL, DRIVER_NAME);
	if(IS_ERR(timerdev.device)){
		return PTR_ERR(timerdev.device);
	}

	init_timer(&timerdev.timer);
	timerdev.timer.function = timer_callback;
	timerdev.timer.data = (unsigned long)&timerdev;
	printk("timerdev init ok\n");
	return 0;
}
static void __exit timer_exit(void)
{
	del_timer_sync(&timerdev.timer);
	cdev_del(&timerdev.cdev);
	unregister_chrdev_region(timerdev.devid, TIMER_CNT);
	device_destroy(timerdev.class, timerdev.devid);
	class_destroy(timerdev.class);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("wanyisq");
MODULE_DESCRIPTION("imx6ull timerdev driver by pinctr");
