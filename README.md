# Agile Button

## 1、介绍

Agile Button 是基于 RT-Thread 实现的 button 软件包，提供 button 操作的 API。

- 按键操作的状态变化为：`未按下  ——>  按下  ——>  未按下`。

- 为了消除干扰，默认的消抖时间为 15ms，可以通过 `agile_btn_set_elimination_time` API 更改消抖时间。

- 间隔在 500ms 内的连续操作，记为多次操作，更改对象的 `repeat_cnt` 属性，通过该属性可获取连续操作次数。

- 在按下期间会计算对象的持续按下时间，可通过对象的 `hold_time` 属性获取。

```C
   ____          __      __          ____
__|    |__  ——>    |____|    ——>  __|    |__

```

Agile Button 提供了 4 种事件：

```C
BTN_PRESS_DOWN_EVENT
BTN_HOLD_EVENT
BTN_PRESS_UP_EVENT
BTN_CLICK_EVENT
```

可以通过 `agile_btn_set_event_cb` API 设置每个事件的触发回调。

- BTN_PRESS_DOWN_EVENT

从 `未按下  ——>  按下` 触发一次。

```C
   ____          __      __
__|    |__  ——>    |____|

```

- BTN_HOLD_EVENT

一直处于按下状态，默认每隔 1s 触发一次，可以通过 `agile_btn_set_hold_cycle_time` API 更改触发周期。

```C
__      __
  |____|

```

- BTN_PRESS_UP_EVENT

从 `按下  ——>  未按下` 触发一次。

```C
__      __          ____
  |____|    ——>  __|    |__

```

- BTN_CLICK_EVENT

一次完整的操作触发一次，即：`未按下  ——>  按下  ——>  未按下`。

**注意**：`BTN_PRESS_UP_EVENT` 也会被触发。

```C
   ____          __      __          ____
__|    |__  ——>    |____|    ——>  __|    |__

```

### 1.1、特性

1. 代码简洁易懂，充分使用 RT-Thread 提供的 API
2. 详细注释
3. 线程安全
4. 断言保护
5. API 操作简单

### 1.2、目录结构

| 名称 | 说明 |
| ---- | ---- |
| doc | 文档目录 |
| examples | 例子目录 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.3、许可证

Agile Button package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.4、依赖

- RT-Thread 3.0+
- RT-Thread 4.0+

## 2、如何打开 Agile Button

使用 Agile Button package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] agile_button: A agile button package
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 Agile Button

- 帮助文档请查看 [doc/doxygen/Agile_Button.chm](./doc/doxygen/Agile_Button.chm)

如果未使能 PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT:

1. agile_btn_env_init 初始化环境

2. 创建一个线程，周期调用 agile_btn_process，建议周期时间不要太长

- agile_btn_create / agile_btn_init 创建 / 初始化对象
- agile_btn_set_elimination_time 更改消抖时间，可忽略
- agile_btn_set_hold_cycle_time 更改持续按下触发周期时间，可忽略

  该操作也可在运行过程中执行

- agile_btn_set_event_cb 设置事件触发回调
- agile_btn_start 启动运行
- agile_btn_stop 运行过程中强制停止

### 3.1、示例

使用示例在 [examples](./examples) 下。

### 3.2、Doxygen 文档生成

- 使用 `Doxywizard` 打开 [Doxyfile](./doc/doxygen/Doxyfile) 运行，生成的文件在 [doxygen/output](./doc/doxygen/output) 下。
- 需要更改 `Graphviz` 路径。
- `HTML` 生成未使用 `chm` 格式的，如果使能需要更改 `hhc.exe` 路径。

## 4、联系方式 & 感谢

- 维护：马龙伟
- 主页：<https://github.com/loogg/agile_button>
- 邮箱：<2544047213@qq.com>
