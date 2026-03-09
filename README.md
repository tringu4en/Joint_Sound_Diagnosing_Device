# Joint&Waves: Wearable Acoustic Joint Diagnosis System

**Design and development of a wearable system for diagnosing joint conditions via acoustic analysis of movement-induced sounds.**

![Device Outside](hardware/device_outside.jpg)
![Device Inside](hardware/device_inside.jpg)

## Overview
Knee osteoarthritis (OA) affects over 365 million people globally, with standard diagnostics relying on costly, radiation-heavy methods like X-rays and MRIs. Joint&Waves is a low-cost, non-invasive wearable device utilizing Vibroarthrography and Edge AI. It performs real-time acoustic analysis of joint sounds to classify conditions as Normal, Osteoarthritis, or Ligament Injury, enabling continuous joint health tracking.

## Hardware Architecture
The data acquisition node is engineered for optimal patient comfort and non-intrusive monitoring.
* **Microcontroller:** ESP32-S3 Module.
* **Sensor:** SPH0645 MEMS microphone for high-fidelity acoustic capture.
* **Power:** Safely managed Li-Po battery with a modern Type-C charging interface.
* **Dimensions:** Ultra-compact design measuring 3.75x2.7x2.35 cm.
* **Endurance:** Sustains up to 40 minutes of continuous data acquisition and on-device AI inference per full charge.

## Signal Processing & Edge AI Pipeline
By computing the diagnosis directly on the edge node, the system significantly reduces energy overhead and secures raw acoustic data.
1. **Acquisition:** Captures raw joint sound during movement. Spectral analysis confirms critical features are concentrated within the 0-8000 Hz frequency band.
2. **Signal Processing:** Filters the raw sound and extracts key acoustic features, prioritizing mean amplitude, spectral entropy, and mean frequency. 
3. **Inference:** A highly optimized Random Forest TinyML model runs directly on the ESP32-S3 to classify the joint condition. 
4. **Transmission:** The diagnosis result is transmitted via BLE to a PC or smartphone for real-time visualization.

## Performance Metrics
The Random Forest model demonstrates strong operational stability and classification capability without signal degradation. 
* **Normal Joints:** 100% accuracy in classification.
* **Pathological Joints:** Strong differentiation capabilities, with an 83% recall for Osteoarthritis and 85.7% recall for Ligament Injuries.

## Repository Contents
* `/hardware`: Device schematics and internal/external photographs.
* `/firmware`: ESP32-S3 C++ code for I2S microphone interfacing, DSP filtering, and BLE transmission.
* `/model`: Feature extraction scripts and TinyML model deployment files.
* `/docs`: Project poster and extended evaluation charts.

## Authors and Acknowledgements
* **Authors:** Nguyen Tan Tri, Ta Hoang Truc Tho.
* **Acknowledgements:** Special thanks to Assoc. Prof. Le Ngoc Bich for the opportunity to design this wearable device, and to Nguyen Nhat Minh and Ta Minh Tri for their research and development support.
