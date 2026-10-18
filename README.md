# HeroX

HeroX is my custom full-size mechanical keyboard project, built around a **Seeed Studio XIAO nRF52840**.

so basically i wanted a full-size board that could do Bluetooth and normal USB, but i also wanted to go further than just switches on a PCB.. per-key RGB, haptics, a media knob, battery tracking, multiple Bluetooth profiles, a proper 4-layer board and a custom case. i designed the electronics in KiCad, worked on the case in Fusion 360 and put together the firmware side too, which turned into a much bigger rabbit hole than expected lol

![HeroX PCB and switch layout](Images/Screenshot%202026-10-18%20102543.png)

## why i made this

i wanted a keyboard that i could actually use as a daily driver without giving up the full-size layout, numpad, navigation cluster or function row, but i didnt want it to be just another giant boring rectangle either

main goals :

- design a full-size keyboard PCB in KiCad
- support Bluetooth and wired USB-C
- scan the keyboard matrix using I2C GPIO expanders so the small XIAO controller has enough pins
- add addressable SK6812 MINI-E RGB lighting
- include a media encoder, haptic feedback and battery monitoring
- make a proper 4-layer PCB intended for manufacturing
- design a custom case in Fusion 360
- keep the project files, production references, images and BOM together so the build is reproducible

the fun part is that everything affects everything else.. a small PCB change can affect case clearances, connector access and firmware pin assignments. thats the kind of detail that makes a custom keyboard project way more involved than it looks from the top

## project status

HeroX is documented here as a design project with CAD, PCB screenshots, firmware-related screenshots and a parts list. the hardware still needs to be assembled and tested before i can claim that every feature works on a finished physical board, so treat this as a design/build project rather than a fully validated retail product

whats in this repo :

- `3D CAD/` — exported 3D model files
- `BOM Images/` — supplier and component sourcing screenshots
- `Firmware/` — firmware-related project files
- `Fusion/` — Fusion 360 design files
- `Images/` — PCB, schematic, CAD and firmware screenshots
- `PCB/` — KiCad PCB design files
- `PDFs/` — exported project documents, if present
- `Production/` — manufacturing outputs, if present
- `BOM.csv` — parts list, supplier links and estimated prices

## hardware summary

| | |
|---|---|
| Project | HeroX |
| Layout | Full-size mechanical keyboard |
| Controller | Seeed Studio XIAO nRF52840 |
| Connectivity target | Bluetooth LE and wired USB-C |
| Matrix support | I2C GPIO expanders (PCA9555-compatible parts listed in BOM) |
| Switch diodes | 1N4148W SOD-123 |
| RGB | SK6812 MINI-E addressable LEDs |
| Battery monitoring | MAX17048 fuel-gauge IC listed in BOM |
| Haptics | DRV2605L haptic-driver IC listed in BOM |
| Controls | Rotary/media encoder shown in the project design |
| Switches | Akko V3 Creamy Blue Pro 5-pin MX switches |
| Keycaps | 104/108-key ANSI PBT set |
| Stabilizers | DUROCK V3 100% PCB-mount kit |
| PCB | 4-layer board quote listed in the BOM |
| CAD | Fusion 360 and exported 3D files |

some of the interesting engineering is hidden underneath the keycaps.. the PCB has to fit the whole matrix, lighting and support circuits, while the case needs to line up with the switches, ports and controls. the CAD and PCB images below show the project from a few different angles

## images

### PCB and switch layout

![HeroX PCB and switch layout](Images/Screenshot%202026-10-18%20102543.png)

### PCB routing and board layers

![HeroX PCB routing view](Images/Screenshot%202026-10-18%20102603.png)

![HeroX PCB routing view](Images/Screenshot%202026-10-18%20102608.png)

![HeroX PCB layer view](Images/Screenshot%202026-10-18%20102614.png)

![HeroX PCB view](Images/Screenshot%202026-10-18%20102620.png)

### schematic

the schematic screenshots show the keyboard matrix and the supporting circuits used across the design

![HeroX schematic](Images/Screenshot%202026-10-18%20102647.png)

![HeroX schematic detail](Images/Screenshot%202026-10-18%20102652.png)

![HeroX circuit schematic](Images/Screenshot%202026-10-18%20102655.png)

