# Traffic
### A bluetooth and wi-fi connected red light/green light handler.
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

## Web Bluetooth app
> Built and intended to be deployed via a separate webserver

> [!WARNING]
> This does require additional enablement. This is largely available on Chromium-based browsers.
> Additional details: https://developer.chrome.com/docs/capabilities/bluetooth

> Can potentially be included as a js/index file to be served from an ESP32 if space is not an issue


#### Features

- Direct control of Traffic light via Bluetooth in browser

- Enable/Disable commands noted for ESP32 project
- Bluetooth connection updates process and enabled lights visually
- Directly control individual lights
  - Tapping/Clicking on a light will enable/disable it
  - Toggles below process selection allow you to override all lights or process

##### Instructions
- Load `webapp` directory
- Run `npm install`
- To run local development application:
  - Run `npm run dev` to start a webpack HTTPs server
- To build JS file to be used elsewhere
  - Run `npm run build`
  - `app.js` and `index.html` files will be saved to `dist/` directory

##### Language(s)

- React/NodeJS/Javascript

##### Depends

| External Libraries | Version |
| --------- | --------- |
| NodeJS | v22.8.0 (tested) |

### License
[License](LICENSE.md)
