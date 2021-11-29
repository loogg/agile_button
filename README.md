# Agile Button

## 1、介绍

Agile Button 是基于 RT-Thread 实现的 button 软件包，提供 button 操作的 API。

### 1.1、特性

1. 代码简洁易懂，充分使用RT-Thread提供的API
2. 详细注释
3. 线程安全
4. 断言保护
5. API操作简单

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

- 在打开 Agile Button package 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。
- 文档请查看 [doc/doxygen/Agile_Button.chm](./doc/doxygen/Agile_Button.chm)

### 3.1、示例

使用示例在 [examples](./examples) 下。

### 3.2、Doxygen 文档生成

- 使用 `Doxywizard` 打开 [Doxyfile](./doc/doxygen/Doxyfile) 运行，生成的文件在 [doxygen/output](./doc/doxygen/output) 下。
- 需要更改 `Graphviz` 路径。
- `HTML` 生成未使用 `chm` 格式的，如果使能需要更改 `hhc.exe` 路径。

## 4、注意事项

1. 调用 `agile_btn_create` API创建完按键对象后，调用其他API确保按键对象创建成功，否则被断言。
2. 两次按下间隔在500ms内记为连续按下， `repeact_cnt` 变量自加，否则为0。应用层可以根据该变量得到连续按下多少次，执行相应的动作。
3. 消抖时间设置可以使用 `agile_btn_set_elimination_time` 该API设置，默认为15ms;BTN_HOLD_EVENT事件回调函数的周期可以使用 `agile_btn_set_hold_cycle_time` 该API设置，默认为1s。
4. 通过 `hold_time` 变量可以知道按下持续时间，应用层可以根据其做相应的操作

## 5、联系方式 & 感谢

* 维护：马龙伟
* 主页：<https://github.com/loogg/agile_button>
* 邮箱：<2544047213@qq.com>