# 1.使用步骤
## 1.初始化timer相关工作
### 1.定义一个timer_list类型的变量
```
struct timer_list {
    struct list_head entry;
    unsigned long expires;  //定时器超时时间，单位是节拍数
    struct tvec_base *base;
    void (*function)(unsigned long);    //定时器回调函数
    unsigned long data; //传递给function的参数
    int slack;
};
struct timer_list timer;
```
### 2.使用init_timer函数初始化timer_list类型变量，然后初始化timer_list变量相关的参数
```
init_timer(&timer);
timer.function = function;
timer.expries = xxx;
timer.data = xxx;   
//可以设置将某个结构提作为参数传入function，这样可以在function函数内使用相关变量 timer.data = (unsigned long)&dev;
add_timer(&timer);  //启动定时器
```
### 3.定时器相关函数
```
int del_timer(struct timer_list *timer) //删除函数，多核系统上需要等待其他处理器定时处理函数退出

int del_timer_sync(struct timer_list *timer)//上述函数的同步版本

int mod_timer(struct timer_list *timer, unsigned long expires)  //修改定时器定时周期，此函数会激活定时器

```