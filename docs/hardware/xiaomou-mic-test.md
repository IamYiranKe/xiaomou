推荐流程

  - 进入音频测试模式: 上电时按住 BOOT（或在运行中短按触发“网络配置”模式），设备进入 kDeviceStateWifiConfiguring 后，短按 BOOT 会看到日志 Enabling audio testing。
  - 开始录音测试: idf.py monitor 中保持串口连线，对着麦克风说话 5–10 秒，确认日志持续输出 Audio testing queue 的采样帧。
  - 回放验证: 再次短按 BOOT 关闭测试（日志 Disabling audio testing），缓存的麦克风数据会编码→解码→扬声器回放，能清晰听到自己的声音即说明麦克风链路正常。
  - 恢复状态: 若仍在网络配置页，可再按一次 BOOT 退出音频测试，或长按 EN/RST 重启。

  日志验证

  - 成功启动: AudioService: Enabling audio testing、AudioInputTask 读入 16k PCM。
  - 队列情况: 若看到 Audio testing queue is full，说明录音过长，可直接停止让其回放。
  - 编解码链路: 回放时应出现 Opus codec task 的 encode/decode 计数增加；如需更详细信息，可在 menuconfig → Component config → Log output 提升 AudioService 日志级别。

  常见问题
  - 按键无效: 某些板型需长按 BOOT 超过 2 秒才切换，或在 main/boards/<board>/... 中查找按键事件逻辑。
  - 进一步测试: 若要远端验证，可使用 WebSocket 协议向设备发送 SendStartListening，上位机抓包接收的 Opus 帧并解码收听。