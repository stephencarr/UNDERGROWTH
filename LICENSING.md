# Source provenance and licensing

Except for the explicitly identified fm1-base files and Clouds-derived reverb below, the C musical core, interface, adapters, tests and build scripts were created for UNDERGROWTH. No public source licence has been selected for these original files. Publication alone must not be described as an open-source licence grant. A licence decision remains with the project owner.

No JIANT, Felucca-Engines, SLOOP or proprietary M-Vave DSP was copied into the musical core. The Clouds-derived reverb identified below is the only incorporated third-party DSP. There are no bundled fonts, images or remote assets in the browser instrument. The device view includes the panel UI code identified below. WebAssembly and Web Audio are browser platform APIs. Zig 0.13.0 and GCC were used as build tools, not redistributed executables.

The separate native-host experiment used fm1-base, pinned to commit 02f422c7d9de121ca099bb2fdebe5c035f59e8ad:
https://github.com/charlesvestal/fm1-base/tree/02f422c7d9de121ca099bb2fdebe5c035f59e8ad

The pinned upstream source remains an external build dependency. The experimental firmware package includes the generated font assets and applicable licence notices; it does not bundle the upstream source tree. Any distribution of those components must preserve their applicable notices and comply with their licences.

## FM1 panel template

`web/fm1-panel.js` and `web/fm1-panel.css` are adapted from Charles Vestal’s `fm1-base/web/emu/index.html`, revision `02f422c7d9de121ca099bb2fdebe5c035f59e8ad`. Copyright (C) 2026 Charles Vestal. Those files and their modifications are distributed under MPL-2.0; the full licence is in `LICENSES/fm1-base-MPL-2.0.txt`. Their source is published alongside the application at https://github.com/stephencarr/UNDERGROWTH/tree/main/web. The generated `index.html` incorporates those components as part of a larger work and retains their notices. This does not select a licence for UNDERGROWTH’s original files.

Changes extract the drawing, accessible input targets and encoder interaction from the emulator template, and connect them to application callbacks instead of fm1-base’s firmware transport. The original UNDERGROWTH adapter in `web/device.js` renders its menus and forwards commands to the existing shared C/WASM engine. No SLOOP code or assets are incorporated.

## Hardware target

The hardware build now uses the same pinned fm1-base source above and felucca-bsp `cd76d43ab4b94d6da9a8f1f24f3f4446135196b7` (MPL-2.0). They remain external, unmodified build dependencies. `firmware/app.c`, `app.h`, and `core/controller.c` are original UNDERGROWTH adapters/controller, not copied template implementations. Screen drawing calls the upstream platform API. Its generated Barlow fonts carry SIL OFL 1.1; the three SDK package assets carry Apache-2.0. Their exact versions/hashes are recorded in `firmware/dependencies.json`.

`tools/firmware_build.py` copies the applicable MPL, font and SDK licence texts with the experimental package and identifies upstream source revisions in its manifest. Original UNDERGROWTH code still has no selected public source licence. No Caravel source has been copied: its 0.6.0 installation guide is referenced only for the user's migration route. The experimental package is distributed with the browser flasher and its licence notices; it has not been installed on hardware.

## Browser firmware installer

`web/vendor/fm1-base/fm1ports.js`, `fm1pkg.js` and `fm1ota.js` are unmodified copies from fm1-base revision `02f422c7d9de121ca099bb2fdebe5c035f59e8ad`, distributed under MPL-2.0 with their original source headers. `flash.html`, `web/flash.js` and `web/flash.css` are original UNDERGROWTH integration code. The `.fwsc` in `firmware/releases/` is the experimental binary from the pinned hardware build; the source and build dependency identities are recorded in its release manifest.

## Mutable Instruments Clouds reverb

`core/room.h` adapts `clouds/dsp/fx/reverb.h` and its `FxEngine` allpass semantics
from Mutable Instruments eurorack commit
`f4cfa7697095a28d292bff758beb51deb49b5a86`, Copyright 2014 Emilie Gillet:
https://github.com/pichenettes/eurorack/tree/f4cfa7697095a28d292bff758beb51deb49b5a86/clouds/dsp/fx

Those portions and adaptations are MIT licensed. The complete upstream notice
is in `LICENSES/Mutable-Instruments-MIT.txt`, retained in `core/room.h`, embedded
in the standalone HTML and copied alongside the experimental firmware. Changes
use explicit C float circular buffers, rate-scaled lengths, tank-delay modulation,
new bounded decay/damping mappings, aux return/width and instrument lifecycle.
This is a topology adaptation, not a bit-exact Clouds emulation. No JIANT,
FM1-DUB, X0X or Plaits implementation is copied. UNDERGROWTH's other original
files retain their existing undecided licensing status.
