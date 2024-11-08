# picow\_infrared\_relay

## Overview

Running on a Raspberry Pico W, this will listen on UDP for standard infrared 38khz timings and then light
up an infrared led to emit a matching signal.

I wanted to control my Yamaha soundbar from the command line. I can run "up\_volume.sh" on my desktop to
send a UDP packet over wifi to a $6 Pico W, which then sends the appropriate "up volume" signal to my speaker
as if it were an infrared remote control.

## Hardware required

The total hardware cost should be under $10.

This works on the Raspberry Pi Pico W, which currently costs $6. It could likely work on any RP2040 hardware
supported by the official Pico SDK.

In addition to the Pico W, you'll need an infrared LED, a resistor and some wire. You'll need to solder wires to your Pico
if you don't have pins installed.

I bought my LED from adafruit. It's currently available as "super-bright 5mm IR LED - 940nm" for $0.75. You can connect
it to any ground pin and a GPIO pin through a resistor.
Any GPIO pin should work if set in config.h. I used GP22 "PIN 22" (actual pin 29).

You can test the LED to make sure it's wired in the correct direction by tapping the GPIO side to 3V3 (through the resistor!)
and looking at it with a smartphone camera. The camera will pick up the IR light and make it visible.

I used a 500 Ohm resistor, but others will work too. If I were to do it again,
I'd try a 1000 Ohm. Some people recommend using a transistor to get additional
power to the LED. Without a transistor, I had an effective range of 6ft, which
was much more than I needed so I didn't use one.

You'll also need a power supply for the Pico W. A standard micro-usb power supply will work.

## Software required

This is written in C and is meant for the official Raspberry Pi Pico SDK. The SDK is available on github
and is fairly easy to install:

```bash
cd /opt # Assuming you install to /opt/pico_sdk
git clone https://github.com/raspberrypi/pico_sdk.git --branch master
cd pico_sdk
git submodule upgrade --init
```

You'll also need some packages. For Raspberry Pi OS, they recommend installing:
```bash
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Then, you can download the project files into a directory, assuming /opt/pico\_sdk:
```bash
git clone https://github.com/sanjayrao77/picow_infrared_relay
cd picow_infrared_relay
ln -s /opt/pico_sdk/export/pico_sdk_import.cmake .
mkdir build
cd build
export PICO_SDK_PATH=/opt/pico_sdk
cmake ..
```

At this point, you'll want to edit config.h for your wifi password and IP settings. You can use
a static IP (recommended) or a dynamic IP with dhcp. You might want to change the UDP\_PASSWORD8
to another 8-character password. This password would then be set in picow\_infrared\_helper to match.

After your config.h has been set, you can compile:
```bash
cd picow_infrared_relay
cd build
make
```

Hopefully that makes picow\_irserver\_background.uf2 and picow\_irserver\_poll.uf2. You might
as well use picow\_irserver\_background.uf2 but the poll version might use less electricity.

To copy to the Pico W:
```bash
mount -t auto /dev/sda1 /mnt/sda1
cp picow_irserver_background.uf2 /mnt/sda1/
sync
```

## BUGS

On early versions, I had trouble with long-term stability. It would run for weeks and then stop. The problem
may have been solved in the initial release but it could take months to find out.

## Sending IR Codes

I have another project on my github called picow\_infrared\_helper. That program holds all the information
about IR codes and sends it to this program over UDP. With this design, if you want to add new IR commands you don't
have to change anything on the Pico.

I have another project called rpi\_infrared\_reader. That is written for the Raspberry Pi, not the Pico. I've
only used it on a RPI 4B but it should work elsewhere. When running properly with an IR sensor attached, it
can read IR timing from a physical remote control and save it in a way that can be imported into picow\_infrared\_helper.

The three programs together act as a learning remote, over wifi.
