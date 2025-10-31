Nano Fan Controller — THT + Screw Terminals (Arduino Nano)
Rev B — 2025-10-30

WHAT'S INCLUDED
- KiCad 6/7 project: Nano_Fan_Controller_THT.kicad_pro / .kicad_sch / .kicad_pcb
- BOM: BOM_THT_ScrewTerm_Nano.csv

FOOTPRINTS / EXTERNAL CONNECTORS (all THT screw terminals)
- J1 (motor): Phoenix MKDS 3.5mm 1x03 — pins: 1=+24V, 2=GND, 3=PWM
- J2 (VIN 24V): Phoenix MKDS 3.5mm 1x02 — pins: 1=+24V, 2=GND
- J3 (rotary): Phoenix MKDS 3.5mm 1x05 — pins labeled C,H,M,L,B on silks
- J4 (optional buck header): 2x02 2.54mm — VIN-, VIN+, VOUT-, VOUT+
- TVS: P6KE33A axial
- MOSFET: 2N7000 TO-92
- All resistors axial DIN0207, electrolytics radial, C1 ceramic disc

ROUTING STATUS
- Basic placement and net assignments are present. Please open in KiCad -> Assign footprints (already set) ->
  Update PCB from schematic -> Finish routing (suggest top-layer traces, 0.8mm power, 0.3–0.4mm signal).
- Pour a GND plane on F.Cu if desired.
- Label silks: +24V/GND/PWM on J1; +24V/GND on J2; C/H/M/L/B on J3.

GERBERS
- In KiCad PCB Editor: File → Plot…
  • Format: Gerber, Layers: F.Cu, B.Cu (if used), F.SilkS, F.Mask, B.Mask, Edge.Cuts
  • Drill: Generate Drill Files → Excellon, check "Merge PTH and NPTH"
  • Zip outputs for your fab.
Note: This environment can't run KiCad to auto-plot Gerbers; the board is ready to plot on your machine.

NOTES
- Keep 470µF caps close to their rails; place TVS near J1.
- Keep the PWM trace short and away from the buck inductor if you mount one.
- Verify screw-terminal orientations before ordering (3.5 mm pitch).

