# Nightlight Project - To-Do List 

## March 15, 2025
### This Time
- [ ] **Continue firmware development**
  - Refactor things into a "controls class" and add shaking functionality along with designing the firefly light show
- [ ] **Continue PCB review** 
  - Eliminate all errors and continue to look things over for potential improvements
  - Prep Fabrication files for odering
  - Find any ways to lower order cost (coupons, vias price,quanity)
- [ ] Finish prepping mouser order


### Last Time
- [X] **Continue firmware development** (focus on touch reliability)  
  - Improve **`CapTouch` class**  
  - _Got to the point where toggling the LEDS with the touch button is wrking well along with long presses_
- [x] **Confirm charge controller compatibility** with LiFePO4 batteries
  -_Did not work, decided to re-design with a  new charge controller designed for LIFEPO4_
- [x] **Continue PCB review**  
  - _Made a mistake and had components too close to PCB edge, had to redo layout_
    - _Moved components outside of area surrouding perimeter_
    - _Improved grounding by adding a Bottom Copper pour and dropping vias thoughout PCB, also made a keepout zone beneath the elctrode and kept traces to the elctrode as short as possible_
    - _Added a PPTC fuse between battery and regulator_


## March 11, 2025

### This Time
- [ ] **Continue firmware development** (focus on touch reliability)  
  - Improve **`CapTouch` class**  
- [ ] **Confirm charge controller compatibility** with LiFePO4 batteries 
- [x] **Continue PCB review**  

### Last Time  
- [x] **Build a new test board** (MCU + essential components)  
  - Test **LEDs, Touch, and Tilt switch**  
  - Serial communication issues persist on the current board  
- [ ] **Confirm charge controller compatibility** with LiFePO4 batteries  
- [ ] **Continue firmware development** (focus on touch reliability)  
  - Improve **`CapTouch` class**  
  - Draft an **updated flowchart** (on ferry)  
- [X] **Meet Phil (Friday)**  
  - Discuss **LED strips & USB-C pinout**  
- [ ] **Continue PCB review**  

## February 25, 2025  

### This Time  
- [ ] **Build a new test board** (MCU + essential components)  
  - Test **LEDs, Touch, and Tilt switch**  
  - Serial communication issues persist on the current board  
- [ ] **Confirm charge controller compatibility** with LiFePO4 batteries  
- [ ] **Continue firmware development** (focus on touch reliability)  
  - Improve **`CapTouch` class**  
  - Draft an **updated flowchart** (on ferry)  
- [ ] **Meet Phil (Friday)**  
  - Discuss **LED strips & USB-C pinout**  
- [ ] **Continue PCB review**  

### Last Time (February 21, 2025)  
- [x] **Decided to monitor battery voltage via MCU ADC**  
  - Voltage divider optional if direct measurement is unstable  
- [x] **First draft of PCB finished**  
  - Will get **Aaron/Phil to review** before ordering Rev2  
- [x] **Firmware project started**  
  - Initial tests for **onboard PTC** were inconclusive  

---

## February 21, 2025  

### This Time  
- [ ] **Test PTC and begin writing firmware R1**  
  - Start with a **flowchart or pseudo-code**  
- [ ] **Continue reviewing and editing PCB**  
  - Ask someone else to look it over  
- [ ] **Figure out how to attach LEDs to the board**  
  - Talk to Phil  
- [ ] **Look at inventory and create BOM for Rev2** (low priority)  

### Last Time (February 13, 2025)  
- [x] **Could not find a solution directly using the charge controller or regulator**  
  - Decided to monitor voltage using **MCU ADC** instead  
  - Added a voltage divider in case it's necessary  
- [x] **First draft of PCB completed**  
  - Plan to review it over the next week and get feedback from Aaron or Phil  
- [x] **Firmware project created**  
  - Initial tests for onboard PTC  

---

## February 13, 2025  

### This Time  
- [ ] **Find a solution to prevent over-drawing from the battery**  
  - Either monitor with MCU or use LVD in power supply/charge controller  
- [ ] **Begin PCB layout for Rev2**  
  - Reference Rev1 notes  
- [ ] **Start writing firmware**  
  - Figure out **PTC**  
  - Start with **flowchart or pseudo-code**  

### Last Time  
- [x] **Started working on Rev1 firmware**  
- [x] **Confirmed initial hardware functionality**  
 