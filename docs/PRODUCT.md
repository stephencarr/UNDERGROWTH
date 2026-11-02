# Product orientation

UNDERGROWTH is a playable rhythm and bass instrument for the FM1, developed alongside a browser prototype. The musician starts with an empty 16-step canvas, authors every hit and note, shapes individual sounds, and performs parameter changes. The browser is both an accessible way to try the instrument and a development surface for the shared engine.

Read this first for intent, then [engineering orientation](ENGINEERING.md) for implementation and validation. [ROADMAP.md](ROADMAP.md) records accepted design decisions and future work; [HARDWARE.md](HARDWARE.md) records the hardware build and physical acceptance limits. The repository's [milestone contract](../AGENTS.md) governs delivery.

## Musical identity

The four current organisms are Kick, Chatter, ROOT and Backbeat. Kick is synthesized percussion; Chatter is synthetic, grain-like noise/resonant fragmentation; ROOT is monophonic harmonic bass. Backbeat combines two skin resonances and filtered wire noise, with clap-like clusters at wider Spread settings. The percussion is synthesised, without samples. Each has a recognisable identity rather than a generic synthesizer editor.

The accepted direction is **strong intentional controls, subtle hard-wired DNA**. ENV means **Environment**, not a conventional ADSR editor. Its controls must produce useful, pronounced changes. Smaller, bounded and deterministic responses to velocity, tempo or musical context add character without rewriting the musician's score or generating notes. See the organism/control table in [ROADMAP.md](ROADMAP.md).

Startup and empty Play are silent. Enabling audio prepares the instrument without starting it. No automatic composition or preloaded pattern is part of the current experience. Planned arp evolution is a future, deliberately controlled workflow, not permission to make today's instrument generate events on its own.

## The player's workflow

1. Choose Kick, Chatter, ROOT or Backbeat with ALGORITHM and shape the track's draft sound on EDIT/ENV.
2. Open SEQ. Its sixteen lower keys address sixteen steps for every organism. First activation of an untouched step captures draft sound, ENV and velocity; ROOT also captures pitch, gate and slide.
3. Tap to toggle a step. Turning it off preserves authored data for reactivation. Hold for 350 ms to open the persistent trig-lock menu without toggling. SELECT changes its Sound, ENV and Notes / velocity pages; SEQ closes it. Editing a rest keeps it silent.
4. Play and perform. Stored locks determine a step's sound; manual takeover temporarily overrides them. Rejoin or finishing the take releases takeover. The draft must never be contaminated by a passing playhead's lock.
5. REC selects Hits, Motion or Both for an armed track. Hits overdubs live events; Motion writes touched values on existing hits; Both supports touched values on new and existing hits. Rests remain rests in motion-only recording. One Undo restores the whole take. Viewing another track does not change the armed track.

ROOT has separate pitch/gate/slide/velocity articulation. Velocity is authored strength for all tracks; playback Human does not rewrite it. Swing is global; Human currently affects sequenced Chatter. Track Send feeds the existing shared, fixed filtered delay. Global transport must work across pages. Stop ends recording; Panic clears voices and effect tails without erasing the score.

The default browser view follows the FM1 panel; Expanded view shares the same running engine and score. Labels, keyboard roles, draft versus step ownership, armed track and unavailable features should remain visible. Touch, pointer, keyboard and assistive activation need coherent behavior.

## What exists and what remains open

| Area | Current state |
| --- | --- |
| Musical engine | Four 16-step lanes, sound/ENV locks, velocity, ROOT articulation, groove, recording, takeover/Rejoin and Undo. |
| Browser | Shared C/WASM engine and panel controller, JavaScript IO/drawing, FM1 and Expanded views. |
| Firmware | Shared C plus FM1 adapter; experimental `0.2.0-hw1` package and validation record are checked in. |
| Device evidence | Cross-build/package and host checks are recorded as passed. No physical flash, playing, CPU-headroom or recovery validation is established. |
| Persistence | Session-only. Reload/close loses the score. SAVE is unavailable. |
| Effects | Fixed shared delay with track sends. Track filters, reverb, adjustable global effects and a silent FX automation lane are planned. |
| Other reserved controls | LFO, ARP and PRESETS do not establish implemented modulation, arp or preset workflows. |

For shared effects automation, the planned owner is a dedicated silent FX lane, independent of instrument hits. Do not create conflicting global locks on multiple instrument tracks. Future punch-in effects must preserve SEQ's keyboard contract. Keep roadmap proposals labelled as proposals until implemented and validated.

## Delivery and product claims

Every functional milestone needs both the browser and a real validated FM1 `.fwsc`. Bluetooth, Wi-Fi and CPU2 remain off unless Stephen requests them. Packages remain experimental until physical acceptance. Building firmware does not authorize device access, installation or bootloader modification.

The published browser and experimental installer are at https://stephencarr.github.io/UNDERGROWTH/ and its `flash.html` page. Publication does not prove listening approval or physical readiness. Hardware validation and recovery remain explicit open work in [HARDWARE.md](HARDWARE.md).

Original-project licensing remains undecided; retain upstream notices and consult [LICENSING.md](../LICENSING.md). Keep future agents informed by updating these guides when behavior changes. Older study sections in README and historical proposals in WORKFLOW_RESEARCH describe earlier stages; use current code/tests, the current FM1 control section and accepted decisions to resolve discrepancies.
