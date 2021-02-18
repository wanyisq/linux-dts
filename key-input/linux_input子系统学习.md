# 1.使用前的配置
## 1.使能内核key驱动
```
->Device Drivers
	->Input device support
		->Generic input layer
			->Keyboards
				->GPIO Buttons
```

将key驱动编入内核。

## 2.设备树修改
### 1.使用linux内核的input驱动时，因为gpio_key.c的of_match_table的兼容属性
```
{.compatible = "gpio-keys",},
```
所以设备树节点兼容属性必须是“gpio-keys”。

### 2.在此节点下可以创建字节点表述不同的按键，可以使用lable属性区分不同按键的名字。
### 3.如果是普通按键gpio，可在设备树子节点内指定gpios属性
```
up {
				label = "GPIO Key UP";
				linux,code = <KEY_1>;
				gpios = <&gpio1 0 1>;
			};
```
### 4.如果是指定中断功能，可在设备树子节点内制定interrupts属性
```
down {
				label = "GPIO Key DOWN";
				linux,code = <KEY_2>;
				interrupts = <1 IRQ_TYPE_LEVEL_HIGH 7>;
			};
```
### 5.示例完整的设备树gpio-keys字节点
```
keys {
			compatible = "gpio-keys";
			pinctrl-names = "default";
			pinctrl-0 = <&pinctrl_gpio_keys &pinctrl_gpio_key1>;
			status = "okay";

			key0{
				label = "key 0";
				gpios = <&gpio5 1 GPIO_ACTIVE_HIGH>;
				linux,code = <KEY_1>;
			};

			key1{
				label = "key 1";
				gpios = <&gpio1 18 GPIO_ACTIVE_HIGH>;
				linux,code = <KEY_2>;
			};
	};
```
