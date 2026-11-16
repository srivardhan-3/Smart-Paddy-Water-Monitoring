# Prototype Test Plan

## 1. Compile and bench test
- [ ] Install ESP32 board support in Arduino IDE.
- [ ] Open `Paddy_Water_Level_Monitor.ino`.
- [ ] Update Wi-Fi settings and GPIO pins.
- [ ] Compile for the exact ESP32 board.
- [ ] Upload with pump output disabled.
- [ ] Open Serial Monitor at 115200 baud.
- [ ] Confirm dashboard IP or fallback access-point details.

## 2. Ultrasonic sensor calibration
- [ ] Confirm ECHO voltage is safe for ESP32.
- [ ] Compare readings against a ruler at multiple distances.
- [ ] Record distance, reference distance and error.
- [ ] Measure actual sensor mounting height.
- [ ] Validate calculated water level against a staff gauge in still water.
- [ ] Repeat under realistic field conditions.

## 3. Dashboard and control
- [ ] Open dashboard using the IP printed in Serial Monitor.
- [ ] Confirm live distance and water-level readings.
- [ ] Test manual mode with pump output disabled.
- [ ] Test automatic threshold logic with an LED/low-voltage test load only.
- [ ] Verify invalid sensor readings force the output OFF.

## 4. Optional SIM800L
- [ ] Verify module supply voltage and peak current capability.
- [ ] Confirm SIM registration and SMS operation.
- [ ] Enable SMS only after bench testing.
- [ ] Keep the phone number private in a local configuration file if repository is public.

## 5. Field validation
Record test date, weather, sensor mounting, water-surface condition, reference level, measured level and error. Do not claim ±0.1 cm accuracy unless repeatable validation supports it.
