# UNDERGROWTH · rhythm + ROOT study 02

A browser-first rhythm study: two original synthesized percussion voices and ROOT monophonic bass, a blank 16-step canvas, step locks and immediate performance controls. Nothing composes or plays by itself.

## Hardware milestone in progress

Every milestone must now produce a validated FM1 firmware package as well as this browser prototype. The panel controller and screen text have moved into shared C, and `firmware/` provides the physical-panel/audio adapter. Bluetooth and Wi-Fi are disabled. See [hardware build, Caravel 0.6.0 migration and recovery](docs/HARDWARE.md).

The host simulator and browser pass their checks, but the real device cross-build is currently blocked by compiler-download access. **No flashable `.fwsc` is ready yet.** The published Pages instrument remains the last completed browser release while this milestone is unfinished.

## Open it

1. Download and extract the package. Open **index.html** in a recent Chrome or Edge desktop browser. It contains its C/WASM engine and all interface assets; it has no external assets, analytics or remote calls.
2. Press **Enable audio**, with your listening volume low initially. This prepares audio but does not play anything.
3. Choose **Kick**, turn on steps 1, 5, 9 and 13, then press **Play**. Choose **Chatter** and plant a few contrasting steps. There is no built-in pattern.
4. Set **Swing** for the global pocket and **Human** for a little chatter movement. Both start at zero; no hits are generated.
5. Shape **Body**, **Edge**, **Length**, and **Send**. Send feeds a dark, filtered, tempo-linked delay with bounded feedback.

**Browser verification:** automated Chromium tests against a local HTTP server passed using the bundled index.html and real AudioContext/AudioWorklet output. Silent startup, empty Play, both live triggers, both sequencer lanes, locks, manual override/Rejoin, hit recording, Undo, Panic, keyboard triggers after button clicks and 320–680px layout checks passed. This is not a listening audition or verification of the public deployment. The standalone-file route remains unverified. If your browser rejects local-file AudioWorklet loading, serve the extracted folder locally:

```sh
python3 -m http.server 8000 --bind 127.0.0.1
```

Then visit `http://localhost:8000/index.html`. Keep the folder intact. The modular version is at `http://localhost:8000/web/`. No sign-in, hardware connection, build tools or installation is required to use the prebuilt files. On Windows, the Python command may be `py -m http.server 8000 --bind 127.0.0.1`.

There is no save/load yet. This is a **session-only sketch**: reload or close loses the pattern. Switching away stops playback for safety. Do not rely on this build to preserve work.

## Controls

- **Voice cards** select the lane and macros without triggering sound. **Tap kick/chatter** plays live.
- **A/S** or **Z/X** trigger kick/chatter. **1/2** select voices. **Space** toggles transport when focus is not in a form control or button. **Escape** silences everything. Native button Space/Enter remain accessible.
- **16 steps** toggle authored hits on the selected lane. Play on an empty lane/session is silent.
- **Edit step locks:** enabling this selects an existing hit when possible and releases live overrides so you can hear the locks. The panel beside the sound sliders names the voice, step and locked parameters. Choose a step, then move Body, Edge, Length or Send. Use **Enable hit** if that step is silent, and **Play loop** to hear it when playback reaches the step. Swing and Human remain global. A lock can exist on an inactive step without creating a hit. Disable the mode to edit the voice's base sound. Clear lock removes that step's locks.
- **Live override:** moving a base macro during playback temporarily takes priority over its step locks and also changes its base value. **Rejoin** releases the live override so stored locks govern again. Stop also releases overrides. Locks apply for their whole step, including the decay of a previous hit.
- **Swing:** global sixteenth-note swing, displayed as 50.0% (straight) to 66.7% (2:1 shuffle). Alternating subdivisions move later while each pair keeps its total duration. Both lanes and their step locks follow this grid.
- **Human:** sequenced chatter gets a fixed per-step delay of up to 8 ms and velocity softening of up to 12% at full strength. The same timing/accent shape repeats each loop; it does not regenerate random jitter. Kick timing/velocity and live taps are unaffected. Nothing adds or removes hits. This is an original groove treatment inspired by classic swing workflows, not an emulation of specific MPC hardware or templates.
- Both groove controls are global, outside step-lock mode, and covered by one-edit Undo/redo. Changes affect upcoming scheduling; already queued chatter keeps its current small delay. Clear rhythm retains the groove settings; reloading resets them. At Swing/Human zero, audio is bit-identical to the previous rhythm study at 44.1/48 kHz.
- **Record hits** arms quantized percussion overdubbing while playing. A live tap is heard immediately and marks the nearest step on the swung grid for future playback; existing other hits/locks remain. Use **Record motion** separately for per-trigger sound-slider overdubs. These two record modes are mutually exclusive. After arming while stopped, press Play and tap. Stop disarms.
- **Record motion:** plant hits first, arm, press Play, then move Body, Edge, Length or Send. Each existing sequenced hit captures the latest value of each control you moved on that voice as a step lock. A moved control continues writing its held value until you finish the take. Switch voices to capture controls on the other lane too. Untouched parameters, other locks, base sounds and rests are preserved. **Finish motion** or Stop releases the live override so the recorded locks govern playback. One Undo restores the entire take, and another redoes it. Arming without moving controls does not replace your previous Undo. Live taps are immediate but do not create motion locks. Swing, Human, tempo and master are outside motion recording. Step editing and global groove editing are unavailable during a take; Finish motion to use them.
- While playing outside lock edit, sound-slider readouts follow the effective sounding values, including recorded locks. In lock edit, they show the selected step.
- **Undo** swaps the last authored edit with its previous version (a second press redoes it). One level, including clear, step edits, macro drags and individual recorded hits. Not a history stack or a whole-take undo yet.
- **Stop** stops future events but allows current percussion and delay to decay. **Silence / Escape** immediately clears voices and delay, stops and disarms, without erasing the pattern.
- **Clear rhythm** clears both lanes and locks and silences. Undo restores the score but stays stopped.

