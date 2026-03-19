# ESP32-Cam Autonomous MQTT Camera

## Concept
This circuit realizes an autonomous, battery-powered surveillance camera based on the ESP32-Cam (AI-Thinker model).

### Functionality
* **Autonomous Operation:** Designed for long-term battery use.
* **Interval Capture:** The ESP32 wakes up at defined intervals, captures a photo, and sends it via WiFi/MQTT to a Home Assistant instance.
* **Power Efficiency:** Uses Deep Sleep mode between cycles to minimize power consumption.
* **Manual Trigger:** A tactile button allows manual image triggering at any time.
* **Status Indication:** Two LEDs provide visual feedback for status or errors (e.g., connection issues).

## Project Structure
The repository is organized to separate hardware design from firmware development while maintaining a unified workspace.

```text
esp32-cam-system/           # Root directory (Open this in VS Code)
├── .gitignore              # Global ignore rules for PIO and KiCad
├── platformio.ini          # Central configuration (redirects to firmware/)
├── README.md               # Project documentation
├── .vscode/                # VS Code specific settings
│   ├── settings.json       # IntelliSense and include paths
│   └── tasks.json          # Build and Upload shortcuts
├── firmware/               # Firmware source files
│   └── src/                # .cpp and .h files (Logic & Headers)
├── hardware/               # KiCad PCB project
│   ├── esp32-cam-hw.kicad_pro
│   └── libraries/          # Local Footprints and Symbols
└── docs/                   # Datasheets, 3D models (STL), and images
```

## Development Environment & Settings

### PlatformIO & VS Code
To maintain a clean root directory while keeping firmware in a subfolder, the central `platformio.ini` is configured as follows:
* **Custom Paths:** Redirects `src_dir` and `include_dir` to the `firmware/` folder.
* **Hardware Abstraction:** PSRAM is explicitly enabled via build flags (`-DBOARD_HAS_PSRAM`) to handle image processing.
* **IntelliSense:** The `.vscode/settings.json` includes `${workspaceFolder}/firmware/src` to ensure header files are recognized without errors.

### KiCad Workflow
* **Portability:** All custom footprints and symbols are stored in `hardware/libraries/`.
* **Path Variables:** Library paths should use the `${KIPRJMOD}/libraries/` variable to ensure the project remains functional when cloned to different machines.

## Hardware Description

### Components

#### Main Board
* **U1 (ESP32-Cam AI-Thinker):** Core module containing the ESP32-S SoC, camera, and SD card slot.
* **J1 (Power Supply):** 2-pin JST-XH connector for 5V external power.
* **J2 (Programming):** 5-pin header for FTDI flashing and UART debugging.
* **J3 (Front Panel Link):** 4-pin socket to connect the daughterboard.

#### Daughterboard (Front Panel)
* **SW1 (Button):** Manual capture trigger, push button DTR 7/RT (Connects IO12 to GND).
* **D1 (Red LED):** Error indicator (MQTT/WiFi failure).
* **D2 (Status LED):** Activity indicator (Capturing/Processing).

### Connections and Pinout

| Pin | J2 (Programming) | J3 / J4 (Link) | GPIO | Description |
|---|---|---|---|---|
| 1 | 5V | GND | - | Common Ground |
| 2 | GND | TRIGGER | IO12 | Manual trigger / Wake-up |
| 3 | U0T (TX) | ERR_LED | IO13 | Error LED (Red) |
| 4 | U0R (RX) | STS_LED | IO15 | Status LED |
| 5 | IO0 | - | IO0 | Pull to GND for flashing |

## PCB Milling Specifications (Isolation Milling)
Optimized for CNC isolation milling on copper-clad boards.

### Manufacturing Parameters
* **Track Width:** 0.5 mm to 0.8 mm (Signals); 1.0 mm (Power).
* **Clearance (Isolation Gap):** 0.4 mm to 0.6 mm.
* **THT Pads:** Standard 2.0 mm outer diameter (1.0 mm drill). Custom footprints (e.g. SW1 DTR 7/RT) use 2.6 mm pads with 2.0 mm drills to maintain isolation gaps.
* **Thermal Reliefs:** Required for all pads in copper zones.
* **Antenna Keepout:** A dedicated copper-free area must be maintained under the ESP32-S PCB antenna to avoid WiFi signal attenuation.

### Milling Constraints
* **Tooling:** 30° Engraving Bit.
* **Depth of Cut:** 0.05 mm to 0.1 mm (Auto-leveling highly recommended).