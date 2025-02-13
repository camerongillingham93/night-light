# Nightlight Rev 1 Notes
### February, 13, 2025

## Assembly  Notes

- Get stencil and use hot plate for next order, voltage regulator was very challenging to hand solder and bridged many times before successfully soldering. 

- Was missing 10K 1210 SMT resistors from order, R1,R2,R6

- Need to think of a better method for attaching LED strips or use different part, they are very thin and challenging t

## Testing

Voltage regulator - PAM2401SCADJ
- Once soldered succesfully, swept the Vin range given by the data sheet and had a steady 5V output

Charge controller - MCP73831-2-OT
- Discharged a 3800 mAh 3.7 li-ion to 3.6V 
- scraped open the Vbatt trace leading to the charge controller to measure current
- Set up meters to measure Vbatt and Ibatt
- plugged in USB cable and monitored output 
- I out was 100mA and stayed at the current until Vbatt was 4.2V, At this point Ibatt started to lower until ~60mA and maintained a voltage of 4.2. This is as expected with a 10K resistor on PROG pin. 

MCU ATtiny1614-SS
- Flashed the ATTINY via serial UPDI with a cp2021 USB-UART module
- Instructions @ https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md
- tested by placing a LED across switch pin and running a BLINK script.
- for development i have broken out the serial pins on the MCU. USB to UART is solderded to a dedicated perf board that can switch between UPDI to program and serial comms for troubleshooting. 

## Next time TODO

- Ground was missed on charge controller IC
- Add decoupling caps to MCU
- add 0 OHM R to separate regulator and charge controller for testing
- Use a bi-colour LED on the STAT pin of the charge controller
- Use ADC on MCU to measure Vbatt and disable nightlight at low voltage threshold and prompt charge somehow, OR find a solution to use a pin from the power supply for some sort of LVD
- find a better USB-C port that actually uses CC resistors so product will work with PD
- breakout serial pins for comms with the board