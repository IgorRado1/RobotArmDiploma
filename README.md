# 🥖 RobotArmDiploma

This repository contains the code and project files for a proof-of-concept robotic sorting system developed as part of a **bachelor's thesis**. The goal is to automatically **sort bread objects** (white and black bread) based on **color and weight**, and to detect **box fullness** using sensors.

## 📦 Project Structure

```text
RobotArmDiploma/
├── all3sensorsV5.ino          # Arduino code: color + weight + 2 ultrasonic sensors + bin LEDs
├── arduino_to_twincatV3.py    # Python script to send data from Arduino to TwinCAT (via pyads)
├── RobotskaRokaV2/            # TwinCAT PLC project (POUs, GVLs, etc.)
├── .gitignore
└── README.md
``` 
## 🧠 Features

- **Color detection** (TCS3472)
- **Weight classification** (HX711 + 4-wire load cell)
- **Bread type recognition** based on color and weight
- **Bin fullness detection** using 2x HC-SR04 ultrasonic sensors
- **LED Indicators** for full bins (4 red LEDs)
- **Real-time communication** between Arduino and TwinCAT via Python & pyads
- **PLC logic** handles movement and resets

## 🔧 Hardware Used

- Arduino Uno
- TCS3472 color sensor
- HX711 + load cell
- 2× HC-SR04 ultrasonic sensors
- Fischertechnik R4 robotic hand
- Beckhoff CX7000 (TwinCAT)
- Breadboards, LEDs, resistors, wiring


## 🖥 TwinCAT PLC

Folder `RobotskaRokaV2/` contains:

- `GVL` with shared variables like `breadID` and `BinFullNumber`
- POUs for automatic robotic arm movement
- Main sorting logic implemented in the POU **`SortTest`** (FB) for the automatic sorting cycle  
- Function blocks (e.g., edge detection, timing)
- PLC mappings for inputs/outputs
- Logic to reset state and react to bin full

## 🚀 System Workflow

1. Arduino reads bread color, weight, and bin fullness.
2. A Python script (`pyads`) receives and sends processed data to the TwinCAT PLC.
3. The PLC executes robotic arm movement based on received bread ID.
4. If any bin is full, a corresponding LED lights up.

## 📚 License

This repository is part of an academic project and is shared under the MIT License for educational purposes.

*Developed by Igor Radošević as part of a bachelor thesis project.*
