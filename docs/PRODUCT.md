# Product orientation

UNDERGROWTH is a playable rhythm and bass instrument for the FM1, developed alongside a browser prototype. The musician starts with an empty 16-step canvas, authors every hit and note, shapes individual sounds, and performs parameter changes. The browser is both an accessible way to try the instrument and a development surface for the shared engine.

Read this first for intent, then [engineering orientation](ENGINEERING.md) for implementation and validation. [ROADMAP.md](ROADMAP.md) records accepted design decisions and future work; [HARDWARE.md](HARDWARE.md) records the hardware build and physical acceptance limits. The repository's [milestone contract](../AGENTS.md) governs delivery.

## Musical identity

The five current organisms are Kick, Chatter, ROOT, Backbeat and CHORD. Kick is synthesized percussion; Chatter is synthetic, grain-like noise/resonant fragmentation; ROOT is monophonic harmonic bass. Backbeat continuously blends shaker collision grains, resonant skin/wire snares and clustered handclaps. Edge reaches crushed crackle and metallic fragments; Spread stretches the internal grains and flams. Intermediate settings mix these textures rather than selecting isolated presets. The percussion is synthesised, without samples. Each has a recognisable identity rather than a generic synthesizer editor.

The accepted direction is **strong intentional controls, subtle hard-wired DNA**. ENV means **Environment**, not a conventional ADSR editor. Its controls must produce useful, pronounced changes. Smaller, bounded and deterministic responses to velocity, tempo or musical context add character without rewriting the musician's score or generating notes. See the organism/control table in [ROADMAP.md](ROADMAP.md).

Startup and empty Play are silent. Enabling audio prepares the instrument without starting it. No automatic composition or preloaded pattern is part of the current experience. Planned arp evolution is a future, deliberately controlled workflow, not permission to make today's instrument generate events on its own.

## The player's workflow

1. Choose Kick, Chatter, ROOT, Backbeat or CHORD with ALGORITHM and shape the track's draft sound on EDIT/ENV.
2. Open SEQ. Its sixteen lower keys address sixteen steps for every organism. First activation of an untouched step captures draft sound and velocity (and ENV on percussion tracks); ROOT also captures pitch, gate and slide.
3. Tap to toggle a step. Turning it off preserves authored data for reactivation. Hold for 350 ms to open the persistent trig-lock menu without toggling. SELECT changes its Sound, ENV and Notes / velocity pages; SEQ closes it. Editing a rest keeps it silent.
4. Play and perform. Stored locks determine a step's sound; manual takeover temporarily overrides them. Rejoin or finishing the take releases takeover. Adding steps on another track preserves the current track’s manual takeover. The draft must never be contaminated by a passing playhead's lock.
5. REC arms and shows Hits, Motion or Both for a fixed track. Changing mode finishes the previous take and arms the next mode on that same track; finishing stays on the working page. Hits overdubs live events; Motion writes touched values on existing hits; Both supports touched values on new and existing hits. Rests remain rests in motion-only recording. One Undo restores the whole take. Viewing another track does not change the armed track.

ROOT has separate pitch/gate/slide/velocity articulation. Velocity is authored strength for all tracks; playback Human does not rewrite it. Swing is global; Human currently affects sequenced Chatter. Track Send feeds the existing shared, fixed filtered delay. Global transport must work across pages. Stop ends recording; Panic clears voices and effect tails without erasing the score.

HOME mixes four track levels and mutes independently of authored score Undo and motion. In HOME / step-entry SEQ, the upper keys provide track selection, Undo/Rejoin/audition and (in SEQ) same-organism full-trig copy/paste. Each track remembers its selected step and editing pages. SEL auditions instead of cycling tracks; in SEQ it hears the selected trig’s locks/articulation without creating a hit. EDIT and ENV both have an articulation page. See the [hardware-first workflow and research](UX_DESIGN.md).

The default browser view follows the FM1 panel; Expanded view shares the same running engine and score. Labels, keyboard roles, draft versus step ownership, armed track and unavailable features should remain visible. Touch, pointer, keyboard and assistive activation need coherent behavior.

## What exists and what remains open

| Area | Current state |
| --- | --- |
| Musical engine | Five 16-step lanes, sound/ENV locks, velocity, ROOT articulation, groove, recording, takeover/Rejoin and Undo. |
| Browser | Shared C/WASM engine and panel controller, JavaScript IO/drawing, FM1 and Expanded views. |
| Firmware | Shared C plus FM1 adapter; experimental `0.4.3-hw1` package and validation record are checked in. |
| Device evidence | Cross-build/package and host checks are recorded as passed. No physical flash, playing, CPU-headroom or recovery validation is established. |
| Persistence | Session-only. Reload/close loses the score. SAVE is unavailable. |
| Effects | Fixed shared delay with track sends. Shared reverb and delay return are implemented. Track filters, further delay controls and a silent FX lane are planned. |
| Other reserved controls | LFO, ARP and PRESETS do not establish implemented modulation, arp or preset workflows. |

