# HexLight Firmware

Language: PIC32 C/C++

Supported Hardware: PIC32MX270F256B (HexLight Hardware)

A 4-channel LED controller, supporting advanced colour models, and communication over USB and RS485.
This complements my other project, [jorticus/hexlight-gui](https://github.com/jorticus/hexlight-gui)

Features:

- High resolution high frequency PWM, for flicker-free LED dimming
- Microphone input for audio control of LEDs
- USB HID interface, no drivers required!
- USB Audio interface (work in progress, audio is buggy)
- Advanced CIE xyY/XYZ colour models, for true colour control!

TODO:

- Audio processing (FFT, Beat detection) for control of LEDs
- 4 channel RGB+W (Red, Green, Blue, White) control algorithm, for more natural whites.
- RS485 interface (uses the same framing protocol as what I'm sending over USB-HID)
- Full implementation of CIE XYZ models

**THIS PROJECT IS A WORK IN PROGRESS**
