# 简介
- 本项目只支持linux系统运行，目前在ubuntu 22.04 / 20.04 下测试没有问题
- 本项目实现几个利用ncurses库实现的纯文本游戏

- ccz 参考三国志曹操传mod的文本游戏
- tetris 用纯文本界面实现俄罗斯方块
- greedy-snake 贪吃蛇

# 当前状态
ccz
- 显示角色状态及属性
- 显示角色战斗详情，可控制滚动显示

tetris
- 支持自定义任意形状的俄罗斯方块，合理宽高的方块，每个方块都可单独定义自己的属性
- 自定义游戏界面尺寸，分离逻辑和界面，方便更换绘制模块，如ui库绘制
- 已完成方块所有基本功能

greedy-snake
- 具有最基本的移动，吃食物，死亡功能
- 简易演示，没有配置文件，通过修改代码来控制移动速度，窗口大小等参数
# 下亿步目标
- 添加炸弹方块等

# 快速开始  
*开始前检查是否安装了编译器，nucrses和cmake库*
```bash
# $ sudo apt install gcc g++ ncurses-dev cmake

$ mkdir build && cd build
$ cmake ..
$ make install
```
# 运行
```bash
$ cd test
$ ./ccz -c cfg/config.cfg
#或者直接使用默认配置文件 
$ ./ccz
```
# 日志
```bash
所有游戏日志都保存在tmp目录下
$ tail -f tmp/tui-game.log
```
同理运行其他游戏