# 简介
- 本项目只支持linux系统运行，目前只在ubuntu 22.04 下测试
- 本项目实现几个利用ncurses库实现的纯文本游戏

- ccz 参考三国志曹操传mod的文本游戏
- tetris 用纯文本界面实现俄罗斯方块

# 当前状态
ccz
- 显示角色状态及属性
- 显示角色战斗详情，可控制滚动显示

tetris
- 支持自定义任意形状的俄罗斯方块，每个方块都可单独定义自己的属性
- 自定义游戏界面尺寸
- 已经支持更丰富的配置文件
# 下亿步目标
- 看新想法

# 快速开始  
*开始前检查是否安装了编译器，nucrses和cmake库*
```bash
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
同理运行其他游戏