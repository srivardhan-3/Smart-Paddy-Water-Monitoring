# Smart Paddy Field Water-Level Monitoring and Irrigation Support

An ESP32 prototype for measuring paddy-field water level with a JSN-SR04T waterproof ultrasonic sensor and viewing readings through a local Wi-Fi dashboard.

## What is included
- ESP32 Arduino firmware with ultrasonic distance measurement.
- Water-level calculation using a configurable sensor mounting height.
- Local web dashboard hosted by the ESP32.
- Automatic threshold-based pump-control output and manual ON/OFF controls.
- Optional SIM800L SMS alert code (disabled by default).
- Hardware notes, test checklist and an illustrative architecture diagram.

## Hardware used / planned
- ESP32 development board
- JSN-SR04T ultrasonic sensor
- Optional relay driver / contactor interface
- Optional SIM800L GSM module and separate suitable power supply
- Battery pack and regulated DC converters
- Optional local display (not implemented in this firmware version)

## Measurement calculation
`water_level_cm = SENSOR_MOUNT_HEIGHT_CM - measured_distance_cm`

Set `SENSOR_MOUNT_HEIGHT_CM` to the physically measured distance from the sensor reference plane to the chosen zero-water datum. The example default is 100 cm only as a starting configuration; measure your actual installation and calibrate against a staff gauge.

## Firmware configuration
Open `Firmware/Paddy_Water_Level_Monitor.ino` and update:
1. `WIFI_SSID` and `WIFI_PASSWORD`.
2. GPIO assignments to match your wiring.
3. `SENSOR_MOUNT_HEIGHT_CM`.
4. `LOW_LEVEL_THRESHOLD_CM` and `HIGH_LEVEL_THRESHOLD_CM` after deciding your field's safe operating range.
5. Set `ENABLE_PUMP_OUTPUT` to `true` only after testing with a low-voltage indicator and verifying the driver circuit.
6. SIM800L is optional. Keep `ENABLE_SIM800L` false unless it is wired and powered correctly; update the destination phone number before enabling.

The ESP32 starts a local access point if it cannot join the configured Wi-Fi. The serial monitor prints the dashboard address.

## Important limitations
- This is generated prototype firmware, not a certified irrigation controller.
- Ultrasonic readings can be affected by sensor angle, turbulence, condensation, mounting, and target geometry.
- The earlier ±0.1 cm accuracy goal is not guaranteed. Validate the complete installation against a physical reference and document measured error.
- The pump output is a low-voltage control signal only. Do not connect a mains pump directly to an ESP32 or small relay module.

## Safety
- Verify the JSN-SR04T module version and ECHO voltage. ESP32 GPIOs are not 5 V tolerant; use a suitable level shifter/divider if needed.
- Use an isolated, properly rated contactor/driver, overload protection, earthing and enclosure for any mains pump. Have mains wiring checked by a qualified electrician.
- SIM800L needs a separate stable supply capable of its current bursts. Do not power it from the ESP32 3.3 V pin.
- Never publish real Wi-Fi passwords, phone numbers, API keys or tokens in a public repository.

## Upload
Upload this folder's contents to your GitHub repository. Replace the illustrative diagram with actual photographs and a verified schematic when available.
