# FM1 workflow research and proposed UNDERGROWTH interaction model

Research date: 2026-11-01. Status: **proposal**, except the independently verified touch-button repair. No new effects, FX lane, or redesigned sequencing/REC modes are claimed implemented by this document.

## Findings

The hardware-shaped shell is useful, but the current mapping was premature. In particular, LFO should not be the only route to motion recording; ARP should not unexpectedly mean recovery; ENV should not silently duplicate SEQ; and entering SEQ must have a clear, consistent keyboard contract for every instrument.

I compared manuals with the actual input handlers of four public FM1 projects. These are implementation examples, not evidence of which workflow users prefer or a substitute for playing the device. No external community user study or physical FM1 test was performed.

| Project | Verified pattern | What UNDERGROWTH should borrow |
|---|---|---|
| SLOOP | ALGORITHM chooses track; PRESETS chooses sound; SELECT walks pages. Page buttons enter their families. PLAY controls transport globally. REC arms while stopped and overdubs while playing. Held steps expose edits and locks. | Stable meanings, immediate transport, explicit recording state, contextual four-knob rows. |
| X0X | Lower keys write/toggle steps; upper keys choose drum tracks. Holding a step on EDIT locks sound; holding on SEQ edits pitch/gate/accent/slide. REC records notes and knob motion. FX has shared delay/reverb with per-part sends. | Always show what the keys do; distinguish step properties from sound locks; visibly animate recorded parameters. |
| JIANT | Buttons open remembered sections and cycle groups; SELECT walks pages; ALGORITHM always owns track selection. SEQ STEP supports holding a step and turning knobs to lock parameters. FX separates track sends, buses and master. | Stable track selection, separate track/bus scope, visible parameter ownership, a scalable page-family model. Avoid importing its entire large menu tree. |
| FM1-DUB | Buttons consistently open named layer/effect pages. FX cycles Dub/Echo. PLAY remains transport. REC deliberately means dub hold in this instrument. Input code distinguishes button-down, release and held gestures. | Clear visible page identity and performance-oriented effect pages. Do **not** copy REC-as-freeze: UNDERGROWTH already promises recording. |

X0X explicitly assigns FX/master motion to the 909 pattern. That is a workable ownership rule, but it would be an arbitrary dependency for UNDERGROWTH: changing or removing a percussion pattern should not unexpectedly replace global effects automation.

### The button failure

Reproduced with Chromium touch emulation: tapping PLAY left `playing=0`; tapping SEQ left the screen at SOUND. The inherited hit target suppresses `touchstart` defaults, so the browser need not emit a compatibility click. Our adapted button handler relied on `click`. Mouse `.click()` checks had passed, masking the failure.

Repair: act on pointer-down (as transport handlers in the reference firmware do), suppress the subsequent duplicate click, retain keyboard and assistive click activation, and show active page/transport/REC lights. Verify both input paths independently. Touch emulation is evidence of the browser event defect; it does not establish physical iOS/Android coverage.

## Core rule: two keyboard roles, visible on every screen

**SEQ + not recording: the 16 lower keys are the 16 steps.** This applies while stopped and while playing, for Kick, Chatter and ROOT alike.

1. Choose a track and shape its draft sound.
2. Press SEQ. The display shows the selected track, a readable 4×4 step grid, cursor, playhead and `KEYS: STEPS · TAP: STAMP`.
3. Tap an empty step: enable its trigger and capture the draft track sound as explicit per-step locks. ROOT also captures the selected input note, gate and slide.
4. Change the draft sound and tap another step: that hit keeps the new settings without changing the previous one.
5. Hold an existing step and turn a knob to edit that parameter on that step. Releasing returns to the draft sound.
6. A plain tap on an existing step selects/auditions it without erasing it. A dedicated visible ERASE action removes it. Explicit STAMP replaces its snapshot if desired. This avoids destroying a hit while trying to inspect it.

The treatment of occupied steps in item 6 is a **recommendation**, not a settled user requirement. An Elektron-style toggle is another valid option. The important distinction is that placing a hit, selecting it, and deleting it must not be ambiguous. The user's full-sound snapshot rule is authoritative here; do not describe it as an exact reproduction of every Elektron machine's default trig behavior. The Elektron comparison is a design reference, not a verified manual citation in this research.

