# Debug Backend & Camera Capture Guide

## Overview

当前固件调试场景使用单端口的 `debug/ov2640_test_backend.py` 脚本模拟 OTA、WebSocket 与图片上传。
脚本基于 `aiohttp`，在 `http://<HOST_IP>:8000` 同时提供：

- `/ota.json` OTA 接口；
- `/ws` WebSocket 服务；
- `/upload` 图片上传接口。

脚本启动后典型输出：

```
[backend] debug-backend aiohttp v2: Backend starting
[backend] debug-backend aiohttp v2: Starting HTTP server
[backend] debug-backend aiohttp v2: HTTP server bound to 0.0.0.0:8000
```

后续在调试日志中可以看到 WebSocket 握手过程（connected / sent hello / recv hello / sent initialize）以及图片上传日志（`Received image bytes...`，`Saved capture to capture.jpg`）。

## Firmware Notes

为了在禁用 PSRAM 的硬件上保持兼容，`main/boards/common/esp32_camera.cc` 做了两项调整：

1. 预览图的 `heap_caps_malloc` 在无 PSRAM 时退回内部 8-bit RAM；
2. JPEG 编码队列的 `heap_caps_aligned_alloc` 在无 PSRAM 时同样落入内部 RAM。

这样即便板载未连接 PSRAM，摄像头依旧可以完成拍照与编码流程，不会因为缓存分配失败导致 `Capture()` / `Explain()` 抛异常。

## Usage Steps

1. **释放端口**  
   确保 8000 端口未被旧脚本占用，可通过 `sudo lsof -i :8000` 查询并 `kill` 对应进程。

2. **运行脚本**  
   在仓库根目录执行：
   ```
   python debug/ov2640_test_backend.py
   ```
   等待脚本输出 “HTTP server bound...” 等提示，表示服务启动完毕。

3. **触发拍照**  
   固件连接 Wi-Fi 并完成 OTA 伪装后，脚本会自动发送 `self.camera.take_photo` 指令；若未自动触发，可通过一次“开始监听”操作让设备调用 `OpenAudioChannel()`。

4. **验证结果**  
   上传成功时脚本会打印：
   ```
   [backend] ... Received image bytes: <size>
   [backend] ... Saved capture to capture.jpg
   ```
   项目根目录随即生成 `capture.jpg`，可以通过 `ls -lh capture.jpg` 查看。

> ⚠️ 每次修改固件后，请重新 `idf.py build` 并烧录，以确保最新修改生效。

## Pending Checks

- 尚未在实际硬件上验证新的脚本 + 固件组合是否稳定生成 `capture.jpg`。
- 若脚本或固件仍出现异常日志，请记录串口与脚本输出，并重新反馈以协助排查。

