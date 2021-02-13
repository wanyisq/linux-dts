# 1. 设备树
## 1. pinctrl属性
设备树中pinctrl属性可以添加多个值，值之间使用空格隔开
例如：
```
pinctrl-0 = <&pinctrl_led_pin_snvc &pinctrl_leds>;
```
## 2. gpio属性
设备树中gpio属性也可以添加多个值，值之间用“,”隔开，结束使用";"
例如：
```
leds0 = <&gpio1 3 GPIO_ACTIVE_LOW>,
				<&gpio1 5 GPIO_ACTIVE_LOW>,
				<&gpio1 6 GPIO_ACTIVE_LOW>,
				<&gpio5 3 GPIO_ACTIVE_LOW>;
```

# 2. 驱动程序
## 1. 设备树匹配
platform驱动中使用of_match_table表匹配设备树中某一个节点的属性，当表中的compatible值匹配到设备树中字节点的compatible属性后就表明此驱动兼容这个设备
例如：

leddev.c
```
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
```
imx6ull.dts
```
wan-leds{
		compatible = "board-leds";
		pinctrl-name = "default";
		pinctrl-0 = <&pinctrl_led_pin_snvc &pinctrl_leds>;
		status = "okay";

		leds0 = <&gpio1 3 GPIO_ACTIVE_LOW>,
				<&gpio1 5 GPIO_ACTIVE_LOW>,
				<&gpio1 6 GPIO_ACTIVE_LOW>,
				<&gpio5 3 GPIO_ACTIVE_LOW>;
	};
```

## 2. 驱动注册函数
在使用probe函数注册驱动时，使用
```
register_chrdev_region
```
和
```
class_create
```
函数时需要注意，注册的字符驱动名字和设备类名字不能一样，否则编译不通过，会提示class创建时在目录下已存在目录名字（字符驱动名字）

## 3. 驱动查找设备节点
使用
```
of_find_node_by_path
```
函数查找设备数节点，所以输入的参数应是设备树字节点。

使用
```
of_gpio_named_count
```
函数统计设备树节点下gpio属性的gpio个数。

### 3. 模拟设备挂在主机nfs目录
使用命令
```
mount -t nfs -o nolock,vers=3 10.0.2.2:/home/book/nfs_rootfs /mnt
```