# Wordle in C++ Plus

大一计算机程序设计课程作业 Wordle 小游戏的增强版。
为了保留回忆，旧代码已经完整备份到 `legacy/Wordle Plus 2022.cpp`；根目录的 `Wordle Plus.cpp` 是新的跨平台版本。

## 新增功能

- 使用相对路径读取 `ALL.TXT` 和 `SOLUTION.TXT`，不用再把词库放到 `D:\`。
- 支持随机单人局、每日挑战、双人 Duel、Hard Mode、统计面板和分享用 emoji 战绩卡。
- 重新实现 Wordle 判分逻辑，能正确处理重复字母。
- 移除对 Windows 专属背景音乐路径的强依赖，Linux/macOS/Windows 都可以编译运行。
- 统计数据会保存到 `.wordle_stats`（这个运行时文件不需要提交）。

## 编译和运行

```bash
make
./wordle_plus
```

也可以不用 Makefile，直接运行：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic "Wordle Plus.cpp" -o wordle_plus
./wordle_plus
```

## 游戏提示

- 绿色：字母和位置都正确。
- 黄色：答案包含这个字母，但位置不对。
- 灰色：答案不包含这个数量的该字母。
- 游戏中输入 `quit` 可以放弃当前局。
