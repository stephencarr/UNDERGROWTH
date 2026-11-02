# Hardware first: make a rhythm without losing your place

Design/research: 2026-11-02. This is an implemented design hypothesis for the browser and experimental FM1 controller, not a claim of expertise, a user study, or physical validation. Stephen requested a complete workflow pass after playing the prototype. The acceptance test is a musician making and changing a rhythm with the physical controls; browser screenshots alone cannot establish that it feels good.

## Start with hands and intent

The FM1 gives us four adjacent parameter knobs, three navigation encoders, fourteen buttons, sixteen lower keys, eleven upper keys and a small non-touch screen. Keep the printed controls meaningful. Put common operations on reachable single actions; make the current object, keyboard role and recording owner visible. A page change must not become a musical edit.

The core loop is **choose → shape → plant → vary → perform → recover**. A player should be able to follow that loop without learning every page first. The secondary loop is **arm → perform → finish → Undo**. Mixing is performance, distinct from rewriting the score.

## What the research contributes

These sources inform the design; the mappings below are our choices, not copied interfaces or experimentally proven improvements.

- [Nielsen’s usability heuristics](https://www.nngroup.com/articles/ten-usability-heuristics/) support clear feedback, familiar terminology, consistent actions, visible exits and recoverability. Applied here: persistent transport/armed-track status; explicit DRAFT versus STEP ownership; SEQ closes locks; Undo is directly reachable. Reading a heuristic does not establish that our implementation meets it.
- [Shneiderman’s direct-manipulation paper](https://www.cs.umd.edu/~ben/papers/Shneiderman1983Direct.pdf) motivates showing the edited object and making changes incremental and reversible. Applied here: one lower key per step; selected-trig labels and locks remain visible; same-organism copy/paste retains the whole trig, with Undo. No command syntax is required to plant a rhythm.
- [Fitts’s law](https://lawsofux.com/fittss-law/) is a reason to protect target size and spacing, not proof of a numeric speed gain. We retain the physical geometry, horizontal panel scrolling and a responsive Expanded view. Browser focus outlines and keyboard activation remain available. Finger comfort on actual hardware is still unmeasured.
- [Hick’s law](https://lawsofux.com/hicks-law/) suggests keeping immediate choices limited and grouping advanced actions. We expose the main task families first and put key-map reference/recovery on HOME pages. We do not hide basic step entry behind recording setup or a general-purpose action menu.
- [Roland TR-8S reference manual](https://static.roland.com/assets/media/pdf/TR-8S_Reference_eng02_W.pdf), pp. 13, 16 and 19–21: explicit mixing/mute, step entry, instrument performance and motion contexts provide useful precedents. We use distinct visible contexts; UNDERGROWTH’s particular modes, parameter ownership and grain synthesis remain its own. This older manual describes its specified firmware, not every current TR-8S feature.
- [Circuit Tracks hardware overview](https://userguides.novationmusic.com/hc/en-gb/articles/25494476280850-Circuit-Tracks-hardware-overview) distinguishes note entry, recording and copy/clear operations. Its [mixing guide](https://userguides.novationmusic.com/hc/en-gb/articles/26207162799506-Mixing-and-performing-on-your-Circuit-Tracks) uses visible track mute controls. We adopt immediate access to mix state and repeated editing operations, not its pad layout. Our mutes ramp dry/send gain; they do not stop score playback or rewrite notes.

The earlier [FM1 workflow research](WORKFLOW_RESEARCH.md) compares SLOOP, X0X, JIANT and FM1-DUB. It is historical and contains superseded proposals. The current mapping in this document and shared C controller takes precedence. Attempts to retrieve the indexed Elektron Digitakt II PDFs returned 404; their search excerpts are not used as verified manual evidence here. No third-party workflow implementation or DSP was copied.

## Physical control map

| Control | Stable role | Feedback / exit |
| --- | --- | --- |
| MASTER | Output level | Independent of score/locks. |
| ALGORITHM | Selected organism everywhere | Current track in header; actual encoder detent count is honoured. Changing the viewed track never transfers recording ownership. |
| SELECT | Page within this section | Page number and name. EDIT/ENV/FX repeated presses also advance pages. |
| PRESETS | Future preset selection | Explicitly unavailable; no unrelated hidden navigation. |
| KNOB1–4 | Four labelled values on the screen | HOME: four track levels. EDIT: sound or articulation. ENV: Environment or articulation. FX: current send. GLO: tempo/Swing/Human. REC: KNOB1 chooses take mode. |
| HOME | Mix and a reliable navigation anchor | Always returns to mix; SELECT then offers Recovery and Key guide. Hold HOME for Panic. |
| EDIT | Draft sound, then notes/velocity | Separate remembered page per track. Playback locks cannot contaminate the draft. |
| ENV | Environment, then notes/velocity | Separate remembered page per track. Same parameter IDs as locks/motion. |
| SEQ | Sixteen lower keys address sixteen steps | Tap toggles with authored data retained. Hold 350 ms opens persistent trig locks. Press SEQ to close. |
| SEL | Audition; confirm on Recovery/REC | No track cycling. In SEQ hears the selected trig, including a rest’s locks and articulation, without adding it to the score. |
| REC | Arm / finish the take | First arm shows setup. Choose Hits/Motion/Both, leave for a sound/effect page to perform. Finish stays on the working page; finishing from setup returns to the previous section. |
| PLAY / STOP | Transport on every page | Current running/stopped state stays visible. Stop finishes recording and releases ROOT. |
| OCT−/+ | Live pitched keyboard octave | ROOT note labels and out-of-range keys are explicit. No hidden step bank. |
| SAVE / LFO / ARP | Their named future functions | Unavailable screens. Session-only remains visible. |

### Three keyboard roles

1. **HOME = mix.** First four lower keys mute/unmute Kick, Chatter, ROOT, Backbeat. Four knobs set their levels. Lower keys 5–16 do nothing. Upper 1–4 select tracks; Undo, Rejoin and Hear remain accessible.
2. **SEQ step entry = steps and tools.** Sixteen lower keys keep the same time positions for every organism. Upper keys 1–4 select tracks, 5 Copy, 6 Paste, 7 Undo/redo, 8 Rejoin, 9 Hear, 10/11 previous/next lock page. Copy/paste requires SEQ and the same source organism. Page keys require an open lock menu. The HOME key guide explains these otherwise unprinted roles.
3. **Sound pages and Hits/Both recording = live instrument.** All 27 keys are musical; ROOT uses semitones. During Hits/Both recording in SEQ, upper keys are notes rather than shortcuts. Motion-only SEQ remains a step selector and cannot create hits. HOME remains the mixer even while a take is armed; use EDIT/ENV to play live.

The browser key captions, accessible descriptions and both adapters’ light masks come from the shared controller. They are feedback, not separate JavaScript navigation rules. ROOT keys outside the current pitch range show OUT and are silent.

### Retain context without hiding ownership

Each track remembers its selected step and each section’s page. Switching track closes a trig menu and shows the new track’s draft; it must never quietly redirect an existing menu transaction. HOME is a deliberate exception to page memory: its button always returns to mix. Copy/paste is a single session clipboard tagged with the source organism. A cross-organism paste is refused visibly before taking an Undo snapshot.

Short taps that overlap holds still follow the authored-data contract. Canceled presses do not become hits. Holding a rest does not enable it. Audition is an explicit temporary sounding context, released by a new hit, the next sequencer step, draft edits, Rejoin or Panic. A finite audition gate prevents ROOT from sounding indefinitely.

## Recording and performance state

- Hits, Motion and Both use the same armed-track transaction in Device and Expanded views. Expanded’s Motion button is only a shortcut into that shared mode, not another recorder.
- Changing mode closes the prior take and immediately arms the next mode on the same track. It does not silently abandon recording. The next actual edit/event owns the new take’s Undo boundary.
- Viewed and armed tracks are independent. Other-track synthesis edits are refused; finish the take before copying, pasting, editing velocity or using Undo. This keeps the take’s snapshot intact.
- One completed authored edit/take is one Undo/redo slot. Copying, auditioning, navigation and mixing do not replace that slot. Paste does, unless already grouped into the selected trig menu’s edit transaction.
- Mix levels and mutes are separate session performance state: not trig locked, motion recorded or restored by score Undo. A 5 ms gain ramp avoids an abrupt amplitude jump. Muting prevents new dry/send contribution; shared delay tails can continue. Panic clears all voices and effect tails, keeping the score and chosen mix.
- There is no disk or device storage write, project saving, new effect, arp or preset engine in this UX slice.

## Concrete walkthroughs

These are acceptance scenarios, not measured human completion times.

| Intent | Intended path | What must remain true |
| --- | --- | --- |
| Make a kick pattern | ALGORITHM Kick → EDIT → SEQ → lower 1/5/9/13 → PLAY | Empty startup; draft captured once per untouched step; no automatic notes. |
| Change one sound | Hold its lower key → turn knobs → SELECT for ENV → SEQ | Rest stays rest; other steps/draft unchanged; Undo restores the whole menu edit. |
| Repeat a designed hit | Select source → upper 5 → select destination → upper 6 | Locks, velocity and ROOT pitch/gate/slide preserved; rejected wrong-track paste preserves previous Undo. |
| Change track fast | Upper 1–4 in SEQ, or ALGORITHM | Same step positions; selected step/page restored for that track; no accidental audition. |
| Hear an edited rest | Hold empty step → edit → upper 9 / SEL | Actual selected locks/articulation heard; no new trig, transport or take. |
| Overdub texture | REC → choose Motion/Both → ENV/EDIT/FX → move controls → REC | Armed track remains fixed; untouched locks survive; finish stays in place; one Undo restores the take. |
| Drop the bass out | HOME → lower 3; turn third knob if needed | ROOT score intact; mute visible; no unexpected note from the mute key; unmute restores the mix. |
| Recover a bad edit | Upper 7 in SEQ/HOME; twice to redo | No trip through a recovery menu; empty Undo reports that fact. |
| Stop everything sounding | Hold HOME, or browser Escape/Silence | Voices and delay cleared; score retained; no flash or storage write. |

The old Undo path from SEQ required HOME → SELECT → SEL; the new upper-key path is one activation. Selecting a known track from SEQ can be one key rather than up to three detents. These are control-count comparisons only: they do not measure learning, finger travel, errors or musical flow. The extra keyboard role is a deliberate tradeoff requiring visible labels and a playing test.

## Evidence and remaining debt

Validate shared native tasks, clipboard/refusal/Undo boundaries, finite audition, mix ramps, ROOT releases, key roles/lights, mouse and genuine touch input, recording ownership and narrow layouts. Compare native/WASM output with the new performance interactions. Run the complete milestone gate, produce the matching experimental firmware, wait for review, then verify the actual public simulator before claiming publication.

Remaining questions for Stephen’s next play: do the upper-key roles stay memorable; can he reach a useful lock quickly; is switching from step entry to live recording obvious; are mute versus level and draft versus trig unmistakable; is HOME a reliable place to recover? Actual FM1 reach, encoder feel, screen readability, CPU headroom and recovery remain unvalidated. Noncritical discoveries become deferred debt under the project’s rapid-prototype policy rather than an excuse to hold the release indefinitely.

### Validation recorded for 0.3.0-hw1

The complete milestone gate passed locally: native invariants, native/WASM bit parity including paste and mix changes, mouse/touch workflows and layout, simulated updater safety, platform-host scripts including the new UX trace, and the pinned compiler’s package/instruction/memory checks. App image 109272 bytes; data+bss 31948/98304 bytes; pool 99724/344064 bytes. Review and live deployment verification remain the publication gates. These checks do not measure user task speed, listening approval or physical FM1 usability.

## ROOT trial — 0.4.0-hw1 (draft)

Only ROOT adopts this workflow; the other organisms retain their current controls.
EDIT has Sound (Tone, Colour, Decay, Attack), Notes/velocity, then Character
(Structure, Intensity, Feedback, Motion). The fixed carrier/modulator topology
belongs to ROOT's DNA; these macros shape ratios, index, bounded feedback and
modulation decay. Hold a step and SELECT through Sound, Character, Atmosphere,
Atmos mix, Sends and Notes to author locks. Sound/Character/atmosphere macros and
both sends support locks, motion and take Undo. The prior cross-track takeover
reset is fixed: stamping another lane cannot clear ROOT's live edits.

In ROOT SEQ, the knobs immediately address selected-step Note, Gate, Slide and
Velocity. Press SEQ again to switch between Steps and Keyboard. In Keyboard,
keys assign and audition the selected step's pitch without enabling a rest.
Hits/Both recording plays and records notes instead of assigning the selected
step. OCT-/+ changes the playable octave. EDIT's Notes page still sets the draft
pitch; the main EDIT page is a live keyboard without changing draft pitch.

ENV now controls an independent eight-grain maximum drone using an original
4096-sample woven texture generated at init, nominal sample rate 22050 Hz. The
first page has Position, Grain size, Density and Pitch (C1–C4); the second has
Level, Scan, Enabled and ENV send. Enable is a track switch, excluded from trig
locks and motion; Play is also required. Default is off. Stop fades it out;
Panic clears it immediately. ROOT mute/level affect both layers. The browser
plots the source, scan playhead and actual grain positions; the FM1 shows a
compact shared-controller grain/playhead strip. No file import, source selector,
note-following mode or additional effects are implied by this trial.

FX's ROOT track page has Synth send and ENV send, both feeding the existing
shared filtered delay. Synth send remains parameter 3; atmosphere send is 16.
The sequencer can lock these amounts; it does not own an additional gain stage.
Reverb, crush and additional effect routing remain future work.

For CPU comparison on a physical FM1, observe the existing CPU/P/X meter with
the same pattern and effects: first ENV Enabled OFF, then ON. Repeat with large
grains and maximum density. Record steady CPU, peak and any overrun-count change.
Peak and X are cumulative; restart for independent peak comparisons. Desktop
render timing and cross-build memory checks do not establish device headroom.
No physical device access or flashing is part of this trial's software validation.

Prior PR #7 review findings (audition ownership, Expanded groove during Hits,
and Finish motion during Both) are accepted/deferred noncritical debt for this
trial. They are not represented as fixed by the ROOT changes.

ROOT trial PR #8 review debt: when switching from Device Keyboard mode to Expanded,
the step grid inherits Keyboard mode. Return to Steps with SEQ before using the
Expanded step grid. This reversible note-editing issue is accepted/deferred
noncritical debt; it is not represented as fixed by the hosting setup.

## Shared reverb — 0.4.1-hw1

FX has three pages: track Sends, global Reverb, global Delay. All tracks feed the
same stereo room using their existing Send; ROOT's synth and atmosphere sends
remain independent and can be locked or motion-recorded per step. Reverb and
delay receive the send in parallel. Reverb level, Decay, Damping and Width are
shared settings. Delay has an independent return, so either effect can be heard
alone. The room starts at zero return; raise a send and the Reverb knob to hear
it. Repeated FX or SELECT advances pages. Expanded exposes the same five global
values (including Delay return).

Global FX edits participate in score Undo, are independent of selected track,
and are refused during a take so they cannot replace its Undo snapshot. Global
FX trig/motion automation remains deferred to the planned silent FX lane. Stop
and track mutes allow effect tails to decay; Panic clears the room immediately.
The original fixed-size room uses four damped combs and two diffusers per stereo
side, with bounded feedback and smoothed controls. It allocates no runtime memory.

Reverb's range was tuned after the owner found the original full return too
quiet. Full send plus full Reverb now washes the layer essentially fully wet;
the direct layer and distinct delay return recede smoothly at the top of the
range. Lower settings retain definition. The return has eight times the first
trial's gain and a longer bounded decay range; it does not depend on output
clipping to obscure the dry attack. Full-level native checks cover comparable
wet/dry energy on ROOT and ENV plus removal of the direct pre-reflection attack.

Accepted/deferred PR #8 review debt: new ROOT Expanded sliders show draft values
rather than effective values during motion/playback; toggling ENV Enabled as the
first Motion/Both edit is not captured in the take snapshot; and changing global
FX inside an open trig menu can combine later trig edits with its Undo boundary.
These are noncritical workflow/feedback issues, not fixed by this release.

ROOT atmosphere independence (0.4.2-hw1): ordinary ROOT stamping captures slots
0–10 only. Slots 11–18 are not implicitly locked by note entry. The continuous
drone runs with no steps and retains its level/send when a bass sequence plays.

## Clouds wash and explicit sends — 0.4.3-hw1 (published experimental)

ROOT ordinary notes capture synth Sound/Character and articulation, excluding
Synth send (3) and every atmosphere parameter. A deliberate Sends-page trig
edit or motion take still creates a send lock; reactivating a note retains it.
In FX playback, LIVE / FX SENDS shows what is heard and an encoder starts there.
When stopped it shows/edits draft sends. Explicit locks resume on Rejoin and
Stop/Play; ordinary snapshots no longer hide a zero send behind a 100% draft.
Expanded Synth/ENV sends follow effective playback values too.

The original Schroeder room above is superseded by a Clouds-derived modulated
tank, with four input diffusers and cross-coupled branches. Full send/Reverb
removes direct audio; Decay reaches ambience at mid travel and a longer wash at
maximum. Global ownership, Undo/REC guard, parallel delay, mute/Stop tails and
Panic remain. See REVERB_EVALUATION.md; human listening remains outstanding.

## CHORD candidate — fifth-track controls

ALGORITHM now selects five tracks. Existing upper-key track/copy/paste shortcuts
are retained; select CHORD with ALGORITHM. HOME's lower key 5 mutes CHORD; SELECT
page 4 exposes its level on KNOB1. The first four levels remain on HOME page 1.
CHORD EDIT cycles Sound, Notes/velocity, Harmony, Palette tones. ENV offers Attack,
Release, Sweep, Decay. SEQ knobs directly edit selected Root/Gate/Velocity/Slot;
repeated SEQ switches Steps/Keyboard. Palette edits preserve existing trigs.

Held CHORD trig menus cycle Sound, Envelope, Harmony, Trig tones, Notes/velocity.
They can edit a rest without enabling it; SEL auditions it. Explicit harmony
locks expand the actual palette definition rather than retaining a live reference.
Copy/paste is same-track, with Undo. The Expanded CHORD grid returns to Steps
before editing so it does not inherit the panel keyboard role. Expanded has a
fifth card/mixer and harmony sliders; Q–I plays chords while CHORD is selected.
This adds the organism to the current UI; the planned full UI overhaul remains
separate. See CHORD.md for controls and acceptance.

PR #14 accepts/defers noncritical CHORD review debt: held-key/Hear overlap can
latch at maximum Decay (Stop/Panic clears it); closed-menu step edits Undo per
detent; gates use straight duration under Swing; HOME's fifth level row crowds
the footer; F/Hear recording omits harmony snapshots. Use chord keys/MIDI or SEQ
stamping for captured harmony. See CHORD.md for details and workarounds.
