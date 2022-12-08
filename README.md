# Traffic
### A bluetooh and wi-fi connected red light/green light handler.
> A traffic light script for the [ESP32](https://www.espressif.com/en/products/socs/esp32) boards.

#### Features

- Current commands:
  - Red Light/Green Light
  - Traffic simulator
  - Random light selection
  - Flash red/yellow/green/all
  - Shuffle between all commands
  - Stop/End/Off
- Automatically turns lights off if they've been on too long
- Controls accessible over:
  - wi-fi via HTTP request
  - Bluetooth
- Maintains responsiveness by not using `delay()`

##### Language(s)

- Arduino (C/C++)

##### Depends

| External Libraries | License |
| --------- | --------- |
| N/A | N/A |

##### Instructions

- Load into Arduino IDE of your preference
- Select ESP32 board
- Compile/Upload
- Play with traffic

### License
[License](LICENSE.md)