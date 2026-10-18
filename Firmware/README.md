# Herox Firmware

This is the complete ZMK firmware for the Herox controller and PCB revision represented by the project schematic and board files.

The GitHub Actions build produces `Herox.uf2` as an artifact. This source archive does not include a prebuilt UF2 image.

## Hardware support

- 104-key ANSI matrix through two TCA9555 expanders
- Three stored Bluetooth host profiles with dedicated profile LEDs
- USB and Bluetooth HID with NKRO, media, and mouse reports
- EC11 encoder with press action
- 104 individually addressed SK6812MINI-E key LEDs
- DRV2605L LRA haptics with three persistent strength levels
- MAX17048 battery reporting
- Caps Lock and Num Lock indicators
- ZMK Studio over USB and Bluetooth
- Persistent lighting and haptic settings
- Idle and deep-sleep power management

The RGB rail exists only while USB is present. The firmware detects that condition, turns the strip off on battery power, and limits estimated LED current to 400 mA to stay below the board's 0.5 A lighting fuse.

## Main controls

Hold the Menu key to use the Control layer. Tapping the same key still sends Menu.

| Control | Action |
|---|---|
| Menu + F1 | Select Bluetooth host 1 |
| Menu + F2 | Select Bluetooth host 2 |
| Menu + F3 | Select Bluetooth host 3 |
| Menu + F4 | Clear the selected Bluetooth pairing |
| Menu + F5 | Use USB output |
| Menu + F6 | Use Bluetooth output |
| Menu + F7 | Toggle USB/Bluetooth output |
| Menu + F8/F9 | Previous/next RGB effect |
| Menu + F10/F11 | RGB brightness down/up |
| Menu + F12 | Toggle RGB |
| Menu + Print Screen | Unlock ZMK Studio |
| Menu + Scroll Lock | Soft power-off |
| Menu + Pause | Restart firmware |
| Menu + Escape | Enter UF2 bootloader |
| Menu + Grave | Toggle haptics |
| Menu + 1/2 | Haptic strength down/up |
| Menu + 3 | Test haptics |
| Menu + Minus/Equal | RGB hue down/up |
| Menu + Q/W/E | Previous track, play/pause, next track |
| Menu + A/S/D | Volume down, mute, volume up |
| Menu + Z/X/C/V | Undo, cut, copy, paste |
| Menu + arrow keys | Mouse movement |
| Menu + numpad 7/8/9 | Previous effect, RGB toggle, next effect |
| Menu + numpad 4/6 | RGB speed down/up |
| Menu + numpad 1/2/3 | Brightness down, RGB toggle, brightness up |
| Menu + encoder turn | RGB brightness |
| Encoder turn | Volume |
| Encoder press | Mute |

## Bluetooth pairing

Hold Menu and press F1, F2, or F3. Pair `Herox` from that device's Bluetooth settings. Repeat with the other profile keys for the other devices. The selected profile LED is solid when connected and blinks while waiting for a connection.

Keystrokes go to the selected profile. Switching profiles does not erase any of the other pairings.

## Flashing

1. Double-press the reset button on the XIAO nRF52840 to open its bootloader drive.
2. Copy the built `Herox.uf2` to that drive.
3. Wait for the board to restart.

Existing Bluetooth bonds and saved preferences remain in settings storage. Use Menu + F4 if a host needs to be paired again.

## ZMK Studio

Connect the keyboard by USB, open ZMK Studio, and press Menu + Print Screen when Studio asks to unlock the keyboard. The two shipped layers and three additional Studio layers are available for live remapping.

## Reproducible build

The repository pins ZMK to commit `268b1b1e82150460f00fd701bcd08583d5c75d29`. The included GitHub workflow builds the same XIAO nRF52840 ZMK variant with the `herox_x1` shield and USB Studio transport.