For shared effects automation, the planned owner is a dedicated silent FX lane, independent of instrument hits. Do not create conflicting global locks on multiple instrument tracks. Future punch-in effects must preserve SEQ's keyboard contract. Keep roadmap proposals labelled as proposals until implemented and validated.

## Delivery and product claims

Every functional milestone needs both the browser and a real validated FM1 `.fwsc`. Bluetooth, Wi-Fi and CPU2 remain off unless Stephen requests them. Packages remain experimental until physical acceptance. Building firmware does not authorize device access, installation or bootloader modification.

The published browser and experimental installer are at https://stephencarr.github.io/UNDERGROWTH/ and its `flash.html` page. Publication does not prove listening approval or physical readiness. Hardware validation and recovery remain explicit open work in [HARDWARE.md](HARDWARE.md).

Original-project licensing remains undecided; retain upstream notices and consult [LICENSING.md](../LICENSING.md). Keep future agents informed by updating these guides when behavior changes. Older study sections in README and historical proposals in WORKFLOW_RESEARCH describe earlier stages; use current code/tests, the current FM1 control section and accepted decisions to resolve discrepancies.

### ROOT trial (draft, 0.4.0-hw1)

ROOT now trials separate Sound and FM Character pages under EDIT, direct selected
step notes and a deliberate Steps/Keyboard switch under SEQ, and an independently
enabled granular drone under ENV. FX exposes separate synth/atmosphere sends into
the existing shared delay. The sampler uses an original generated texture and
at most eight grains; its pitch is independent of ROOT's sequence. Default ENV
is off. Other organisms retain their earlier controls. See the ROOT trial in
[UX_DESIGN.md](UX_DESIGN.md) for exact controls and remaining limits. This does
not establish publication, listening approval or physical FM1 CPU validation.

### Shared reverb (0.4.1-hw1)

FX now offers track Sends, global Reverb and global Delay pages. Reverb has level,
Decay, Damping and Width; delay has a separate return. The existing track sends
and ROOT's separate synth/atmosphere sends feed both effects in parallel. Reverb
starts off. Raise a layer's send and the global Reverb return to hear it. Sends
support existing locks/motion; the shared effect settings have score Undo and
are not edited during recording. Stop/mute preserve tails, while Panic clears
them. A dedicated global FX automation lane, track filters, crush and sample
import remain future work. Physical room/grain CPU headroom still needs FM1 use.

At maximum Reverb and full send, that layer is essentially fully wet. Direct
attack and the distinct delay return recede, giving a diffuse wash rather than
only adding a quiet tail. Lower amounts retain definition.

### Independent ROOT atmosphere fix (0.4.2-hw1)

Ordinary ROOT steps capture only synth Sound/Character settings. They no longer
capture the atmosphere's pitch, grain controls, level or send. ENV needs Enabled
ON, a nonzero Level and PLAY; an empty sequence works. Changing ENV while stopped
then playing a bass sequence preserves the chosen drone settings and FX send.

### Clouds-derived wash and send ownership (0.4.3-hw1, published experimental)

The room now uses a Clouds-derived modulated diffuse tank. Decay has useful
ambience below maximum and a long wash at the top. Full send plus full Reverb
removes the direct layer; zero-send tracks retain their dry sound. Delay return
can be set to zero to audition the reverb alone. Listening acceptance remains
with Stephen; numeric tail/energy checks do not establish it.

Ordinary ROOT note entry also excludes Synth send. Raising it while stopped
then playing no longer restores an implicit zero. Deliberate send locks and
motion are retained; Rejoin restores those locks. FX shows the effective send
while playing and the draft while stopped. Sound/Character locks remain per trig.
ENV continues independently with Enabled ON, nonzero Level and PLAY, including
on an empty sequence. Stop fades ENV and lets the shared tank decay.

### CHORD — 0.5.0-hw1 candidate

CHORD adds an independent four-tone harmonic track, from short filtered dub
stabs through bright hollow chords to slow/sustained articulations. Eight editable
palette slots hold intervals, inversion and spread; each trig captures its actual
harmony. Root/gate/velocity stay separate from timbre. Two bounded tail banks
allow eight simultaneous tones. This is a draft implementation pending the full
milestone gate and listening acceptance. See [CHORD.md](CHORD.md).

## Same-track live takeover fix — 0.5.1-hw1 candidate

Adding a previously untouched step used to clear the sounding track's manual
takeover. Older trigs then resumed their captured values, making the sound revert
while the draft controls still showed the edits. Stamping now captures the draft
without releasing takeover. Existing trig locks remain intact. Rejoin, finishing
a take and Stop still deliberately release overrides. This applies to all five
tracks; the native regression checks 44.1/48 kHz and browser coverage repeats the
Chatter EDIT → SEQ → new-hit path using actual mouse and touch input.

The reproduction failed before the fix; five-track native regression and
native/WASM parity pass after it. The full 0.5.1-hw1 gate/review and preview update
are pending. Previously recorded 0.5.0 validation and accepted CHORD debt remain
separate; this fix does not claim the other review findings are resolved.