### What “current knob positions” means

Introduce a **draft sound / note buffer per track**. Encoders update this buffer. A stamping command copies it atomically into the selected step in the shared C engine.

Do not sample the currently sounding lock or an animated browser slider: during playback that belongs to whichever step the playhead happens to be visiting. Otherwise writing step 9 could accidentally copy step 2's automation instead of the musician's edits. Display `DRAFT`, `STEP 09`, `LIVE` and `REC` ownership explicitly.

A snapshot covers track-owned sound and effect parameters, including values not on the currently visible knob page. Today that means Body, Edge, Length and Send. Future parameters use stable IDs and scoped ownership. Global tempo, Swing, Human and global effects are excluded from an instrument trig snapshot.

Rests never become hits merely because a control moves. Support parameter-only events later on the dedicated FX lane; do not quietly create audible instrument triggers.

## Recording and transport

PLAY/STOP must work on every page and during every non-destructive menu. Its light and the playhead must agree with audio state. Stop ends a take and releases live overrides; Panic immediately clears voices and effect tails.

REC owns recording configuration. Suggested behavior:

- Tap REC: arm/disarm the last chosen mode and show the REC screen. While stopped, show ARMED; while playing, show RECORDING. A later tap ends the take. Never silently change mode because a different page is open.
- REC mode is a screen option: **Hits**, **Motion**, eventually **Hits + Motion**. The current engine's mutually exclusive flags need redesign before claiming simultaneous mode works.
- With Hits recording, the keyboard becomes the live instrument. Events overdub onto the chosen quantization grid. Returning to non-recording SEQ restores step keys.
- With Motion-only recording, the keyboard may remain a step display/selector; it must not accidentally stamp a new hit. Moved track parameters write only onto existing triggers. Finish releases takeover and plays the result.
- With Hits + Motion, a played note/hit writes its pitch/trigger and the touched track parameter values in the same event transaction.
- Record mode and armed track are independent of page. The user must be able to navigate to FX and record effects while REC remains armed.
- One take is one Undo transaction. Do not replace this snapshot with each knob event or page change. An empty/no-op take preserves the previous Undo.

REC screen: track/scope, mode, ARMED/RECORDING, quantization, touched parameter names and `REC: FINISH`. Initially keep 16 steps and grid quantization; do not add count-in/free-time/replace modes just because references have them.

## Recommended panel map

| Control | Proposed purpose | Essential feedback |
|---|---|---|
| MASTER | Output level | Volume readout; no accidental pattern lock or motion capture |
| ALGORITHM | Track selection everywhere (Kick / Chatter / ROOT / later FX lane) | Track name and colour persist across sections |
| PRESETS | Track sound/preset selection | Do not repurpose it as navigation; explicitly mark unavailable until presets exist |
| SELECT | Page within current section; row navigation when a menu is open | Page breadcrumb / row highlight |
| KNOB1–4 | Four labelled values on the current page | Value, unit, base/lock/record ownership, changed/locked marks |
| HOME | Overview, track status, levels/mutes | Full transport and recording state; clear path back |
| SEL | Track/scope chooser and contextual confirm | Confirm target; not a hidden second track switch |
| EDIT | Sound pages for selected track | Draft macros; held-step lock target |
| ENV | Envelope/gate pages | Track-specific controls; unsupported fields visibly unavailable |
| SEQ | Step-entry page first, then note/step properties | 4×4 grid, active steps, selected step, moving playhead, keyboard role |
| REC | Recording menu and arm/disarm | Hits / Motion / Both, scope and state |
| PLAY/STOP | Global transport action | Persistent transport overlay/light; keep editing page rather than opening a pointless transport-only screen |
| FX | Track FX / Sends / Global bus pages | Explicit TRACK versus GLOBAL ownership |
| GLO | Tempo, swing, Human, system options | Pattern/global scope and units |
| LFO | Modulation section when implemented | Honest unavailable screen until there is modulation; not motion-recording navigation |
| ARP | Arpeggiator section when implemented | Honest unavailable screen; not a destructive recovery menu |
| SAVE | Project/save section when implemented | Honest session-only status until persistence exists |
| OCT−/+ | Live octave; contextual step-page banks only if patterns grow | Display octave or step bank explicitly |

