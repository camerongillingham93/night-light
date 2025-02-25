# TODO 
### Febreuary 25, 2025
## This Time
1. Build a new board to write firmware with as Serial comms has stopped working with current setup
    - Just solder MCU and necessary components to test LED's, Touch, And tilt switch
2. Confirm if the chosen charge controller will work with LIFEP04 batteries
3. Continue firmware DEV prioritizing writing a reliable touch class
    - Work on overall flowchart on ferry
4. see fil on friday and chat
     - LED strips and USB-C pinout
5. Continue with PCB review
## Last Time
1. Confirmed utilizing PTC will not work without ATmels software and not going to venture down that path
     - Have been playing arounf with different methods of writing my own touch library with varying levels of success.
     confident this will be fine
2. Have been reviewing, will continue
3. havent been to see fil yet
4. not necessary at this point

### February, 21, 2025
## THIS TIME
1. Test PTC and begin writing firmware R1 
    - start with a flow chart or pseudo-code
2. Continue reviewing and editing PCB
    - ask someone else to look ino over 
3. Figure out how to attach LED's to board 
    - (Talk to Phil)
4. Look at inventory and create BOM for 2nd revision
    - Low priority

## Last Time 
1. Could not find a solution directly using the charge controller or regulator, have decided to monitor witht he ADC of the MCU. Will add a volatge divider incase its necessary but i belive i will be able to meaure directly witht he ADC. 
2. Have finshed a first draft of the PCB, and will review it over the next week or so,and then ask Aaron or Phil to take a look. Then it will sit until i have MONEYS to order the next revision
3. Have created  a project for the firware and included some files to tets out the onboard PTC

### February, 13, 2025
## THIS TIME
1. Find a solution to prevent over drawing from battery
    - EIther monitor with MCU or LVD in power suplt/charge controller
2. Begin on PCB layout for rev2
    - Reference rev1 notes
3. Start writing firmware
    - Figure out PTC
    - start with flow chart? 