# Source provenance and licensing

The C musical core, web interface, adapters, tests and build scripts in this browser-only tree were created for UNDERGROWTH. No public source licence has been selected for these original files. Publication alone must not be described as an open-source licence grant. A licence decision remains with the project owner.

No JIANT, Felucca-Engines, SLOOP or proprietary M-Vave DSP was copied into the musical core. There are no bundled third-party runtime libraries, fonts, images or remote assets. WebAssembly and Web Audio are browser platform APIs. Zig 0.13.0 and GCC were used as build tools, not redistributed executables.

The separate native-host experiment used fm1-base, pinned to commit 02f422c7d9de121ca099bb2fdebe5c035f59e8ad:
https://github.com/charlesvestal/fm1-base/tree/02f422c7d9de121ca099bb2fdebe5c035f59e8ad

That upstream source, its adapter, host-generated font assets and test outputs are intentionally excluded from this browser-only tree. The prior private development package preserves the upstream source and licence notices. Any later inclusion of those components requires preserving their applicable notices and complying with their licences.