Every section button should have a meaningful screen identity. Not every action requires stealing the screen: PLAY is better as an always-available action with persistent status. Reserve combinations for optional shortcuts after the primary workflow works with single presses. Publish a short on-screen help page instead of relying on README knowledge.

## Effects: track ownership first, a dedicated global FX lane second

Recommended signal path:

`voice → track tone/drive/filter → track level/pan → dry mix`

`                               ↘ per-track send → shared delay/reverb → wet mix`

`dry + wet → master processing → output limiter`

This is an architecture proposal, not a claim that those DSP blocks exist today. Currently we have one fixed filtered delay with a per-voice Send macro.

### Track-owned effects

Track processing and sends belong to the track and are lockable on its triggers and recordable with motion. A kick can throw just its last hit into delay; ROOT can have a different drive or filter setting on one note. Later locks on a track do not rewrite another track's values.

### Shared/global effects

Delay time, feedback, return tone, global freeze and master processing affect the mixed bus. Putting independent global locks on multiple instrument tracks creates conflicts when their triggers coincide. Do not solve this with an invisible “last track wins” rule.

**Recommendation: a dedicated FX automation track with 16 steps and the same transport.** Its steps are parameter events, not notes. It starts empty, makes no sound by itself, supports manual steps, motion overdub, Undo and mute/bypass. Initially share pattern length and timing; independent FX lengths/polyrhythms can wait.

Example: a kick's step 13 locks its send high; FX lane step 13 raises delay feedback; FX lane step 15 restores it. Their ownership is unambiguous.

Global FX motion needs a time-grid sampling rule because no instrument hit is required. Sample touched values at FX grid boundaries (later interpolate appropriate continuous parameters); never gate global automation on kick or bass notes. Freeze/bypass are discrete events; feedback/time/filters require bounded ranges and suitable smoothing to avoid clicks or unsafe runaway feedback. Delay-time transitions need their own DSP strategy, not just a slider write.

### Precedence and recovery

- Track parameters: manual takeover while performing/recording → active track lock → draft/base, with explicit Rejoin or take completion releasing takeover.
- Global parameters: manual takeover → FX-lane event → global base. Only the FX lane owns stored global automation.
- Modulation, when added, must respect locks; decide per-parameter whether a lock fixes the final value or the modulation's base. Do not implement this implicitly.
- Define lock lifetimes: current code applies a lock for a whole step, including an earlier hit's decay. For trig-based sound design, evaluate whether selected parameters should latch for the voice's full note instead. This needs audible tests before changing existing grooves.
- Scope Undo: one stamped step, one held-step edit, one motion take. A global FX take must not erase a prior track pattern accidentally.

## Prioritized implementation slices and acceptance criteria

### 0. Reliable input — implement now

Touch, mouse, keyboard and assistive activation must invoke each button exactly once. PLAY works on every page; page/REC lights reflect state. Hold/cancel/release cannot leave a key stuck. Verify the public URL, not only source/unit tests. This research includes the confirmed touch repair only.

### 1. Coherent SEQ + REC workflow

Move page/keyboard/record state into a tested shared controller suitable for later hardware integration. Add atomic C `stamp_step(track, step, draft)` semantics. Implement per-track draft buffers and the REC mode screen. Remap ALGORITHM/SELECT consistently. Add visible knob labels and keyboard-role footer.

Acceptance: shape Kick, SEQ, stamp 1/5/9/13; reshape and stamp 7; all earlier snapshots remain unchanged. Repeat for ROOT with pitch/gate. During playback stamping uses the draft, never the passing playhead's lock. PLAY works from any section. REC records hits, Motion records touched parameters on existing hits, both can be reached from REC. Touch and keyboard tests exercise complete paths. One Undo restores each edit/take.

### 2. Track FX and better dub delay

