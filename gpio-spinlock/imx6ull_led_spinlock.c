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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_NAME "leds"
#define LED_NUM	1

#define LEDON	1
#define LEDOFF	0

struct led_dev
{
	int major;
	int minor;
	dev_t devid;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *node;
	int led_gpio;
	int dev_stats;
	spinlock_t lock;
};
struct led_dev led = {0};

static ssize_t led_open(struct inode *node, struct file *filp)
{
	unsigned long flag = 0;
	spin_lock_irqsave(&led.lock, flag);
	if(led.dev_stats){
		spin_unlock_irqrestore(&led.lock, flag);
		printk("led open dev stats failed\n");
		return -EBUSY;
	}
	led.dev_stats++;
	spin_unlock_irqrestore(&led.lock, flag);
	filp->private_data = &led;
	return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t count, loff_t *offt)
{
	return 0;
}
static ssize_t led_write(struct file *filp, const char __user *buf, size_t count, loff_t *offt)
{
	int ret = 0;
	unsigned char databuf[1];
	unsigned char ledstatus = 0;
	struct led_dev *dev = filp->private_data;
	
	ret = copy_from_user(databuf, buf, count);
	if(ret < 0 ){
		printk("kernel write failed\n");
		return -EFAULT;
	}

	ledstatus = databuf[0];
	if(ledstatus == LEDON){
		gpio_set_value(dev->led_gpio, 0);
	}else
	{
		gpio_set_value(dev->led_gpio, 1);
	}
	return 0;
}
static int led_release(struct inode *node, struct file *filp)
{
	struct led_dev *prv_led = filp->private_data;
	unsigned long flag = 0;
	
	spin_lock_irqsave(&led.lock, flag);
	printk("led relsase led stats %d\n", led.dev_stats);
	if(led.dev_stats){
		led.dev_stats--;
	}
	spin_unlock_irqrestore(&led.lock, flag);
	return 0;
}
static struct file_operations led_ops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_release,
};

static int __init leds_init(void)
{
	int ret = 0;
	spin_lock_init(&led.lock);
	led.node = of_find_node_by_path("/leds");
	if(led.node == NULL){
		printk("find node none\n");
		return -EINVAL;
	}else
	{
		printk("fine node\n");
	}
	
	led.led_gpio = of_get_named_gpio(led.node, "gpios", 0);
	if(led.led_gpio < 0){
		printk("led gpio not found\n");
		return -EINVAL;
	}
	printk("led gpio is %d\n", led.led_gpio);
	ret = gpio_direction_output(led.led_gpio, 0);
	if(ret < 0){
		printk("can't set led gpio\n");
	}

	if(led.major){
		led.devid = MKDEV(led.major, 0);
		register_chrdev_region(led.devid, LED_NUM, DRIVER_NAME);
	}else
	{
		alloc_chrdev_region(&led.devid, 0, LED_NUM, DRIVER_NAME);
		led.major = MAJOR(led.devid);
		led.minor = MINOR(led.devid);
	}
	printk("led major %d, minor %d\n", led.major, led.minor);

	led.cdev.owner = THIS_MODULE;
	cdev_init(&led.cdev, &led_ops);
	cdev_add(&led.cdev, led.devid, LED_NUM);

	led.class = class_create(THIS_MODULE, "led class");
	if(IS_ERR(led.class)){
		printk("led class %d\n", IS_ERR(led.class));
		return PTR_ERR(led.class);
	}
	led.device = device_create(led.class, NULL, led.devid, NULL, DRIVER_NAME);
	if(IS_ERR(led.device)){
		return PTR_ERR(led.device);
	}
	printk("led init ok\n");
	return 0;
}
static void __exit leds_exit(void)
{
	cdev_del(&led.cdev);
	unregister_chrdev_region(led.devid, LED_NUM);
	device_destroy(led.class, led.devid);
	class_destroy(led.class);
}

module_init(leds_init);
module_exit(leds_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("wanyisq");
MODULE_DESCRIPTION("imx6ull led driver by pinctr");
