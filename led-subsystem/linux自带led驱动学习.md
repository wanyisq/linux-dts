# 1. 使能LED驱动配置
## 1. 配置LED驱动步骤
首先使用 make menuconfig 命令打开LED驱动配置项

    -> Device Drivers

        ->LED Support 

            ->LED Support got GPIO connected LEDs

## 2. 配置项分析
### 1. 执行以上步骤后，linux内核就会在.config文件内使能“CONFIG_LEDS_GPIO=y”这一行，重新编译内核生成zImage文件后就可以启动linux系统。
### 2. 在执行编译内核时，led灯驱动文件夹内的/drivers/leds/Makefile文件，就会执行
```
obj-$(CONFIG_LEDS_GPIO) += leds-gpio.o
```
此时LED灯驱动文件 /drivers/leds/leds-gpio.c就会被编译进内核中。

## 3. 使用LED驱动需要注意的项
### 1. leds-gpio.c文件内驱动匹配表兼容属性为
```
static const struct of_device_id of_gpio_leds_match[] = {
    {.compatible = "gpio-leds",},
    {},
};
```
所以设备树中led相关的设备节点兼容属性也必须是“gpio-leds”。

### 2. led驱动名字已经规定为“leds-gpio”

## 4. 设备树的改造
### 1.以上所说设备树兼容性属性一定要为“gpio-leds”
### 2.每个字节点都必须设置gpios属性，表示led所使用的GPIO引脚
## 3. 可选属性
### 1.可以设置lable属性表示led灯的名字，例如red、green等。
### 2.可以设置linux,defualt-trigger属性，表示led的默认功能，例如：backlight表示背光、default-on表示led打开、heartbeat表示作为心跳灯。
### 3.可以设置default-state属性，属性值有on、off、keep可选，表示led默认状态。
### 4.设备树不需要改造pinctrl字节点

## 5.linux内led驱动使用方法
```
echo 1 > /sys/class/leds/xxx/brightness  //打开ledxxx
```
xxx表示设备树led节点相关的字节点lable
