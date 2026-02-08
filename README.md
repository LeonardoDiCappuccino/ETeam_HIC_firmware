# ETeam HIC Firmware

This is the firmware sourcecode for the ETeam HIC, based on [DAPLink](https://daplink.io/) V0257 (Copyright © 2006-2023 Arm Ltd)
It has been adapted to meet requirements and fixed related bugs. Due to the fragility of the project generator and modifications,
DapLink V0257 currently the only working release so DO NOT UPDATE!

# Requirements

- Python 3.9 (newer won't work, older not tested)
- GNU Make (only Version 4.4.1 tested)
- Arm GNU Toolchain (only Version 14.2 tested)

# Installation

Clone:
```
git clone ../DapLink_ETeam.git
cd DapLink_ETeam
```

Install [virtualvenv](https://virtualenv.pypa.io/en/latest/user_guide.html) with Python 3.9:
```
python3.9 -m pip install virtualenv
python3.9 -m virtualenv venv
```

Install requirements in the virtual environment (venv):
```
venv/Scripts/activate.bat
pip install -r requirements.txt
```

Fixing bugs under windows (recommended):
```
cp .\makefile.tmpl .\venv\Lib\site-packages\project_generator\templates\makefile.tmpl
cp .\libusb-1.0.dll .\venv\Scripts\libusb-1.0.dll
```

# Building

## Bootloader (bl)

The Bootloader get's flashed only once via SWD onto the HIC and enables an easy way to flash different Interface Firmware onto the HIC

Make sure the venv is activated:
```
venv/Scripts/activate.bat
```

Building Bootloader with ``progen``:
```
progen generate -t make_gcc_arm -p stm32f103xb_bl -b
```
- ``-t <tool>`` toolchain to build (only ``make_gcc_arm`` is working)
- ``-p <project>`` project to build (in this case the bootloader ``stm32f103xb_bl``)
- ``-b`` building flag

## Interface Firmware (if)

The Interface Firmware is the actual firmware and there are two types of it
- **generic:** Contains features like CMSIS-DAP(flashing, debugging), CDC(UART communication)
- **specific:** Same features as generic + specific to one target MSC support (drag-n-drop flashing)

Make sure the venv is activated:
```
venv/Scripts/activate.bat
```

Building generic Interface Firmware with ``progen``:
```
progen generate -t make_gcc_arm -p stm32f103xb_generic_if -b
```
- ``-t <tool>`` toolchain to build (only ``make_gcc_arm`` is working)
- ``-p <project>`` project to build (in this case the generic one ``stm32f103xb_generic_if``)
- ``-b`` building flag

Building specific Interface Firmware with ``progen``:
```
progen generate -t make_gcc_arm -p stm32f103xb_<target>_if -b
```
- ``-t <tool>`` toolchain to build (only ``make_gcc_arm`` is working)
- ``-p <project>`` project to build (e.g. ``stm32f103xb_stm32f411re_if``)
- ``-b`` building flag

For available specific Interface Firmware refer to ``projects.ymal``.
Feel free to add further ones ([Reference](https://github.com/ARMmbed/DAPLink/blob/main/docs/PORT_BOARD.md))

# Flashing

## Bootloader

1. Wire up a ST-Link, (working) ETeam HIC, etc. to the "Firmware SWD" pins of the unprogrammed HIC
2. Use...
    - STM32CubeProgrammer
    - pyOCD:
        ```
        pyocd install stm32f103cb   # only if not already installed
        pyocd pyocd flash --erase chip -t stm32f103cb .\projectfiles\make_gcc_arm\stm32f103xb_bl_crc.hex
        ```
    - etc.
    to flash the bootloader executable (``projectfiles/<target>/stm32f103xb_bl_crc.hex`` or ``.bin``)
3. Now you are ready to flash the Interface Firmware

## Interface Firmware

1. Plug in the HIC via USB into a computer, while **shorting the "Firmware" jumper** with tweezers or so
2. A drive called "MAINTENANCE" should pop up. There you just drag-n-drop the wished Interface Firmware(``projectfiles/\<target\>/stm32f103xb_<target or "generic">_if_crc.hex`` or ``.bin``)
3. Power cycle the HIC