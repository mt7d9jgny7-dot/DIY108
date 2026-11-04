# RP2040 薄膜键盘主控固件 (WebHID)

基于 Raspberry Pi Pico SDK 与 TinyUSB 框架从零开发的 RP2040 薄膜键盘裸机固件。完全不依赖 QMK，采用 C 语言开发，代码高度模块化，具备启动快、低延迟、免装软件网页改键的特点。

## 特色功能

- 极度轻量：裸机开发，直接操作 GPIO 矩阵扫描。
- 全模块化：扫描、报文、数据层完全解耦。
- WebHID 改键：Chrome/Edge 打开网页即可实时下发配置。
- 掉电保存：配置写入 RP2040 内部 Flash，重启不丢失。
- 复合设备：同时支持键盘、鼠标、多媒体控制。
- 自动构建：GitHub Actions 一键在线编译。

## 硬件引脚

- 行 (Rows 0~7): GP0 到 GP7
- 列 (Cols 0~17): GP8 到 GP22, GP26 到 GP28

注意：若 PCB 走线不同，请修改 matrix.c 中的 ROWS 和 COLS 数组。

## 工程结构

- `.github/workflows/build.yml`：GitHub Actions 自动编译工作流。
- `web/`：网页驱动文件夹
  - `common.js`：网页公共逻辑与 WebHID 通信封装。
  - `index.html`：可视化网页驱动的入口页面。
  - `style.css`：网页样式表。
- `CMakeLists.txt`：CMake 构建配置文件。
- `hid.c` / `hid.h`：业务层，负责 HID 报文分发与 WebHID 分包重组。
- `keycodes.h`：所有 HID 键码、鼠标、多媒体及宏触发键定义。
- `keymap.c` / `keymap.h`：数据层，默认键位表、Flash 读写与时间戳校验。
- `macro.c` / `macro.h`：引擎层，宏动作解析与播放状态机。
- `main.c`：极简主循环，负责各模块调度。
- `matrix.c` / `matrix.h`：硬件层，引脚初始化、矩阵扫描与去抖。
- `tusb_config.h`：TinyUSB 配置文件。
- `usb_descriptors.c`：USB 复合设备与 HID 描述符定义。
- `README.md`：项目说明文档。

## FN 层按键说明

默认在 Layer 0 (基础层)。按住矩阵位置 R3C4 的按键 (基础层的 F 键)，激活 Layer 1 (FN 层)。

鼠标控制区：
- R2C2 (W) -> MS_UP : 鼠标向上
- R3C1 (A) -> MS_LEFT : 鼠标向左
- R3C2 (S) -> MS_DOWN : 鼠标向下
- R3C3 (D) -> MS_RIGHT : 鼠标向右
- R2C5 (R) -> MS_BTN1 : 鼠标左键
- R2C6 (U) -> MS_BTN2 : 鼠标右键
- R2C7 (I) -> MS_BTN3 : 鼠标中键
- R2C8 (O) -> MS_WH_U : 滚轮向上
- R2C9 (P) -> MS_WH_D : 滚轮向下

多媒体控制区：
- R0C0 (LCTL) -> CC_MUTE : 静音
- R0C1 (GRV) -> CC_VOLD : 音量减
- R0C2 (F1) -> CC_VOLU : 音量加
- R0C3 (F2) -> CC_MPLY : 播放暂停
- R0C5 (5) -> CC_MPRV : 上一曲
- R0C6 (6) -> CC_MNXT : 下一曲
- R0C7 (EQL) -> CC_STOP : 停止
- R0C8 (F8) -> CC_BRIU : 亮度加
- R0C9 (MINS) -> CC_BRID : 亮度减

浏览器与系统控制区：
- R1C0 (F5) -> CC_WBAK : 浏览器后退
- R1C1 (1) -> CC_WFWD : 浏览器前进
- R1C2 (2) -> CC_WHOM : 浏览器主页
- R1C3 (3) -> CC_WSCH : 浏览器搜索
- R6C1 (ESC) -> CC_PWR : 系统电源
- R6C2 (LALT) -> CC_SLEP : 系统睡眠

## 如何使用网页驱动

1. 使用 Chrome 或 Edge 浏览器（89+版本）。
2. 双击打开 `web/index.html` 文件。
3. 点击左侧“连接设备”，选择 `RP2040 Film KB`。
4. 在“自定义按键”页点击任意键即可修改功能；在“宏设置”页可录入宏动作。
5. 点击顶部“写入到设备”即可永久保存（写入时网页可能会提示成功，键盘在后台延迟 500ms 后存入 Flash）。

## 编译与刷写

方法一：GitHub Actions (推荐)
1. 将代码推送到 GitHub 仓库。
2. 在 Actions 页面等待自动编译完成。
3. 在 Artifacts 中下载 .uf2 固件。

方法二：本地编译
需提前配置好 PICO_SDK_PATH 环境变量。
mkdir build && cd build
cmake ..
make -j4

刷写固件：
按住 RP2040 板上的 BOOT 键插入电脑，将 .uf2 文件拖入弹出的 RPI-RP2 U盘中，板子会自动重启运行。若固件已在运行，双击 RST 键也可进入刷写模式。

## 自定义调整

- 去抖时间：修改 matrix.c 中的 DEBOUNCE_TIME_MS，默认 5ms。
- 鼠标速度：修改 hid.c 中的 mouse_dx 和 mouse_dy 步长数值，默认 5。
- 设备名称：修改 usb_descriptors.c 的 string_desc_arr 数组内容。