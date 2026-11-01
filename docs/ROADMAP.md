# UNDERGROWTH roadmap

## Decision: strong controls, subtle organism DNA

Accepted with Stephen, 2026-11-01. ENV means **Environment**, not a conventional ADSR editor. Each organism has an opinionated identity that remains recognisable. The player makes the large musical decisions; the organism adds character in its response.

- ENV knobs must have a pronounced, useful effect by themselves. Locks and motion retain that intentional control.
- Parameter precedence is track draft/base → stored trig lock → manual takeover (including motion performance). Motion writes touched values back into the trig locks; finishing/Rejoin releases takeover. DNA then interprets the sounding values.
- Hard-wired DNA is bounded, deterministic and smaller than the control's useful range. It may respond to velocity, tempo and recent musical context. It never rewrites knobs, locks, velocity or motion data, and never invents notes.
- Example: stretching Chatter must clearly spread its fragments apart. A stronger hit may produce slightly denser fractures; it must not undo the chosen stretch.
- Velocity is authored per-trig strength for every organism. Stamping captures draft velocity; live recording stores input velocity. Ordinary browser taps use draft velocity. Human varies playback around the stored strength without editing it.
- No full ADSR, sample player, automatic composition, or new save format is implied by this decision.

## Current milestone — environment and articulation

Implemented for the browser prototype; listening approval and hardware validation remain open.

| Organism | ENV controls | Hard-wired response |
|---|---|---|
| Kick | Length, Damping, Drop, Response | Velocity adds up to 0.06 to/subtracts from Edge, scaled by Response; tempo changes decay by at most 6%; closely spaced retriggers shorten recovery by 4%. |
| Chatter | Stretch, Fracture, Grain, Length | Tempo changes fragment spacing/decay by at most 6%; velocity shifts secondary-fragment count by at most one. Deterministic uneven spacing gives the fractures a tactile shape. |
| ROOT | Release, Glide, Weight, Response | Velocity adds up to 0.06 to/subtracts from Edge, scaled by Response. Slide still requires an overlapping note and the destination's Slide flag. |

Response on Kick and ROOT directly controls onset firmness over a pronounced range; its small pressure-colour modulation is additional. Chatter uses synthetic noise/resonant fragments, not recorded samples. Stretch changes their spacing rather than oscillator pitch. Fracture sets density, Grain sets hardness/duration, and Length sets each fragment's decay. These are auditionable controls, not a claim of acoustic realism. ROOT's gate/slide/pitch and all tracks' velocity live on a separate note/articulation page; continuous ENV parameters support track edits, locks and motion.

Acceptance: silent startup and empty transport; distinct audible/measured ENV extremes; full draft stamping including ENV/velocity; per-step ENV/velocity edits and Undo; motion captures new ENV controls without creating hits; recorded velocities survive playback/Human; ROOT note names, direct draft pitch entry and audition; native/WASM parity and actual browser AudioWorklet output.

## Next — effects

1. Track filters with cutoff/resonance, and separate delay/reverb sends.
2. Global delay time, feedback, tone and return; reverb size, damping and return.
3. Silent global FX automation lane: one owner for shared-bus automation, independent of instrument hits. Bounded feedback, smoothing, Panic and whole-take Undo.

## Later — performance, device parity and persistence

- Dedicated sixteen-key punch-in effects page. Explore momentary throws, repeats, filter moves and reverb blooms. Black-key Hold/latch and modifiers are proposed; decide mappings through playing tests. They must not take over SEQ's keyboard contract.
- Port the browser controller/screens to the FM1 foundation and verify physical controls, CPU/RAM, audio and note-off behaviour. Shared C DSP/sequencing already underlies the browser; the browser UI remains JavaScript. No flash is authorized by this milestone.
- Versioned session saving once parameter ownership settles. Stable IDs and migrations will still be necessary; feature completion cannot eliminate schema evolution.

## Validation recorded before publication

- Native core, groove, motion, ROOT, workflow and environment/velocity tests at 44.1/48 kHz.
- Native/WASM bit identity over 262,144 stereo frames, including ENV motion and velocity events.
- Address/undefined-behaviour sanitizer stress test (leak detection disabled because the test runner uses ptrace; the DSP performs no allocation).
- Chromium AudioWorklet output checks: silent start/empty transport, live voices, sequencing, locks, motion, ROOT, Undo and Panic. New mouse/touch tests cover ENV/velocity stamping and editing, ROOT pitch entry/audition, and longer output from stretched Chatter. Expanded controls and recording-command ordering are also covered.
- These checks were run locally before publication. They do not establish subjective listening approval, Safari coverage or FM1 hardware validation.

## Milestone contract: firmware is a required output

Accepted with Stephen on 2026-11-01: every future milestone must end with a real, validated FM1 `.fwsc` alongside the browser. Shared C now owns the panel controller and screen text as well as DSP/sequencing. `AGENTS.md` and `tools/check_milestone.sh` make the target explicit. Host/WASM success alone cannot complete a milestone; compiler or package-check failures must remain visible blockers.

First hardware milestone: USB-only, single core, no Bluetooth/Wi-Fi. The host/browser tests, pinned cross-build and package validation passed; `build/releases/0.1.0-hw1/` contains an experimental `.fwsc`, manifest, checksums, build report and installer. It is a local draft and has not been flashed or tested on an FM1. See [hardware build and recovery notes](HARDWARE.md), including the Caravel 0.6.0/macOS route and remaining physical acceptance tests.

Hardware diagnostics added to this pending milestone: persistent audio-budget CPU meter, observed peak and cumulative overruns on every display page, updated at 4 Hz. The browser explicitly marks device CPU unavailable; no simulated load is presented as hardware headroom.
