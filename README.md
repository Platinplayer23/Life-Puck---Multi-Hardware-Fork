# Life Puck - Multi-Hardware Fork

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![LVGL](https://img.shields.io/badge/LVGL-9.3.0-green.svg)](https://lvgl.io/)
[![License](https://img.shields.io/github/license/jontiritilli/life-puck)](LICENSE)

**A digital life counter for Magic: The Gathering, Flesh and Blood, Pokémon TCG, and other tabletop games.**

This is a fork of [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) with extended hardware support for multiple Waveshare ESP32-S3 display variants.

---

## 📋 Table of Contents

- [Hardware Support](#-hardware-support)
- [New Features](#-new-features-in-this-fork)
- [Getting Started](#-getting-started)
- [Installation Guide](#-installation-guide)
- [Touch Calibration](#-touch-calibration)
- [Adding Custom Presets](#-adding-custom-presets)
- [Usage Guide](#-usage-guide)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [Credits](#-credits)
- [License](#-license)

---

## 🔧 Hardware Support

This fork supports **two** Waveshare ESP32-S3 display variants:

### Supported Boards

| Board | Display | Touch | GPIO Expander | AliExpress Bundle |
|-------|---------|-------|---------------|-------------------|
| **[ESP32-S3-Touch-LCD-1.85C](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85C)** | ST77916 (QSPI) | CST816 | ✅ TCA9554 | ✅ **Recommended** (includes case + battery) |
| **[ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)** | ST77916 (QSPI) | CST816 | ❌ None | ❌ More expensive |

> **💡 Cost Tip:** The **1.85C variant** is significantly cheaper on AliExpress when purchased as a bundle with case and battery!

### Hardware Specifications

- **MCU:** ESP32-S3-WROOM-1 (N16R8) - 16MB Flash, 8MB PSRAM
- **Display:** 1.85" Round LCD, 360x360px, ST77916 driver (QSPI mode)
- **Touch:** Capacitive touch screen, CST816 controller
- **Connectivity:** USB-C, Wi-Fi, Bluetooth

---

## 🆕 New Features in This Fork

### Multi-Hardware Support
- ✅ **Dual board support**: ESP32-S3-Touch-LCD-1.85C and 1.85
- ✅ **GPIO Expander handling**: Automatic detection and configuration for TCA9554
- ✅ **Unified codebase**: Single project for both hardware variants

### Build System Improvements
- ✅ **Arduino 3.x support**: Uses latest ESP-IDF 5.3 based framework via [pioarduino](https://github.com/pioarduino/platform-espressif32)
- ✅ **Multi-environment builds**: Switch between boards with `platformio.ini` environments
- ✅ **Modern LVGL 9.3.0**: Updated graphics library with performance improvements

### Display & Touch Enhancements
- ✅ **QSPI display support**: High-speed display interface for smoother animations
- ✅ **Touch calibration**: Configurable touch scaling for accurate input
- ✅ **Improved responsiveness**: Better touch detection and debouncing

### Developer Experience
- ✅ **Comprehensive documentation**: Detailed setup and configuration guides
- ✅ **Preset system**: Pre-compile custom game presets to avoid on-device keyboard
- ✅ **Better error handling**: Improved crash recovery and state management

---

## 🚀 Getting Started

### Prerequisites

- **Operating System:** Windows, macOS, or Linux
- **IDE:** Visual Studio Code (recommended)
- **Knowledge:** Basic understanding of C++ and embedded systems
- **Hardware:** One of the supported Waveshare ESP32-S3 boards

### Required Software

1. **Visual Studio Code**: [Download](https://code.visualstudio.com/)
2. **PlatformIO Extension**: Install from VS Code marketplace
3. **USB Drivers**: CH340 or CP210x (usually auto-installed)

---

## 📦 Installation Guide

### Step 1: Install Visual Studio Code

1. Download VS Code from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Install and launch VS Code

### Step 2: Install PlatformIO

1. Open VS Code
2. Go to **Extensions** (Ctrl+Shift+X)
3. Search for **"PlatformIO IDE"**
4. Click **Install**
5. Restart VS Code when prompted

### Step 3: Clone This Repository

**Option A: Using Git (recommended)**
