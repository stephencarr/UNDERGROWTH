# UNDERGROWTH roadmap

Updated 2 November 2026. Current published build: **experimental 0.4.3-hw1**.
Published instrument: https://stephencarr.github.io/UNDERGROWTH/ . Implementation,
listening acceptance and physical FM1 validation are separate statuses.

## Direction and order

Complete a useful organism palette before broader features and general polish.
Kick, Chatter, ROOT and Backbeat are implemented. **The chord organism is the
current development slice, followed by the flexible FM organism.** Audition the new
reverb and ROOT/ENV workflow alongside this work; only critical defects should
interrupt the palette-first priority. Accepted noncritical debt stays deferred.

| Order | Work | Status |
| --- | --- | --- |
| Shipped | Four organisms, shared controller/workflow, ROOT Character and independent ENV, shared reverb and delay return | Published experimental; acceptance checks below remain open. |
| Current slice | Chord organism: dub stabs, bright house/rave chords and sustained ambient textures | 0.5.0-hw1 candidate implements eight editable slots, captured harmony and two four-tone tail banks; acceptance remains open. |
| Following chords | Flexible FM organism with a distinct role beyond ROOT | Planned; final name, controls and voice budget remain open. |
| After the palette | Track filters, separate delay/reverb sends and further delay controls | Planned. Existing reverb controls and delay return are already implemented. |
| Then | Silent global FX automation lane | Planned; one owner for shared-bus automation. |
| After the palette; exact order open | Per-track trig-lock scenes and morphing | Requested; up to four scenes per track as the starting scope. |
| After the palette; exact order open | Complete playful, exploratory UI overhaul | Requested; Teenage Engineering is a design reference, with an original UNDERGROWTH identity. |
| Later | Controlled ROOT arp evolution and punch-in effects | Planned; mappings and sequencing details require playing tests. |
| Later | Versioned session saving and other reserved workflows | Deferred; current sessions disappear on reload. |

Physical FM1 acceptance is outstanding work across these milestones, not a
controller port that still needs implementing. See [HARDWARE.md](HARDWARE.md).

## Decision: strong controls, subtle organism DNA

Accepted with Stephen, 2026-11-01. ENV means **Environment**, not a conventional
ADSR editor. Each organism has an opinionated identity that remains recognisable.
The player makes the large musical decisions; the organism adds character.

- Intentional controls must produce pronounced, useful changes. Trig locks and motion retain that control.
- For parameters that support locks, precedence is track draft/base → stored trig lock → manual takeover. Finishing a take or Rejoin releases takeover. DNA interprets the sounding values without rewriting authored data.
- DNA is bounded, deterministic and smaller than the control's useful range. Velocity, tempo or musical context may colour a supplied event; they never invent notes or rewrite knobs, locks or velocity.
- Velocity is authored per trig. Human varies playback around the stored strength without editing it; Human currently affects Chatter only.
- ROOT ENV is an independent continuous atmosphere. Ordinary notes do not snapshot its settings or either layer's send. Deliberate legacy atmosphere/send locks still exist; their presence does not make per-note ENV automation the intended drone workflow.
- This direction does not imply a general ADSR editor, automatic composition, sample import or a save format. The shipped ROOT trial uses an original generated texture, not an imported-sample workflow.

## Shipped — published experimental

### Organisms and Environment

| Organism | Current role and controls | Bounded response |
| --- | --- | --- |
| Kick | Synthesised kick; ENV Length, Damping, Drop, Response | Velocity colours Edge, scaled by Response; tempo changes decay by at most 6%; closely spaced retriggers shorten recovery by 4%. |
| Chatter | Synthetic grain-like noise/resonant fractures; ENV Stretch, Fracture, Grain, Length | Tempo changes fragment spacing/decay by at most 6%; velocity shifts secondary-fragment count by at most one. |
| ROOT | Monophonic bass with EDIT Sound/Character; ENV Position, Grain size, Density, Pitch, then Level, Scan, Enabled, ENV send | Bass velocity adds bounded colour; ENV remains an independently pitched continuous drone. |
| Backbeat | Continuous shaker → snare → clap blends; Edge reaches crushed crackle and metallic fragments; ENV Length, Damping, Spread, Response | Velocity/tempo add bounded colour and decay response. Grains/flams belong to a supplied hit, not extra sequencer events. |

ROOT's note pitch, gate, slide and velocity remain separate articulation data.
Sound/Character settings support trig locks and motion. Ordinary ROOT note entry
captures synth slots 0–10 except Synth send; ENV and sends require deliberate
locks rather than implicit snapshots. ENV runs with Enabled ON, nonzero Level
and PLAY, including on an empty sequence. STOP fades it; ROOT mix/mute affects
both layers. Following bass-note pitches is only a possible optional future
mode, not a change to the accepted independent ENV behaviour.