## What this slice includes

- Original kick synthesis: swept sine core, short transient and controllable decay.
- Original click/chatter synthesis: short resonant tone, filtered noise and small per-hit burst clusters. Noise colors a hit you supplied; it does not create musical events.
- Sample-clock-owned two-lane 16-step sequencer, 40–220 BPM.
- Four step-lockable macros and explicit manual/lock/base precedence.
- Quantized live hit overdub, per-trigger sound-slider motion overdub, one-edit/whole-motion-take undo/redo, Stop and Panic.
- A fixed, dark eighth-note delay. Its buffer is bounded, so very slow tempos/high sample rates cap its length; this is not the complete future dub-effects engine.
- Original responsive cream/moss interface with accessible semantic controls.
- Same C core compiled native and WASM, plus a thin adapter successfully built in the pinned fm1-base native host.

## What is deferred

Additional percussion, evolving patterns, Keep/Revert, continuous gesture curves, probability/ratchets, session save/load, MIDI, polished hardware controls, firmware packaging and physical FM1 validation. No firmware was flashed. This is not a device-ready release.

## Source layout

- `core/undergrowth.c` / `.h`: authoritative audio, sequencing, authored score, locks, recording and undo. Fixed-size state, no runtime allocation or IO in the C render path.
- `web/`: thin JavaScript control/display and AudioWorklet wrappers. No independent JavaScript sequencer or DSP.
- `index.html`: self-contained build of those assets, including `web/core.wasm`.
- `tests/`: native invariant tests, deterministic rendering/native-WASM comparison, Node AudioWorklet adapter harness.

## Rebuild and verify

Requires Python 3, a C99 compiler, Node, and Zig 0.13.0 (used as the C→WASM compiler). The prebuilt HTML does not require them.

```sh
# Example toolchain install into a project-local directory, if desired:
python3 -m pip install --target .tools ziglang==0.13.0
ZIG="$PWD/.tools/ziglang/zig" sh tools/build.sh
./build/core_test
./build/groove_test
./build/motion_test
./build/root_test
node tests/parity.mjs
node tests/worklet.mjs
python3 tools/bundle.py
```

The build script also accepts `CC`. Zig produces freestanding WASM with no imports. Floating-point contraction is disabled on native and WASM builds to support deterministic comparison. Core/static state is approximately 93.8KB, including a fixed 88.2KB delay buffer. WASM's reserved memory is larger because of its stack and is **not** a device RAM budget. No hardware memory fit or real-time performance conclusion follows from these numbers.

## Verification status

Native invariant, native/WASM bit-parity and worklet-shim tests passed on the original study. The optional fm1-base adapter was separately validated but is not included in this browser-only publication candidate. Automated local Chromium runtime checks also passed as described above. There has been **no listening audition**, public deployment verification or physical hardware validation. This remains a rhythm study for first audition, not a finished instrument.

## Publication status

The original source licence has not yet been selected; see LICENSING.md. Hosting the study does not establish browser or hardware validation.

## ROOT bass

Select **ROOT** (or press **3**) for its independent 16-step note lane. Click a step to select it, enable **Note on**, then choose a pitch (C1–C4), gate and slide. Unchecked steps are rests. Note labels show the phrase on the grid. All three lanes share transport, tempo and Swing; Human continues to affect chatter only.

Hold the on-screen keys, or **Q W E R T Y U I**, to play C2 through C3 in C major. Releasing the current key releases the monophonic voice; the most recently pressed note wins. These are live controls, independent of step selection. **Record hits** also overdubs live bass pitches onto the nearest swung step, using that step's gate and slide settings; held duration is not recorded yet.

Gate spans 10–150% of the step. Above 100%, notes can overlap into the next step. **Slide** on the destination note glides from a still-sounding preceding note at the rate set by ENV Glide (2–202 ms time constant); otherwise pitch starts directly. **Body** adds fundamental weight, **Edge** adds rounded upper harmonics, **Length** controls release, and **Send** feeds the shared dub delay. This first voice is an additive harmonic bass; filter-envelope synthesis is a future sound-design extension.

