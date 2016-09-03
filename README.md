# altair-mqtt-sn-client-example

An example of an Aquila 2.0 MQTT-SN client implementation for the [Altair](http://www.aquila.io/en) development board, designed to work with [Aquila MQTT-SN Gateway](https://github.com/Rodmg/aquila-mqtt-sn-gateway).

## Requirements

For compiling and uploading this code to the board you need the PlatformIO tools and the [PlatformIO IDE](http://platformio.org/get-started).

You also need to install the Altair board support for PlatformIO (WIP).

## Using

First, clone this repo **with all its submodules**:

```
git clone --recursive https://github.com/Rodmg/altair-mqtt-sn-client-example.git
```

Open the project folder from the Atom IDE with PlatformIO IDE plugin installed, connect your board and press the upload button.

Use a serial monitor to see the program output.

## Customizing and understanding the code

Read and edit ``src/main.ino``. I hope the comments are enough for understanding the basic MQTT-SN functionality.