### Workflow and shared controller

The musical engine, panel controller, input/edit semantics and screen text are
implemented in shared C for browser and firmware. Browser JavaScript and the
FM1 adapter handle their respective IO/drawing. The controller port is complete;
physical ergonomics and device behaviour still need acceptance.

Shipped workflow includes four 16-step lanes, persistent trig-lock menus,
velocity and ROOT articulation, Hits/Motion/Both takes, takeover/Rejoin,
whole-take Undo, mix/mutes, typed full-trig copy/paste, audition, and per-track
page/step memory. SEQ taps toggle while preserving authored data; a 350 ms hold
opens locks without toggling or enabling a rest. REC retains its armed owner
across navigation. See [UX_DESIGN.md](UX_DESIGN.md) for current controls and
accepted noncritical workflow debt.

### Effects and routing

Shared effects now include a Clouds-derived modulated diffuse reverb with
Reverb level, Decay, Damping and Width, plus a fixed dark delay with independent
return. Existing track sends and ROOT's separate Synth/ENV sends feed both in
parallel. **Separate sends for delay versus reverb are still planned.**

At full Reverb, fully sent layers lose their direct contribution; unsent layers
retain it. FX shows effective ROOT sends during playback. Explicit send locks
and motion remain; ordinary ROOT notes no longer restore an implicit zero.
Global FX settings have score Undo and refuse edits during recording; no global
FX automation lane exists yet. Stop/mute preserve shared tails; Panic clears
them. See [REVERB_EVALUATION.md](REVERB_EVALUATION.md) for source, comparisons,
validation and publication evidence. Listening acceptance remains open.

## Current slice — chord organism (0.5.0-hw1 candidate)

**Chord organism — always chords.** Morph from rounded, filtered dub stabs through bright, hollow 90s dance/house/rave chord colours, with long, swelling and sustained ambient articulations. Ambient capability belongs in the sound and articulation, rather than depending entirely on the shared reverb. Timbre morphing and harmonic selection are separate: changing the colour need not replace the chosen chord. No single-note/OFF chord mode or silent fallback to a root-only voice is part of this organism.

- Configure a small session-only chord palette. Proposed starting scope: eight editable slots, each combining a chord shape with voicing/inversion and spread. Start with useful built-in shapes; include the ability to author a slot's intervals, within a bounded voice budget.
- Keys choose chord roots; palette slots supply the harmonic shape. Chord slot/shape, root, inversion and spread can vary per trig alongside the sound and ENV locks. A trig captures its actual chord definition, not just a mutable palette-slot reference: later palette changes must not silently rewrite existing trigs.
- Use intentional sound controls with a broad musical range, plus articulation suitable for both short stabs and slow ambient chords. Velocity, motion, Undo, Stop and Panic follow the existing shared workflow. Keep at least two distinct chord tones through range handling and timbre morphing; bounded polyphony and retrigger/tail behaviour need audition and FM1 budget checks.

### Caravel 0.6.0 chord reference

Reference revision: `b50bad57dcf32862f36661a5fc2996e13392b93e`, `firmware/src/chord.c` and the CHORD/SCL controls in `firmware/src/params.c`:
https://github.com/bonszaimusic/caravel/blob/b50bad57dcf32862f36661a5fc2996e13392b93e/firmware/src/chord.c

The inspected implementation offers diatonic triads/sevenths plus fixed major, minor, dominant seventh, major seventh, minor seventh, sus4 and power shapes. It separates CLOSE/OPEN/inversions/added-bass voicing from chord quality, with ROOT/SCALE mapping and NEW/STACK latch elsewhere in the input model. Its chord builder uses at most four tones; mono/legato/unison modes can reduce a chord to its root. UNDERGROWTH takes the separation of harmony and voicing as design guidance, adds an explicitly editable palette, and keeps its dedicated chord organism chordal. Caravel's inspected code does not establish the proposed eight-slot editable bank; that is our own design proposal. No Caravel source has been incorporated.

### Budget and slice acceptance

Each new organism needs a recognisable identity, useful sound/ENV controls,
velocity response and the existing draft/step/lock/motion/Undo workflow in shared
C. Finish each slice with the browser and matching experimental FM1 firmware,
and audition it with the existing organisms. Arp, new sequencing modes and
noncritical cleanup do not enter the palette slice by default.

The 0.4.3-hw1 build uses **312,908 of 344,064 pool bytes**, leaving **31,156 bytes**.
Data+bss is 40,572 of 98,304 bytes. These are separate memory regions; pool space
alone is not a total voice budget. Physical CPU headroom is unmeasured.
Establish chord voice count, note overlap/tail policy, score storage and DSP
memory/CPU budgets early, then validate the matching dense-pattern build.
Do not assume two additional organisms or more grain buffers fit merely because
the current four-track browser plays. Host timings are not FM1 measurements.

