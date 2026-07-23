# Advanced Monitoring and Assistive Wheelchair (AMAW)

## Overview

AMAW is an IoT-enabled voice-controlled assistive wheelchair designed to improve mobility and safety for individuals with physical disabilities.

The system integrates embedded hardware, wireless communication, biomedical sensing, and a mobile application interface to enable:
- Voice-based wheelchair control
- Real-time vital sign monitoring
- Fall detection
- GPS tracking
- Emergency alerts

## Research Publication

This project resulted in a research publication:

**Publication:**

Bano, Z., Qureshi, F., Ansar, M., Imdad, N., Shams, S., & Shamim, F. (2023).  
*Mobility and Health Monitoring in People with Different Abilities: A Prototype Enhancing Independence: Innovating an IoT-Integrated Wheelchair for.*  
VFAST Transactions on Software Engineering, 11, 256-264.  
DOI: 10.21015/vtse.v11i2.1575

## Hardware Components

### Embedded Controllers & Communication
- ESP32 Microcontrollers
- Bluetooth Communication
- Wi-Fi Communication

### Biomedical & Motion Sensors
- MAX30100 Pulse Oximeter Sensor (Heart Rate and SpO2 Monitoring)
- DS18B20 Temperature Sensor
- MPU6050 Accelerometer/Gyroscope Sensor (Fall Detection)

### Location & Connectivity
- GPS Module
- GSM Module

### Motor Control System
- Motor Driver
- DC Motors

### User Interface
- OLED Display
- Mobile Application developed using MIT App Inventor

## Software & Technologies

- Embedded C/C++
- Arduino IDE
- MIT App Inventor
- Firebase Realtime Database
- Embedded IoT communication protocols

## Repository Contents

- ESP32 firmware source code
- Mobile application interface (developed using MIT App Inventor)
- Circuit diagrams and documentation

## Key Engineering Challenges

- Integrating multiple biomedical and motion sensors, communication modules, and motor control components into a unified assistive wheelchair platform
- Establishing reliable wireless communication between the wheelchair, mobile application, and cloud database
- Implementing real-time acquisition, processing, and transmission of physiological data
- Developing a reliable control system integrating voice commands, sensor feedback, and motor actuation

## Future Improvements
- Autonomous navigation and intelligent obstacle avoidance
- Integration of EEG-based brain-computer interface (BCI) control

- Autonomous navigati- Advanced obstacle avoidance
- Improved power management
