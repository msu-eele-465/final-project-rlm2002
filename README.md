# EELE 465 Final Project

## Introduction

This project is a small code breaker game using a binary number and two RGB LEDs. The player starts the game by pressing a button. A countdown will begin, and the LCD display shows the current time. The code is entered using a membrane keypad to select the LED color and a dipswitch to select the number. The player can check their pattern at any time during the game by pressing the button again. A status led will flash as the game progresses, and will flash faster when there are 10 seconds left. If the player does not enter the code in time, the buzzer will go off and a game over message is shown on the lcd. A win message is shown if the correct code is entered. The user may press the button to restart the game and play again.

## Architecture
There are a total of 6 external devices: LED Bar, LCD, an MSP430fr2310, a 4x4 membrane keypad, a PB-12N32MP-05AQ buzzer, a button, dipswitch, two RGB LEDS, and status led. The C code contains functions for all LEDs, getting the dipswitch value, and the buzzer as well as keypad and LCD modules. Below is a broad visualization of the program software architecture.

<img src="docs/assets/final_project_concept-software_arch.drawio.svg" alt="architecture" style="width:50%; height:auto;"> 

## Flowchart
On startup, the lcd displays "Push to Start" on the LCD display. This message will change on a button press to start the game. The player will press the keypad to change the colors of the RGB LEDs. If the button is pressed again, the code is checked, showing a win message on the lcd if correct. Otherwise the game continues until the timer runs out, showing a game over message. The status led will flash faster when there are ten seconds left in the game.

<img src="docs/assets/final_project_concept-flowchart.drawio.svg" alt="flowchart" style="width:75%; height:auto;"> 

## Circuit diagram
Below is the circuit diagram for the project.

<img src="docs/assets/final_project_concept-circuit_diagram.drawio.svg" alt="circuit diagram" style="width:50%; height:auto;">


## Repo structure

- [📁 `docs`](docs): documentation
- [📁 `controller`](controller): The CCS project for the main MCU.
    - [📁 `app`](controller/app): C files for RGB LEDs, Keypad, and Controller.
    - [📁 `src`](controller/src): Header files for RGB LEDs and Keypad.
- [📁 `i2c-lcd`](i2c-lcd): The CCS project for the I2C LCD Display.