## Following chords — a flexible FM organism

**Warm and bright, with a wide morphing range.** A coherent playable FM voice spanning melodic/plucked/bell-like tones, inharmonic and atonal textures, sustained drones, very short micro-sounds and grainy/fractured articulations. Design it to receive pitched note events from keys, the sequencer and the later arp. Arp implementation is still a later feature.

ROOT already has fixed-topology FM Character controls for its bass role. This planned organism must earn a separate lane through a broader register and useful melodic, bell-like, inharmonic and fragmented articulations. Define and audition that distinction before finalising its controls; expanding ROOT's macro list alone does not complete this organism.

- Expose independent musical dimensions for colour/brightness, harmonic-to-inharmonic structure, feedback/roughness, and temporal behaviour. These are continuous sound-design dimensions, not merely a list of presets on one morph knob. Final knob names and page allocation are to be chosen through playing.
- Articulation should reach from tiny bursts through plucks and evolving sustained tones. Grain-like behaviour means synthesised temporal fragmentation in this slice; no sample/granular playback engine is implied. Warm melodic settings should remain easy to find without preventing more extreme settings.
- Trig locks and motion must reach the defining synthesis and articulation controls, so successive steps can produce strongly contrasting colours within one organism. Pitch/root and note attributes stay distinct from sound morphing. Bounded/smoothed parameter changes, note-off and Panic remain required; the CPU/voice budget must be checked in the matching firmware.

## Later — effects, automation and performance

### Effects after the palette

1. Track filters with cutoff/resonance, and independent delay/reverb send amounts.
2. Global delay time, feedback and tone; preserve the implemented return control.
3. Silent global FX automation lane, independent of instrument hits. It alone owns shared-bus events, with bounded feedback, smoothing, Panic and whole-take Undo.

Reverb level/Decay/Damping/Width are already available. Pre-delay and a reverb
low-cut are candidates to evaluate after listening, not commitments for the
current release. Additional room-size controls should likewise earn their place
through playing. Avoid adding controls merely because the algorithm permits them.

### Per-track trig-lock scenes and morphing

Requested by Stephen, 2 November 2026. Each track has up to four trig-lock scenes
as the initial scope. Scenes hold alternative sets of that track's per-trig
settings. A track morph control moves between scenes by interpolating the
settings themselves: each value travels from its source-scene value to the
corresponding destination-scene value. This is parameter morphing, not an audio
crossfade or four simultaneously rendered instruments.

- Continuous settings interpolate smoothly, with exact scene values at the endpoints. A simple starting mapping is adjacent scene pairs along one control; the physical mapping and scene-selection workflow remain design work.
- The musician must be able to capture/edit scenes and understand the active source, destination and morph position. Audition should preserve authored scene endpoints; deliberate edits and recording need a clear Undo transaction.
- Scene scope is per-track trig settings. Shared delay/reverb character remains owned by global FX; scenes must not create competing global automation owners. The relationship to ROOT's independent ENV must be defined explicitly rather than implicitly snapshotting the drone again.
- Before implementation, settle missing-lock/base fallback, how pitch/velocity/gate and discrete settings such as slide or chord shape participate, and precedence with manual takeover, motion and Rejoin. Trig on/off pattern changes are not implied by trig-lock scenes.
- Decide whether/how the morph gesture is recordable and how scene capture uses the draft versus sounding values. These are open details, not implemented behaviour or permission to overwrite existing scenes during performance.

Acceptance should cover exact endpoints, useful intermediate timbres, rest
preservation, track isolation, predictable takeover/recording/Undo and bounded
shared-C CPU/memory use with the full palette. Scene storage needs an explicit
budget alongside the remaining device memory. Feature placement among the
post-palette milestones remains open; this request does not reprioritise chords
and FM.

### Complete playful, exploratory UI overhaul

Requested by Stephen, 2 November 2026. Redesign the instrument's layout,
navigation, typography, colour, visual feedback and interaction language around
a playful, exploratory experience. Teenage Engineering is the design reference:
bold readable typography, purposeful graphic shapes, strong control groupings,
expressive colour and inviting feedback. Develop an original UNDERGROWTH identity;
this is a complete UI/workflow milestone, not a colour-only reskin.