Expose controllable delay tone/feedback/time/return globally and track sends; add only a focused track processor after audition and CPU/RAM measurement. Register parameter ownership, units, range and smoothing once.

Acceptance: a per-trigger delay throw affects only the selected track's send; motion captures it and untouched locks survive. Global delay changes are bounded and audibly smooth. No sustained feedback after Panic. No claim of new effects without listening and actual engine tests.

### 3. Global FX lane

Add a silent 16-step automation lane, clear scope switch on FX, parameter-only events and motion sampling. No duplicate global automation owners.

Acceptance: a feedback swell and explicit return repeat independently of instrument hits; deleting the kick does not delete bus automation; manual takeover/Rejoin and whole-take Undo behave predictably; empty startup remains silent.

### 4. Hardware parity and saving

Port the chosen controller and screen rendering into the FM1 foundation. Browser currently shares audio/sequencing C, **not** the screen/controller implementation. Test the same input traces against host/firmware controller outputs before calling behavior hardware-faithful. Validate CPU/RAM, encoder detents, button scanning, screen readability and note-off on the device before any authorized flash. Stable parameter IDs and scopes prepare a versioned save format without committing to a final schema now.

## Pinned sources

- SLOOP: [controls and recording](https://github.com/isod89/sloop-fm1/blob/91f30182d9095e0ecdb43396554f2ce5a67df179/README.md#the-controls); [actual input handler](https://github.com/isod89/sloop-fm1/blob/91f30182d9095e0ecdb43396554f2ce5a67df179/firmware/src/ui_input.c) (`holds_input`, REC toggling/hold handling; transport dispatch); [layers](https://github.com/isod89/sloop-fm1/blob/91f30182d9095e0ecdb43396554f2ce5a67df179/firmware/src/ui_layers.c).
- X0X: [manual](https://github.com/charlesvestal/fm1-x0x/blob/ca2d377e3b9522579245152a81781a575f4d2fea/docs/MANUAL.md), especially controls, recording, motion/p-locks and effects; [implementation](https://github.com/charlesvestal/fm1-x0x/blob/ca2d377e3b9522579245152a81781a575f4d2fea/firmware/src/app/ui.c) (`motion_touch`, `motion_forget`, page references and button dispatch).
- JIANT: [manual](https://github.com/weavejam/JIANT/blob/af8cdb8ee2a45a6ea3d43f20f259a2a2578b7c14/docs/MANUAL.md), controls/navigation rules and FX/SEQ page groups; [input implementation](https://github.com/weavejam/JIANT/blob/af8cdb8ee2a45a6ea3d43f20f259a2a2578b7c14/firmware/src/ui_input.c), button tap/hold and transport handling.
- FM1-DUB: [manual](https://github.com/sbegueria/fm1-dub/blob/501c4cf22bb2c5ffdcf43467890127de623825dd/docs/MANUAL.md), panel, effects grid and FX pages; [input implementation](https://github.com/sbegueria/fm1-dub/blob/501c4cf22bb2c5ffdcf43467890127de623825dd/firmware/src/app/ui.c), PLAY on press, REC freeze on release, held FX grid.
- fm1-base: [browser panel template](https://github.com/charlesvestal/fm1-base/blob/02f422c7d9de121ca099bb2fdebe5c035f59e8ad/web/emu/index.html), accessible hit targets and touch/pointer input. Our extracted adapter replaced its firmware event path; the touch regression is in that adaptation.

No upstream DSP or workflow implementation was copied by this research. Existing panel code remains covered by the notices in LICENSING.md.

## Implemented follow-up: SEQ and REC

The browser now implements draft stamping, held-step macro locks, explicit Stamp/Erase/Undo actions, ROOT draft note/gate/slide, and a scoped Hits/Motion/Both recording take with whole-take Undo. Stamp, draft values and recording transactions live in shared C/WASM. Browser page selection and input routing remain JavaScript; the shared firmware controller portion of slice 1 is still pending. Track delay-send locks and motion use the existing effect. Global effect controls and the silent automation lane are not implemented by this slice. See README for the revised control map.