![HeroX schematic overview](Images/Screenshot%202026-10-18%20102659.png)

### firmware and build screenshots

![HeroX firmware screenshot](Images/Screenshot%202026-10-18%20102751.png)

![HeroX firmware build screenshot](Images/Screenshot%202026-10-18%20102757.png)

![HeroX firmware output screenshot](Images/Screenshot%202026-10-18%20102813.png)

### CAD and case

![HeroX CAD assembly](Images/Screenshot%202026-10-18%20195431.png)

![HeroX case underside](Images/Screenshot%202026-10-18%20195437.png)

![HeroX CAD case view](Images/Screenshot%202026-10-18%20195447.png)

![HeroX case interior](Images/Screenshot%202026-10-18%20195503.png)

![HeroX PCB plate or case plate](Images/Screenshot%202026-10-18%20195538.png)

![HeroX control module and knob](Images/Screenshot%202026-10-18%20195559.png)

## repo file checklist

the folder names in the current project are:

- `PCB/` — KiCad design files
- `3D CAD/` — 3D exports
- `Fusion/` — editable Fusion design files
- `Firmware/` — firmware files
- `Production/` — manufacturing outputs
- `PDFs/` — PDF exports
- `Images/` — project screenshots used in this README
- `BOM Images/` — sourcing screenshots
- `BOM.csv` — complete bill of materials

the exact filenames inside each folder may change as the project gets updated, so open the folders to find the latest versions. this README intentionally uses the visible folder structure rather than guessing filenames that may not exist in the repository

## cad

the case was designed in Fusion 360. the `Fusion/` folder is intended for the editable design, while `3D CAD/` contains exported geometry for viewing, modifying or preparing for printing. check the available formats in those folders before starting a print or manufacturing order

the case and PCB need to be checked together before fabrication: switch alignment, board mounting, connector access and clearance around the controls all matter. use the supplied CAD and PCB files as the source of truth for the current revision

## firmware note

the project includes a `Firmware/` folder and firmware/build screenshots. the controller listed in the BOM is the Seeed Studio XIAO nRF52840, and the design includes components associated with wireless operation, RGB, battery monitoring and haptic feedback

firmware features and flashing steps should be confirmed against the actual files in `Firmware/` before building. i dont want to claim a particular key combination, flashing image or feature is ready until it has been checked against the committed firmware revision

## bom

the full bill of materials is in [`BOM.csv`](BOM.csv). it includes the part description, quantity, supplier, supplier link and listed price in rupees. supplier prices and stock can change, and some quantities reflect supplier pack sizes or minimum order quantities rather than the exact number installed on the keyboard

**listed BOM total: ₹36,382.32**

a few notable parts from the BOM:

| Part | Purpose |
|---|---|
| Seeed Studio XIAO nRF52840 | Main controller |
| PCA9555DWR I/O expanders | Expand available GPIO for the keyboard matrix |
| 1N4148W SOD-123 diodes | Per-switch matrix diodes |
| SK6812 MINI-E | Addressable RGB lighting |
| MAX17048 fuel gauge | Battery monitoring |
| DRV2605L | Haptic driver |
| Akko V3 Creamy Blue Pro switches | Mechanical key switches |
| DUROCK V3 stabilizer kit | Stabilization for larger keys |
| 104/108-key ANSI PBT set | Keycaps |
| 4-layer PCB | Main keyboard circuit board |

prices are the values currently captured in the BOM, not a guaranteed final build cost. the total can change with shipping, taxes, revised quotes, spare parts and supplier minimums. check the links in `BOM.csv` before ordering anything

## build notes

- inspect the PCB and schematic before ordering a board
- verify the exact component substitutions and footprints against the BOM and KiCad files
- check the CAD exports against the current PCB revision before printing the case
- use a suitable protected battery and verify polarity and connector wiring before connecting it
- test the board in stages instead of powering every subsystem at once
- confirm firmware configuration and flashing instructions from the files in `Firmware/`

## contributing / questions

this is a personal custom keyboard project, and im sharing the design files and build notes so the work is easier to understand and build on. if you spot an issue or have a suggestion, feel free to open an issue on GitHub

thanks for checking out HeroX.. this project is a mix of PCB design, CAD, sourcing and embedded work, and its been a pretty great way to learn how all those pieces fit together :)
