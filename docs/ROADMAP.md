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

## Next slice — complete the organism palette

Stephen's next priority is the remaining organisms, following Kick, Chatter and ROOT. Complete a useful basic sound palette before adding further features and polishing the instrument. Preserve musically productive overdub behaviour during this slice; only critical defects should interrupt it.

### Immediate slice — Backbeat

A fourth independent organism. Stephen rejected the first skin/noise version as too narrow after audition. The revised direction covers shakers → snares → claps, with continuous blends and surprising mashed-up, extreme textures in between. Body sweeps those families; Edge opens brightness into crushed crackle and metallic fragments; Length extends grains, skin and diffuse tails; Send feeds the existing delay. ENV provides Length, Damping, Spread (grain density/spacing, rattles and clap flams) and Response (attack firmness). These are bounded components of one authored hit, not generated sequencer events. Timbre transitions smooth inside the DSP without rewriting locks. This is original synthesis, without samples. Preserve trig locks, velocity, motion and Undo. Browser and matching experimental FM1 firmware are required. Further listening approval remains with Stephen; no additional sequencing features or noncritical lock cleanup is implied.

### Following Backbeat: chord stabs and a flexible FM tone

Stephen chose Backbeat as the immediate next slice, followed by chord stabs and the flexible FM organism. Working labels below describe the future roles; final organism names are open. Build Backbeat first, then the chord organism, then the FM organism. Together with Kick, Chatter and ROOT, these establish the basic sound palette before broader features and polish.

**Chord organism — always chords.** Morph from rounded, filtered dub stabs through bright, hollow 90s dance/house/rave chord colours, with long, swelling and sustained ambient articulations. Ambient capability belongs in the sound and articulation, rather than depending entirely on a future reverb. Timbre morphing and harmonic selection are separate: changing the colour need not replace the chosen chord. No single-note/OFF chord mode or silent fallback to a root-only voice is part of this organism.

- Configure a small session-only chord palette. Proposed starting scope: eight editable slots, each combining a chord shape with voicing/inversion and spread. Start with useful built-in shapes; include the ability to author a slot's intervals, within a bounded voice budget.
- Keys choose chord roots; palette slots supply the harmonic shape. Chord slot/shape, root, inversion and spread can vary per trig alongside the sound and ENV locks. A trig captures its actual chord definition, not just a mutable palette-slot reference: later palette changes must not silently rewrite existing trigs.
- Use intentional sound controls with a broad musical range, plus articulation suitable for both short stabs and slow ambient chords. Velocity, motion, Undo, Stop and Panic follow the existing shared workflow. Keep at least two distinct chord tones through range handling and timbre morphing; bounded polyphony and retrigger/tail behaviour need audition and FM1 budget checks.

**FM organism — warm and bright, with a wide morphing range.** A coherent playable FM voice spanning melodic/plucked/bell-like tones, inharmonic and atonal textures, sustained drones, very short micro-sounds and grainy/fractured articulations. Design it to receive pitched note events from keys, the sequencer and the later arp. Arp implementation is still a later feature.

- Expose independent musical dimensions for colour/brightness, harmonic-to-inharmonic structure, feedback/roughness, and temporal behaviour. These are continuous sound-design dimensions, not merely a list of presets on one morph knob. Final knob names and page allocation are to be chosen through playing.
- Articulation should reach from tiny bursts through plucks and evolving sustained tones. Grain-like behaviour means synthesised temporal fragmentation in this slice; no sample/granular playback engine is implied. Warm melodic settings should remain easy to find without preventing more extreme settings.
- Trig locks and motion must reach the defining synthesis and articulation controls, so successive steps can produce strongly contrasting colours within one organism. Pitch/root and note attributes stay distinct from sound morphing. Bounded/smoothed parameter changes, note-off and Panic remain required; the CPU/voice budget must be checked in the matching firmware.

### Caravel 0.6.0 chord reference