- Cover the browser's FM1 and Expanded views and the shared controller's physical-screen experience. Respect the real panel's fixed knobs, buttons, keys and non-touch screen; browser layouts can take advantage of their larger canvas.
- Make sound shaping and exploration feel immediate. Use clear visual responses for organism character, grains, scenes/morph position, transport and effects, while keeping the musician's authored events and parameter ownership legible.
- Keep draft versus trig versus live state, selected versus armed track, keyboard role, recording, scene capture and recovery unmistakable. Exploration must preserve deliberate note entry, predictable Undo/Panic and silent startup; it does not imply automatic composition.
- Explore the navigation and page hierarchy as part of the overhaul. Preserve required musical capabilities, but do not treat every existing screen arrangement as fixed. Prototype the core choose → shape → plant → vary → perform → recover loop on the actual FM1 control constraints.
- Retain keyboard, touch, pointer and assistive operation, readable contrast and comfortable targets. Colour and animation must not be the sole way to understand state. Expressive feedback must fit the physical display/render budget and avoid obscuring performance controls.

Start with a coherent visual/interaction prototype and task walkthroughs, then
carry the accepted design through shared C and the browser/device adapters.
Acceptance includes musical task clarity, real pointer/touch/keyboard paths,
small-screen layouts, hardware readability/ergonomics and the matching browser
and experimental firmware gate. Exact placement relative to scenes and expanded
effects remains open; the organism palette stays first unless Stephen changes
that priority.

### Controlled ROOT arp evolution

Inspired by Caravel 0.6.0's bounded WALK arp. Start with a small played/latched
note set; neighbour-note wandering, adjustable rests and a restricted register
keep the phrase recognisable. Hold freezes a pleasing phrase; Keep captures it
into the sequencer; Revert returns to the seed. ROOT comes first, with related
Chatter rhythm/articulation exploration later. This is planned original behaviour
in shared C, not an implemented generator or permission to add notes today.

### Punch-in effects and persistence

- Explore a dedicated sixteen-key punch-in effects page for throws, repeats, filter moves and reverb blooms. Black-key Hold/latch and modifiers remain proposals. Preserve SEQ's keyboard contract; settle mappings through playing.
- Add versioned session saving once parameter ownership settles. Stable IDs and migrations remain necessary. No persistence is implemented or authorised merely by listing it here.
- LFO and PRESETS remain reserved/unavailable. Their detailed workflows and priority have not been settled.
- Crush, sample selection/import and extending the granular atmosphere to other tracks remain deferred possibilities, subject to a separate scope and device budget. Note-following atmosphere, if pursued, must be an explicit optional mode.

## Awaiting acceptance

These checks distinguish implemented features from accepted musical/device
behaviour. They do not reopen accepted noncritical debt or automatically block
the next palette slice.

- **Listening:** Stephen's assessment of the 0.4.3 reverb wash at full send/return and useful lower Decay, on synth and empty-sequence ENV. Confirm the independent ROOT/ENV workflow and the organisms' useful ranges together.
- **Physical FM1:** authorised device testing of boot/silence, panel controls and screen readability, velocity/note-off, mix/recording/Panic, USB behaviour, dense-pattern CPU/P/X readings and recovery. Compare grains/reverb off and on. No physical flashing, CPU measurement or recovery validation has been performed.
- **Workflow:** physical reach and clarity of step versus keyboard roles, track/armed ownership, locks, audition and recovery. Automated mouse/touch tests are evidence of implementation, not a usability study.
- **Browser coverage:** automated Chromium evidence exists; Safari and wider physical mobile coverage remain open.

Published experimental is not the same status as listening-approved or physically
validated. See [PRODUCT.md](PRODUCT.md) for the current experience and
[HARDWARE.md](HARDWARE.md) for physical acceptance and recovery limits.

## Delivery contract and evidence

Every functional milestone requires both browser and a real validated `.fwsc`.
Run `tools/check_milestone.sh`; missing dependencies or a failed firmware package
gate leave the milestone incomplete. Preserve the pinned updater, identity/CRC/
loader checks and recovery path. No physical flash, device access or bootloader
change is authorised by software delivery.

0.4.3-hw1 passed the full gate: eleven native suites, native/WASM bit parity,
worklet, mouse/touch browser and simulated updater checks, platform-host traces,
firmware package/instruction/memory checks and exact package comparison. Reverb
sanitizer checks and publication byte verification are recorded in
[REVERB_EVALUATION.md](REVERB_EVALUATION.md). Historical releases and hardware
figures live in [HARDWARE.md](HARDWARE.md); older workflow proposals live in
[WORKFLOW_RESEARCH.md](WORKFLOW_RESEARCH.md).

Keep Bluetooth, Wi-Fi and CPU2 off, startup empty/silent, and sessions temporary
until explicitly changed. Preserve upstream notices; original-project licensing
remains undecided. Read [AGENTS.md](../AGENTS.md) and
[ENGINEERING.md](ENGINEERING.md) for the complete milestone and review rules.

CHORD implementation candidate: see [CHORD.md](CHORD.md) for current controls,
resource limits and verification. It is not yet published or listening-approved.
The broad FM organism follows; scenes and the UI overhaul remain later work.
