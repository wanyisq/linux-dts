#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>


/* 1. 确定主设备号                                                                 */
static int major = 0;
static struct class *led_class;
static struct gpio_desc *led_gpio;
static struct gpio_desc *led1, *led2, *led3;


/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* write(fd, &val, 1); */
static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char status;
	//struct inode *inode = file_inode(file);
	//int minor = iminor(inode);
	
	
	err = copy_from_user(&status, buf, 1);
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	printk("status %d\n", status);
	/* 根据次设备号和status控制LED */
	gpiod_set_value(led_gpio, status);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	//int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 根据次设备号初始化LED */
	gpiod_direction_output(led_gpio, 0);
	
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

/* 4. 从platform_device获得GPIO
 *    把file_operations结构体告诉内核：注册驱动程序
 */
static int chip_demo_gpio_probe(struct platform_device *pdev)
{
	//int err;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* 4.1 设备树中定义有: led-gpios=<...>;	*/

 
	led_gpio = gpiod_get(&pdev->dev, "wan", 0);
	if (IS_ERR(led_gpio)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led_gpio);
	}
	printk("led 0 found\n");
	
	led1 = gpiod_get_index(&pdev->dev, "led", 0, 0);
	printk("led 1 start\n");
	if (IS_ERR(led1)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led1);
	}
    printk("led 1 found\n");

	led2 = gpiod_get_index(&pdev->dev, "led", 1, 0);
	if (IS_ERR(led2)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led2);
	}
    printk("led 2 found\n");

	led3 = gpiod_get_index(&pdev->dev, "led", 2, 0);
	if (IS_ERR(led3)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led3);
	}
    printk("led 3 found\n");

	/* 4.2 注册file_operations 	*/
	major = register_chrdev(0, "100ask_led", &led_drv);  /* /dev/led */

	led_class = class_create(THIS_MODULE, "100ask_led_class");
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led");
		gpiod_put(led_gpio);
		return PTR_ERR(led_class);
	}

	device_create(led_class, NULL, MKDEV(major, 0), NULL, "100ask_led%d", 0); /* /dev/100ask_led0 */
            
    return 0;
    
}

static int chip_demo_gpio_remove(struct platform_device *pdev)
{
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "100ask_led");
	gpiod_put(led_gpio);
    
    return 0;
}


static const struct of_device_id ask100_leds[] = {
//    { .compatible = "other_led" },
    { .compatible = "100ask, leddrv" },
    {},
};

/* 1. 定义platform_driver */
static struct platform_driver chip_demo_gpio_driver = {
    .probe      = chip_demo_gpio_probe,
    .remove     = chip_demo_gpio_remove,
    .driver     = {
        .name   = "100ask_led",
        .of_match_table = ask100_leds,
    },
};

/* 2. 在入口函数注册platform_driver */
static int __init led_init(void)
{
    int err;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	
    err = platform_driver_register(&chip_demo_gpio_driver); 
	
	return err;
}

/* 3. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数
 *     卸载platform_driver
 */
static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    platform_driver_unregister(&chip_demo_gpio_driver);
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


