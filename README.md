# RobotFightCPP

Сервер для MacBook, принимающий MJPEG-кадры от нескольких Raspberry Pi по TCP и передающий их в модуль обработки (`FrameProcessor`).

## Сборка

```bash
mkdir build && cd build
cmake ..
make -j4