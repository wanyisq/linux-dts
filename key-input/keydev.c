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


#define KEY_CNT  2
#define KEY_NAME    "key"
#define INVAKEY     0x00
int key_index_value[KEY_CNT] = {0xF0, 0xF1};

struct key_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *node[KEY_CNT];
    int key_gpio[KEY_CNT];
    atomic_t keyvalue[KEY_CNT];
};
struct key_dev keydev;

static int key_open(struct inode *node, struct file *filp)
{
    filp->private_data = &keydev;
    return 0;
}

static ssize_t key_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0, index = 0;
    unsigned char value[KEY_CNT];
    struct key_dev *dev = filp->private_data;

    printk("key get value %d\r\n", gpio_get_value(dev->key_gpio[0]));
        if(gpio_get_value(dev->key_gpio[0]) == 0){
            while(!gpio_get_value(dev->key_gpio[0]));
            atomic_set(&dev->keyvalue[0], key_index_value[0]);
            printk("key set value %d\r\n", key_index_value[0]);
        }else{
            atomic_set(&dev->keyvalue[0], INVAKEY);
        }

    for(index = 0; index < KEY_CNT; index++){
        value[index] = atomic_read(&dev->keyvalue[index]);
    }

    ret = copy_to_user(buf, &value, sizeof(value));
    return ret;
}

static struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
};

static int key_probe(struct platform_device *dev)
{
    int i = 0;
    printk("key driver and device was matched\r\n");

    if(keydev.major){
        keydev.devid = MKDEV(keydev.major, 0);
        register_chrdev_region(keydev.devid, KEY_CNT, KEY_NAME);
    }else{
        alloc_chrdev_region(&keydev.devid, 0, KEY_CNT, KEY_NAME);
        keydev.major = MAJOR(keydev.devid);
    }

    cdev_init(&keydev.cdev, &key_fops);
    cdev_add(&keydev.cdev, keydev.devid, KEY_CNT);

    printk("key probe mark1\r\n");
    keydev.class = class_create(THIS_MODULE, KEY_NAME);
    if(IS_ERR(keydev.class)){
        return PTR_ERR(keydev.class);
    }
    printk("key probe mark2\r\n");
    keydev.device = device_create(keydev.class, NULL, keydev.devid, NULL, KEY_NAME);
    if(IS_ERR(keydev.device)){
        return PTR_ERR(keydev.device);
    }

    keydev.node[0] = of_find_node_by_name(NULL, "key0");
    if(keydev.node[0] == NULL){
        printk("gpioed node 0 not find key0\r\n");
        goto fail1;
    }

    keydev.node[1] = of_find_node_by_name(NULL, "key1");
    if(keydev.node[1] == NULL){
        printk("gpioed node 1 not find\r\n");
        goto fail1;
    }

    for(i = 0; i < KEY_CNT; i++){
        keydev.key_gpio[i] = of_get_named_gpio(keydev.node[i], "gpios", i);
    }
    gpio_request(keydev.key_gpio[0], "key0");
    gpio_request(keydev.key_gpio[1], "key1");
    for(i = 0; i < KEY_CNT; i++){
        gpio_direction_input(keydev.key_gpio[i]);
        printk("gpio %d set as input\r\n", keydev.key_gpio[i]);
    }
    return 0;

fail1:cdev_del(&keydev.cdev);
    unregister_chrdev_region(keydev.devid, KEY_CNT);
    device_destroy(keydev.class, keydev.devid);
    class_destroy(keydev.class);
    return -EINVAL;
}


static int key_remove(struct platform_device *dev){
    int i = 0;
    for(i = 0; i < KEY_CNT; i++){
        gpio_set_value(keydev.key_gpio[i], 1);
    }

    for(i = 0; i < KEY_CNT; i++){
        gpio_free(keydev.key_gpio[i]);
    }
    
    cdev_del(&keydev.cdev);
    unregister_chrdev_region(keydev.devid, KEY_CNT);
    device_destroy(keydev.class, keydev.devid);
    class_destroy(keydev.class);
    return 0;
}

static const struct of_device_id key_of_match[] = {
    {.compatible = "gpio-keys"},
    {}
};

static struct platform_driver key_driver = {
    .driver = {
        .name = "imx6ull-key",
        .of_match_table = key_of_match,
    },
    .probe = key_probe,
    .remove = key_remove,
};

static int __init keydriver_init(void)
{
    return platform_driver_register(&key_driver);
}

static void __exit keydriver_exit(void)
{
    platform_driver_unregister(&key_driver);
}

module_init(keydriver_init);
module_exit(keydriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanyisq");