Reference revision: `b50bad57dcf32862f36661a5fc2996e13392b93e`, `firmware/src/chord.c` and the CHORD/SCL controls in `firmware/src/params.c`:
https://github.com/bonszaimusic/caravel/blob/b50bad57dcf32862f36661a5fc2996e13392b93e/firmware/src/chord.c

The inspected implementation offers diatonic triads/sevenths plus fixed major, minor, dominant seventh, major seventh, minor seventh, sus4 and power shapes. It separates CLOSE/OPEN/inversions/added-bass voicing from chord quality, with ROOT/SCALE mapping and NEW/STACK latch elsewhere in the input model. Its chord builder uses at most four tones; mono/legato/unison modes can reduce a chord to its root. UNDERGROWTH takes the separation of harmony and voicing as design guidance, adds an explicitly editable palette, and keeps its dedicated chord organism chordal. Caravel's inspected code does not establish the proposed eight-slot editable bank; that is our own design proposal. No Caravel source has been incorporated.

### Slice contract

Each new organism has its own recognisable synthesis identity, useful sound macros and ENV controls, velocity response, and the existing draft/step/lock/motion workflow. Implement it in shared C and finish each slice with the browser and matching experimental FM1 firmware. Audition it with the existing organisms. New sequencing modes, arp evolution, expanded global effects and general UI polish follow the basic palette. Accept noncritical prototype debt; only critical defects interrupt this sequence.

## Implemented workflow pass — hardware first

Stephen requested an overnight study and overhaul of the whole workflow, starting at the human hardware interface. [UX_DESIGN.md](UX_DESIGN.md) records the external UX/manual research, design choices, physical mapping and task acceptance scenarios. This is a design hypothesis to play, not a usability-study result.

HOME becomes a four-track session mixer; lower keys 1–4 toggle mutes. SEQ retains sixteen step keys and tap/hold semantics; upper keys expose tracks, typed full-trig copy/paste, Undo, Rejoin, audition and lock-page navigation. SEL auditions rather than ambiguously choosing a track. EDIT adds a quick articulation page. Selected steps and section pages are remembered per track. REC mode changes retain the same armed owner; finishing preserves the working page. Device and Expanded views share that recorder/mix. Key labels and light masks come from shared C. No saving, arp, new effect or additional organism is implied. Physical testing and Stephen’s workflow acceptance remain open; chord stabs and FM remain next.

## Later — controlled ROOT arp evolution

Inspired by the bounded movement of Caravel 0.6.0's WALK arp. Start with a small played/latched note set; neighbour-note wandering, adjustable rests and a restricted register keep the phrase recognisable. Hold freezes a pleasing phrase; Keep captures it into the sequencer; Revert returns to the seed. ROOT comes first, with related Chatter rhythm/articulation exploration later. Implement original behaviour in shared C for browser and firmware. This is a roadmap item, not part of the current step-editing fix.

## Current interaction fix — tap steps, hold for trig locks

A short SEQ tap toggles its step on/off while preserving authored trig data; only a previously untouched step captures the draft. Holding for 350 ms opens the single trig-lock menu, without toggling or creating a hit. SELECT switches Sound, ENV and Notes / velocity; SEQ closes it. The menu remains open after releasing the key. Remove competing SEQ actions/lock pages and Expanded view's separate lock-mode switch. Expanded steps use the same hold gesture. Rest edits preserve rests; canceled presses never toggle; whole menu edits on one step support Undo.

## After the organism palette — effects

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

First hardware milestone: USB-only, single core, no Bluetooth/Wi-Fi. The host/browser tests, pinned cross-build and package validation passed; `build/releases/0.1.0-hw1/` contains an experimental `.fwsc`, manifest, checksums, build report and installer. The browser flasher is published as experimental; the firmware has not been flashed or tested on an FM1. See [hardware build and recovery notes](HARDWARE.md), including the Caravel 0.6.0/macOS route and remaining physical acceptance tests.

Hardware diagnostics added to this pending milestone: persistent audio-budget CPU meter, observed peak and cumulative overruns on every display page, updated at 4 Hz. The browser explicitly marks device CPU unavailable; no simulated load is presented as hardware headroom.