ROOT uses the same per-step locks, manual override/Rejoin, motion overdubs and whole-take Undo as percussion. Motion records only controls you move, on existing bass notes. Stop releases bass; Panic silences it immediately. Startup remains empty and silent. Patterns remain session-only, and hardware fit/performance remains unverified.

## FM1 device view

The default view now uses the open-source fm1-base emulator panel template (revision and licence in LICENSING.md): eight physical knobs including Master, fourteen buttons, 27 keys and a 240×240 non-touch screen. Drag knobs vertically, scroll on them, or focus and use arrow keys/Page Up/Page Down. The previous interface is available through **Expanded view**. Both views use the same running C/WASM engine and score.

- ALGORITHM / SEL: choose Kick, Chatter or ROOT. SELECT switches subpages on HOME, SEQ, ENV and FX.
- EDIT / HOME: draft Body, Edge, Length and Send. Playback locks do not change this draft. HOME's second page offers Undo/redo, Rejoin and Panic: KNOB1 selects, SEL applies. Hold HOME for Panic; Escape also silences.
- SEQ: the sixteen lower keys address steps for every voice. An empty step captures the sound macros, ENV controls and draft velocity (ROOT also captures draft pitch/gate/slide). An occupied step is selected without alteration. Hold a step and turn a sound knob to edit its lock. SELECT opens explicit Stamp, Erase and Undo actions; KNOB1 chooses, SEL applies.
- ENV page 1: Kick Length/Damping/Drop/Response; Chatter Stretch/Fracture/Grain/Length; ROOT Release/Glide/Weight/Response. All are track editable, trig lockable and motion recordable. Response directly changes onset firmness as well as scaling the subtle velocity colour.
- ENV page 2 (SELECT): draft velocity for percussion; ROOT pitch/gate/slide/velocity on KNOB1–4. ROOT shows note names, defaults to C2 / 75% / no slide, and its keyboard enters the draft pitch while auditioning. SEL auditions without recording.
- SEQ pages (SELECT): sound/stamping, actions, selected-step pitch/gate/slide/velocity, selected-step ENV locks. Percussion uses KNOB1 for velocity on page 3. SEL auditions on that page. Editing an empty step on pages 2–4 preserves its rest; only the stamping page creates hits.
- REC: arms the selected track and opens its screen. KNOB1 chooses Hits, Motion or Both; changing modes finishes the current take, so press REC again to arm the new mode. REC/SEL finishes a take. PLAY starts/stops independently of the current screen. Stop and Panic disarm.
- Hits/Both: keys play live hits, quantized into the armed track while transport runs. Motion: move macros to overdub their values on existing hits. Both captures touched macros on new and existing hits. One Undo restores the whole take. Switching the viewed track does not change the armed track; its name stays on the REC screen. Other-track panel keys/knobs are ignored until the take finishes.
- Motion-only SEQ keys select without stamping. Navigate to EDIT or FX to record their controls. The untouched parameters, rests and draft sound remain intact.
- GLO: tempo, Swing and Human on KNOB1–3. FX: track delay send on KNOB1, lockable and motion-recordable. SELECT shows the fixed global delay's status; adjustable bus parameters and a global FX lane are deferred.
- LFO, ARP, SAVE and PRESETS have explicit reserved/unavailable screens. Sessions still disappear on reload.
- Keys play the selected voice on sound pages. ROOT follows the physical keybed’s semitone ordering, starting F1 by default; OCT± shifts an octave. Notes outside ROOT’s current C1–C4 range are silent. On narrow screens the panel scrolls horizontally to preserve control target size.

This validates a proposed control mapping, not hardware parity: the browser drawing/input wrapper is JavaScript; menu logic, screen text, audio and sequencing now live in shared C. The new FM1 adapter has host tests; its device memory map, CPU performance and flashing package still require cross-build and hardware validation. No firmware has been flashed.

## Environment and organism DNA

The [roadmap and accepted design decision](docs/ROADMAP.md) define **strong intentional controls, subtle hard-wired DNA**. Chatter now uses synthetic grain-like fractures: Stretch spreads them without repitching, Fracture sets density, Grain shapes hardness and fragment duration, and Length scales their decay. This is synthesis, not a sample player. Velocity and tempo add bounded reactions without modifying authored controls.

All tracks have draft and per-trig velocity (0–100%). Stamping captures it; live recording preserves supplied velocity; ordinary taps use the draft. Human scales playback velocity without rewriting it. Velocity, pitch, gate and slide are event attributes, edited separately from continuous ENV motion. In Expanded view, the velocity slider follows Edit step locks: off edits draft strength, on edits the selected trig. New ENV sliders share the same lock/motion editor.

The screen controller remains browser-only. Session saving, expanded global effects, punch-in effects and hardware validation remain on the roadmap; no firmware has been flashed.
