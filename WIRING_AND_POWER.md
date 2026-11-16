# Hardware and Wiring Guide

This is a starting reference. Confirm exact module revision and voltage levels before wiring.

## Example signal connections
| Module signal | ESP32 GPIO | Notes |
|---|---:|---|
| JSN-SR04T TRIG | GPIO 5 | 3.3 V logic output |
| JSN-SR04T ECHO | GPIO 18 | Ensure signal is reduced/level-shifted to a safe ESP32 input level |
| Relay-driver input | GPIO 26 | Firmware physical output disabled by default |
| SIM800L TXD | GPIO 16 (RX2) | Optional; voltage compatibility required |
| SIM800L RXD | GPIO 17 (TX2) | Optional; use suitable level shifting |

## Power
- Power ESP32 from a stable regulated supply.
- Power SIM800L from a separate supply designed for its current bursts; never use the ESP32 3.3 V pin.
- Keep grounds common for low-voltage UART/signals where required by the module design.
- Confirm the sensor supply requirement from the exact JSN-SR04T version.

## Pump control
The firmware GPIO is only a control signal. Use a properly rated isolated driver and contactor, overload protection, earthing and enclosure for a mains pump. Mains wiring must be installed/checked by a qualified electrician.

## Sensor mounting
Measure the distance from the sensor face to the selected zero-water datum. Set that measured value in `SENSOR_MOUNT_HEIGHT_CM`. Avoid aiming at a moving stream, vegetation or sloped surface.
