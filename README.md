# Smart Truck Scale Model with Sensors: Guided Maneuverability

This repository contains the source code, circuit diagrams, and documentation for the Capstone Project (TCC) **Lita Avalon**. The objective of the project is the development of a functional scale model of a semi-autonomous truck focused on assisting safe driving and guided maneuverability, mitigating accidents caused by human error.

---

## Group Members
* **Andrei Bozato Bandeira**
* **Felipe Gabriel Sousa Macedo**
* **Italo Bruno Silva**
* **Luiz Gustavo Neves do Valle**

**Advisors:** Prof. José Antonio Meire and Prof. Luis Carlos da Silva  
**Institution:** ETEC Lauro Gomes 
**Course:** High School with Professional Qualification of Mechatronics Technician (M-Tec PI Mechatronics)
**Year:** 2025

---

## Folder and Document Structure

Below is the description of the repository organization and the meaning of each directory and document:

* **`/Application`**: Contains the application used to control the smart truck scale model.
* **`/ArduinoCode`**: Contains the main source file of the firmware developed in C++ in the Arduino IDE environment for the ESP32 microcontroller.
* **`/documents`**: Folder reserved for the textual documentation of the Capstone Project (Monograph in PDF).
* **`README.md`**: This documentation file with the overview and usage instructions of the project.

---

## Project Summary

The **Lita Avalon** model consists of a simulated physical scale model of an electric truck remotely controlled via Bluetooth. Using the **ESP32** microcontroller as the central processing unit, the system collects data from ultrasonic distance sensors and a magnetometer (digital compass) to act in real time on driver assistance, without removing full driving responsibility from them (Classified in the initial levels of SAE automation).

---

## Technical Specifications and Components

The multidisciplinary development integrated electronic, mechanical components and structured programming in C++:

* **Central Unit (Brain):** ESP32 DEVKIT V1 (30 pins, Dual-Core, classic Bluetooth and BLE).
* **Traction Actuator:** DC Motor 25GA370 (6V, 320 nominal RPM) coupled to a gear motor with a 26:1 gear ratio, operating with polymer gears in a speed reduction configuration for torque gain.
* **Power Driver (Traction):** L298N H-Bridge Module for direction reversal (Forward/Reverse) and speed control by pulse width modulation (PWM).
* **Steering Actuator:** Micro Servo Motor SG90 (Controlled by angles from 0° to 180° via `ESP32Servo` library).
* **Base Sensors:** 
  * HC-SR04 Ultrasonic Sensor for distance reading.
  * QMC5883L Magnetometer (I2C Compass) for direction monitoring.
* **Visual Interface (On-board Computer):** OLED Display SSD1306 ($128\times64$ pixels) managed with the `Adafruit_SSD1306` library.
* **Auxiliary Circuits:** NPN Transistors 2N2222A acting as current drivers for the headlights and turn signals (direction indicator lights).
* **Power Supply:** Two 18650 lithium batteries (providing ~8V in series) regulated by an MP1584EN Step Down module adjusted to 5.5V to power the peripherals.

---

## Semi-Autonomous Functions Implemented

The firmware integrates logical routines to process peripheral information and assist in safe driving:

1. **Automatic Brake:** Monitors the approach of obstacles ahead and blocks acceleration by cutting the PWM active cycle (duty cycle) if it reaches the critical distance.
2. **Light Control by Distance:** Modulates the luminous intensity of the LED headlights (via dedicated PWM) based on the proximity of other vehicles.
3. **Turning Maneuver / Blind Spot Assistance:** Executes scanning routines with the rear ultrasonic sensors in parallel with the activation of the turn signals to warn about side collisions.
4. **Start Stop System:** Turns off the motor active cycle and emits visual alerts on the on-board computer for energy optimization during prolonged stops.
5. **Acoustic Alert:** Emission of beeps through an Active Buzzer proportional to the proximity of objects during reverse gear or emergency braking.

---

## Environment Setup and Compilation

To compile the code contained in the `/src` folder:

1. Install the **Arduino IDE** (Version 2.3.5 or higher).
2. Install the **Silicon Labs CP2102** driver in the operating system for recognition of the ESP32 COM port.
3. In the IDE *Preferences*, add the URL of the additional Espressif board:
   `https://dl.espressif.com/dl/package_esp32_index.json`
4. Install the following libraries through the internal manager:
   * `Adafruit SSD1306` and `Adafruit GFX Library`
   * `ESP32Servo`
   * `Ultrasonic`
5. Select the **DOIT ESP32 DEVKIT V1** board, configure the corresponding serial port, and upload.

## Avalon Control Application (Android)

The control and movement of the scale model are carried out through an application developed exclusively for the project, which communicates with the ESP32 via **Bluetooth SPP (Serial Port Profile)** protocol. 

### Main App Features:
* **Movement Control:** Sending directional commands for forward, reverse, left, and right, translated into PWM signals for the motors.
* **Lighting Control Panel:** Dedicated buttons for manual activation of the auxiliary headlights and signaling lights (turn signals).
* **Monitoring and Telemetry:** The application works in a two-way system, receiving data sent by the truck and displaying proximity alerts on the cell phone screen.

## Practical Demonstrations and Design

In this section is the three-dimensional design conception of the project.

### Three-Dimensional Modeling (CAD): Design Conception of the Avalon Cabin

https://github.com/user-attachments/assets/8d9a7322-8c06-492d-8410-a1b85e56a00c

---
ETEC Lauro Gomes — Industrial Mechatronics (M-Tec PI)



