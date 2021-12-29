# 示例说明

## 1、介绍

该示例提供动态创建和静态创建功能演示，需要 `FINSH` 的支持。请开启 `FINSH` 功能。

## 2、使用

### 2.1、动态创建

- 创建对象并启动

  命令行输入 `dbtn_create [pin] [active]`

  pin: 引脚号，查看驱动中按键 IO 对应的引脚号

  active: 有效电平 1/0

- 删除对象

  命令行输入 `dbtn_delete`

### 2.2、静态创建

- 初始化对象并启动

  命令行输入 `sbtn_init [pin] [active]`

  pin: 引脚号，查看驱动中按键 IO 对应的引脚号

  active: 有效电平 1/0
