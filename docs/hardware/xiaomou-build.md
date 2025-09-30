# Xiaomou 板卡编译指南

本文档介绍如何在本仓库中构建并烧录 Xiaomou 自研板卡的固件。示例命令均在 macOS/Linux 终端下执行，Windows 用户建议使用 WSL 或 ESP-IDF 官方 Docker 环境。

## 1. 环境准备
- **ESP-IDF 版本：** 推荐 5.4 及以上，确保已安装 Python、CMake、Ninja 等依赖。
- **工具链激活：**
  ```bash
  source $IDF_PATH/export.sh
  ```
- **硬件依赖：** ESP32-S3 WROOM-1 模块、ST7796S 3.5" SPI LCD（参见 `docs/hardware/xiaomou-lcd.md`）、ES8311 音频编解码器、OV 系列并口摄像头。

## 2. 获取源码
```bash
git clone https://github.com/IamYiranKe/xiaomou.git
cd xiaomou/xiaomou
```

## 3. 选择目标与板型
1. 设定芯片目标：
   ```bash
   idf.py set-target esp32s3
   ```
2. 启用 Xiaomou 板配置：
   ```bash
   idf.py -DBOARD_NAME=xiaomou menuconfig
   ```
   - `Board Type` 中确保选择 **Xiaomou Custom Board**。
   - 若需自定义服务器、语音或显示设置，可在菜单内调整相应模块。

## 4. 构建与烧录
```bash
idf.py -DBOARD_NAME=xiaomou build
idf.py -DBOARD_NAME=xiaomou -p <串口号> flash monitor
```
- `build` 会生成固件并校验链接脚本。
- `flash monitor` 会烧写并打开串口日志（按 `Ctrl+]` 退出）。

## 5. 资源与调试要点
- **LCD 初始化：** `main/boards/xiaomou/xiaomou_board.cc` 已内置 ST7796S 初始化序列，若显示异常，可对照 `debug/lcd/3.5-36A(1)(1).txt` 调整寄存器。
- **音频链路：** ES8311 相关引脚与 I²S 采样率定义位于 `main/boards/xiaomou/config.h`。
- **摄像头验证：** 利用 `idf.py monitor` 查看帧缓冲日志，若初始化失败会打印错误码。
- **资产管理：** 如需刷新语音或表情资源，可运行 `scripts/build_default_assets.py --board xiaomou`，然后重新烧写。

## 6. 常见问题
- **串口无法识别：** 检查 USB 数据线，确认进入下载模式（默认 BOOT+EN）。
- **LCD 方向异常：** 调整 `DISPLAY_MIRROR_X/Y` 或 `DISPLAY_SWAP_XY` 并重新编译。
- **音频无输出：** 确认 `AUDIO_CODEC_PA_PIN` 拉高，必要时在 `menuconfig` 中禁用省电模式重试。

若硬件参数变动，请同步更新 `main/boards/xiaomou/config.h` 与本文档，以免编译选项失效。
