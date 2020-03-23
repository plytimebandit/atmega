# ATmega

This repo contains code for the ATmega328PU.


## setup

[ATmega328PU](https://www.microchip.com/wwwproducts/en/ATmega328P) connected via [FT232RL](https://www.ftdichip.com/Products/ICs/FT232R.htm) to USB port (FTDI FT232RL, USB to TTL).


## make and upload code

1. Create config file: `make configure`
1. Compile: `make`
1. Upload: `make upload`

Compilation and upload can be performed at once: `make all`.

The upload might need sudo rights for accessing the USB device.


## avrdude programmer config

For using the default config file `~/.avrduderc` needs to contain a specific programmer config:

    programmer
      id    = "arduino_mymod";
      desc  = "Arduino";
      type  = "arduino";
      connection_type = serial;
    ;
