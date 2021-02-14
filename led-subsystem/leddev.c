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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LEDON   1
#define LEDOFF  0
#define LEDDEV_CNT  1
#define LEDDEV_NAME "led_driver_use_dts_platform"
#define LEDDEV_CLASS_NAME   "led_driver_use_dts_platform"//"led_class"

struct leddev_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *node;
    int gpio[4];
    int gpio_count;
};
struct leddev_dev leddev;

static int led_open(struct inode *node, struct file *flip)
{
    flip->private_data = &leddev;
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
    unsigned char databuff[2];
    unsigned char ledstate;
    int i = 0;

    ret = copy_from_user(databuff, buf, cnt);
    if(ret < 0){
        printk("kernel write failed\r\n");
        return 1;
    }

    ledstate = databuff[1];
    i = databuff[0];
    printk("led_write %d, status %d\r\n", leddev.gpio[i], ledstate);
    gpio_set_value(leddev.gpio[i], ledstate);

    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};

static int led_probe(struct platform_device *dev)
{
    int i = 0;
    printk("led driver and device was matched\r\n");

    if(leddev.major){
        leddev.devid = MKDEV(leddev.major, 0);
        register_chrdev_region(leddev.devid, LEDDEV_CNT, LEDDEV_NAME);
    }else{
        alloc_chrdev_region(&leddev.devid, 0, LEDDEV_CNT, LEDDEV_NAME);
        leddev.major = MAJOR(leddev.devid);
    }

    cdev_init(&leddev.cdev, &led_fops);
    cdev_add(&leddev.cdev, leddev.devid, LEDDEV_CNT);

    printk("led probe mark1\r\n");
    leddev.class = class_create(THIS_MODULE, LEDDEV_CLASS_NAME);
    if(IS_ERR(leddev.class)){
        return PTR_ERR(leddev.class);
    }
    printk("led probe mark2\r\n");
    leddev.device = device_create(leddev.class, NULL, leddev.devid, NULL, "LEDDEV_NAME" "%d", MINOR(leddev.devid));
    if(IS_ERR(leddev.device)){
        return PTR_ERR(leddev.device);
    }

    leddev.node = of_find_node_by_path("/wan-leds");
    if(leddev.node == NULL){
        printk("gpioed node not find\r\n");
        return -EINVAL;
    }

    leddev.gpio_count = of_gpio_named_count(leddev.node, "leds0");
    printk("led gpio count is %d\r\n", leddev.gpio_count);
    for(i = 0; i < leddev.gpio_count; i++){
        leddev.gpio[i] = of_get_named_gpio(leddev.node, "leds0", i);
    }



    // leddev.gpio = of_get_named_gpio(leddev.node, "leds0", 0);
    // if(leddev.gpio < 0){
    //     printk("can not get leds-gpios\r\n");
    //     return -EINVAL;
    // }
    printk("led_probe gpio[0] %d, gpio[1] %d, gpio[2] %d, gpio[3] %d\r\n",
                     leddev.gpio[0], leddev.gpio[1], leddev.gpio[2], leddev.gpio[3]);

    gpio_request(leddev.gpio[0], "led0");
    gpio_request(leddev.gpio[1], "led1");
    gpio_request(leddev.gpio[2], "led2");
    gpio_request(leddev.gpio[3], "led3");

    for(i = 0; i < leddev.gpio_count; i++){
        gpio_direction_output(leddev.gpio[i], 1);
    }
    return 0;
}

static int led_remove(struct platform_device *dev){
    int i = 0;
    for(i = 0; i < leddev.gpio_count; i++){
        gpio_set_value(leddev.gpio[i], 1);
    }

    for(i = 0; i < leddev.gpio_count; i++){
        gpio_free(leddev.gpio[i]);
    }
    
    cdev_del(&leddev.cdev);
    unregister_chrdev_region(leddev.devid, LEDDEV_CNT);
    device_destroy(leddev.class, leddev.devid);
    class_destroy(leddev.class);
    return 0;
}

static const struct of_device_id led_of_match[] = {
    {.compatible = "board-leds"},
    {}
};

static struct platform_driver led_driver = {
    .driver = {
        .name = "imx6ull-led",
        .of_match_table = led_of_match,
    },
    .probe = led_probe,
    .remove = led_remove,
};

static int __init leddriver_init(void)
{
    return platform_driver_register(&led_driver);
}

static void __exit leddriver_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanyisq");
