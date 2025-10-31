Gerber & Drill Export — Nano Fan Controller THT (Rev B — 2025-10-30)
==============================================================
Open `Nano_Fan_Controller_THT.kicad_pcb` in KiCad PCB Editor (v6 or v7):

1) Update PCB from Schematic (if you changed anything in the .kicad_sch).
2) Inspect footprint pin numbers and net assignments; adjust any tracks to pad pins.
3) Right-click the GND copper zone outline → "Fill or Refill All Zones".
4) File → Plot…
   Format: Gerber
   Layers to plot:
     [x] F.Cu
     [ ] B.Cu (leave unchecked to stay single-sided)
     [x] F.Mask
     [x] F.SilkS
     [x] Edge.Cuts
   General: Check "Use Protel filename extensions".
5) Click "Plot".
6) Click "Generate Drill Files…"
   • Excellon format
   • Check "Merge PTH and NPTH holes"
   • Units: Millimeters
   • Zeros format: Suppress leading zeros
   Then "Generate Drill File".
7) Zip the plotted Gerbers + drill files and upload to your fab.

Notes:
- This project is set up for single-sided (top) routing. Keep all traces on F.Cu.
- Power traces (+24V, GND) use 0.8 mm width in the sample segments; adjust as needed.
- Keep the TVS diode and 24 V bulk cap close to the motor screw terminal.
- Verify screw terminal footprints match your parts (Phoenix MKDS 3.5mm or similar).
