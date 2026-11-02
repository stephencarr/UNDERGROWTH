# UNDERGROWTH: first FM1 hardware milestone

**Status: the host, browser and pinned FM1 cross-build gate passed. `build/releases/0.3.0-hw1/` contains an experimental firmware package and its validation record. It has not been flashed or physically validated.**

The owner uses macOS and currently runs **Caravel 0.6.0**. Keep it installed while preparing this milestone; returning to stock is not a prerequisite established by the available documentation.

## What is shared

`core/undergrowth.c` owns sound and score; `core/controller.c` owns pages, track/step selection, encoders, buttons, key roles, recording mode, audition and screen text. The browser uses those same functions through WASM. `firmware/app.c` adapts fm1-base's physical panel, display and 24-bit stereo callback.

The application is single-core, without Bluetooth/Wi-Fi. It exposes USB MIDI and USB audio output from the instrument; USB playback into the instrument is disabled initially. MIDI channels 1/2 trigger Kick/Chatter; channel 3 plays ROOT C1–C4 with input velocity; channel 4 triggers Backbeat. Unsupported MIDI messages are ignored. Clock sync is not implemented. Physical capacitive-key velocity is supplied by the pinned platform's touch driver and requires playing tests on the real device.

Panel events enter a bounded queue and are consumed by the audio interrupt. A full queue requests Panic and clears queued events rather than risking a stuck note. Drawing is outside the audio interrupt, only when text changes. The core's 88,200-byte delay buffer uses `.pool` on the FM1. No heap allocations occur in the render path. Linker-map budgets and actual CPU headroom are still required: host timing is not a device measurement.

There is no project save/load. The platform has its panel-calibration object at `0xDC000`; do not assume another firmware's objects are compatible. The host smoke test requires zero storage writes in normal use. Back up Caravel before any future install.

## Reproducible build

Dependency revisions, SDK packaging hashes and the compiler archive checksum are in `firmware/dependencies.json`. They pin fm1-base and its felucca-bsp submodule. The SDK mirror supplies the exact upstream-tested revision, verified by file hashes; newer mismatched files are refused.

Linux x86-64 is the initial build host. macOS users will receive a prebuilt experimental package; no local compiler is needed to install it. Build setup, **not flashing**:

```
python3 tools/prepare_firmware.py
# Set FM1_BASE, AC79_SDK and JIELI_TOOLCHAIN to the paths printed by setup.
python3 tools/firmware_build.py --host
python3 tools/firmware_build.py --release 0.3.0-hw1
```

For the complete milestone gate, set ZIG to Zig 0.13, run a local web server for Chromium tests, and run `sh tools/check_milestone.sh`. The browser tests use `TEST_URL` (localhost by default). If any required dependency is unavailable, the gate fails; there is no placeholder firmware or waived hardware target. The successful Linux build used the SHA-256-verified compiler archive from `firmware/dependencies.json`; the resulting `.fwsc` and `SHA256SUMS` are in `build/releases/0.3.0-hw1/`.

A successful cross-build must include:

- An actual `undergrowth-VERSION.fwsc`, generated from the same controller/core sources as the browser.
- Platform linker-map, RAM/pool headroom, app-slot and instruction checks, with their build report.
- Independent package identity, loader marker and UFW/part CRC validation, including corrupted-package rejection.
- Source/dependency identity, SHA-256 hashes and licence notices. The initial output is explicitly experimental, with `hardware_validated: false`.

The `flash.html` page offers a Chrome/Edge desktop route to the pinned `firmware/releases/undergrowth-0.3.0-hw1.fwsc`. It checks SHA-256, FM1 identity and UFW CRCs before requesting USB MIDI SysEx access. Device identification and backup acknowledgement precede the install command. Its updater is the pinned fm1-base Web MIDI implementation; a port shown under another name can be selected explicitly. The browser gate tests verification and confirmation with simulated MIDI only. The page must be served over HTTPS or localhost; opening it as a local file is not supported. The hosted page is https://stephencarr.github.io/UNDERGROWTH/flash.html; no local server is needed.

## Caravel 0.6.0 to UNDERGROWTH: preparation, not an instruction to flash yet

The [tagged Caravel 0.6.0 installation guide](https://github.com/bonszaimusic/caravel/blob/b50bad57dcf32862f36661a5fc2996e13392b93e/docs/installing.md) documents the Felucca USB-MIDI update protocol, interrupted-install retry and identity checks. Its command-line installer does **not** make a backup. Its public installer offers its own published release, not an arbitrary UNDERGROWTH package. Do not assume pressing Install there installs UNDERGROWTH.

Before the first hardware trial:

1. Save Caravel projects/settings through its compatible editor. Retain the Caravel 0.6.0 release package and matching checksums.
2. Make and verify a full device flash backup with [M-Flasher](https://github.com/jvitkauskas/m-flasher), and keep a separate copy. A project export is not a full flash backup. This step is for the owner to authorize and perform later.
3. Record the actual USB identity and bootloader. Do not infer that m-bootloader is installed merely because Caravel is installed. Close DAWs and other MIDI clients before identification/update.
4. Verify the UNDERGROWTH package's published checksum and experimental designation. Inspect the allowed write range and reject model/identity mismatches; do not force past them.
5. The proposed direct route is the pinned fm1-base USB-MIDI updater in `flash.html`, or the command-line installer shipped with the package. Direct Caravel-to-UNDERGROWTH installation remains untested on physical hardware. No stock reinstall or bootloader replacement is part of this milestone.

## Recovery contract and limits

The pinned platform provides a silent safe mode after two failed application startups, with USB available to retry installation, and a route to chip download mode using REC. These are inherited source behaviours, not tests performed on Stephen's hardware. A severely interrupted image may not reach application-level recovery.

[M-Flasher's safety rules](https://github.com/jvitkauskas/m-flasher/blob/main/docs/safety.md) include a pre-write backup, model checks and verified writes. Its [optional m-bootloader](https://github.com/jvitkauskas/m-bootloader) adds recovery independent of the instrument application. Installing it is a distinct bootloader modification and is not authorized or performed here. No automatic previous-image rollback is promised. Lower-level [FM-1 Transporter](https://github.com/kurogedelic/FM-1-transporter) recovery may require additional hardware.

## Physical acceptance before removing the experimental label

Check boot/reboot and silence; all panel controls/lights; knob direction and held-step release; each voice and velocity; ENV locks/motion/Undo; sequencer/groove timing; ROOT note-off; Master/Panic; USB enumeration on the owner's Mac; long dense-pattern runs with CPU load/xrun readings; update identity verification; and a documented, successfully exercised recovery/reinstall route. Do not deliberately interrupt a flash on the owner's only instrument to manufacture a recovery claim.

## On-screen performance meter

Every hardware page has a bottom strip: `CPU 42% P68% X0 EXP`, with a load bar. CPU is the platform's audio-render time as a percentage of its audio deadline, smoothed over approximately 16 blocks. It is not total processor utilisation. P is the highest **observed smoothed** reading since boot, sampled each UI frame; it is not a raw single-block maximum. X is the platform's cumulative audio-overrun counter since boot. Stop and Panic do not erase these diagnostics.

The strip updates at 4 Hz independently of page changes, outside the audio interrupt. Load at 80% turns amber; 95% or any recorded overrun turns red. Values above 100% remain visible numerically; only the bar saturates. These thresholds are visual indicators, not automatic voice shedding. Host simulation is labelled HOST. The browser displays `CPU -- / FM1 only`, because desktop/browser timing would not establish FM1 headroom.

Meter formatting and redraw cadence are covered by controller and platform-host tests. The cross-build passed; actual FM1 readings still require the hardware acceptance work above.

## Review fixes in 0.1.2-hw1

Step on/off preserves authored sound locks, velocity and ROOT note settings. Only an untouched step captures the draft on its first activation. Overlapping short presses are tracked independently. Reselecting the same trig retains the menu’s Undo snapshot, and opening its menu releases live takeover so edits can be heard. Expanded sliders share the same menu transaction.

The flasher binds the initial update to the explicitly identified input/output pair and identity. When USB re-enumeration changes ports, the owner must identify and confirm the same physical FM1 again; Web MIDI cannot establish physical identity automatically. Escape and Cancel never authorize a write, including retries.

USB connection loss or host sleep invokes Panic, silencing notes and stopping playback/recording. This uses the pinned platform’s USB-host loss notification and its USB power badge transition for device mode. The host tests simulate the latter transition; physical unplug/recovery tests remain outstanding. Screen-reader vocabulary is published at the firmware’s advertised `words/010e19d6e9.json` path and retained for installed releases.

Pending short taps resolve before the lock-menu transaction starts. On USB loss, the adapter quarantines USB packets across render blocks until the backlog is drained and the connection is ready again; non-USB input remains usable and each render still handles at most 32 MIDI packets.

## Backbeat milestone — 0.2.0-hw1

Adds a fourth shared-C organism and independent score lane. Body/Edge blend skin resonance with wire noise; ENV Damping/Spread/Response shape decay, clap clusters and attack. The browser and physical panel select it with ALGORITHM; USB MIDI channel 4 triggers it. The new screen-reader vocabulary is `words/1a16a581bf.json`; the previous release vocabulary is retained. Chord stabs and the flexible FM organism are roadmap items. Physical sound/CPU validation remains outstanding.

## Backbeat texture revision — 0.2.1-hw1

Replaces the narrow skin/noise blend after the owner’s first audition: Body now blends shaker grains, resonant snares and clustered claps. Edge reaches crushed crackle and metallic fragments; Spread controls internal collision density/spacing and clap flams. Short timbre smoothing applies in shared C without changing authored locks. Static collision/filter state is reset by init and Panic; clouds and tails are bounded. Existing MIDI and recording ownership are unchanged. Native checks cover timbral distinction, extreme corners at 8/96 kHz, finite tails, lock/Undo preservation and dense four-voice output. The full milestone gate passed on this revision: native/WASM parity, mouse/touch browser and simulated-updater checks, platform-host traces, cross-build, package CRC/identity and instruction/memory checks. App image: 103420 bytes; data+bss: 31424/98304 bytes; pool: 99724/344064 bytes. Review remains a publication gate. No physical sound or CPU claim follows from the cross-build.

## Hardware-first workflow — 0.3.0-hw1

The shared controller now owns HOME mixing, upper-key step tools, per-track navigation memory, key-role captions/light masks, and a recorder whose mode changes preserve the armed track. A typed clipboard preserves complete trigs; wrong-organism paste is refused. Explicit step audition hears stored locks/articulation on a rest without writing a trig or moving transport. Session mix levels/mutes use a 5 ms gain ramp, sit outside score Undo and motion, and leave shared delay tails to decay. Browser and hardware wrappers consume the same role/light state. The platform-host UX trace checks mixer state, clipboard/Undo, selected-step memory, REC ownership, help-page inertness, Panic and zero storage writes. See [UX design and research](UX_DESIGN.md). Physical ergonomics, screen reading, listening, CPU and recovery remain unvalidated.

For `0.3.0-hw1`, the full milestone gate passed. App image: 109272 bytes; data+bss: 31948/98304 bytes; pool: 99724/344064 bytes. Package identity `FM-1_731000300`; CRC/loader and instruction checks passed. No physical FM1 access or flashing was performed.

## ROOT Character and atmosphere trial — 0.4.0-hw1 (draft)

Adds fixed-topology FM personality macros and a bounded eight-grain sampler in
shared C. The texture is original and generated once at init; the sampler does
not allocate or perform IO in render. Its pool holds 4096 float samples. Observe
the existing physical CPU/P/X strip with ENV off/on and maximum density before
accepting device headroom; no host timing substitutes for this comparison.
Build/package and regression evidence is recorded separately from physical tests.
No physical FM1 access, install, CPU measurement or listening approval is claimed.

Validation for this draft passed the full milestone gate: native suites (including
ROOT grains/high-bit locks), native/WASM bit parity, worklet, mouse/touch browser
and updater simulations, platform-host traces, and FM1 package/CRC/identity,
instruction and memory checks. Image: 116236 bytes; data+bss: 40332/98304 bytes;
pool: 116108/344064 bytes. Identity: FM-1_731000400. In a final dense four-track
host comparison, 10 seconds of audio took 0.1111 CPU seconds with grains off and
0.1176 with grains on (about 5.9% relative added host cost). This is not an FM1
CPU percentage. Physical grain-on/off CPU/P/X readings remain outstanding.

## ROOT with shared reverb — 0.4.1-hw1

Adds an original stereo room to the shared sends, controlled on FX's Reverb page
with level, Decay, Damping and Width. The Delay page adds an independent return.
The room owns a fixed 30,000-float pool buffer covering 8–96 kHz; no IO/allocation
occurs in render. Default reverb return is zero. Panic clears the buffer. Existing
CPU/P/X diagnostics remain the physical acceptance path: compare the same dense
pattern with grains on and reverb off/on. Host measurements are comparative
render cost only. No FM1 access, flashing, listening or device CPU claim is made.

The full milestone gate passed for 0.4.1-hw1: ten native suites, active
grains/reverb native-WASM bit parity, worklet, mouse/touch browser and simulated
updater checks, platform-host traces and real package/CRC/identity, instruction
and memory checks. Image: 118668 bytes; data+bss: 40604/98304 bytes; pool:
236108/344064 bytes. Identity: FM-1_731000401. In the gate's dense four-track
host comparison with grains engaged, 10 seconds rendered in 0.1153 CPU seconds
with reverb off and 0.1436 with reverb on. These are host costs, not FM1 CPU.

The final reverb range also checks full-level wet/dry balance on ROOT and ENV,
with smoothed dry attenuation at maximum Reverb/full send. Feedback stays below
one and extreme values remain finite. These changes require the final refreshed
package/checksums, not the quieter early preview's binary.

Final full-send/full-Reverb balance at 44.1 kHz measured +1.69 dB on ROOT and
-0.60 dB on ENV relative to their dry reference, with the direct attack removed
at maximum. The new regression also verifies a zero-send track retains its
direct attack. This is an objective balance check, not listening approval.

## Independent ROOT atmosphere — 0.4.2-hw1

Ordinary ROOT note snapshots now exclude atmosphere parameters, so note entry
does not restore stale ENV level/send values during playback. The full milestone
gate passed: native regressions, native/WASM bit parity, mouse/touch browser and
simulated updater checks, platform-host traces, package CRC/identity, instruction
and memory checks. Image: 118668 bytes; data+bss: 40604/98304; pool: 236108/344064.
Identity: FM-1_731000402. PR #10 review completed with no findings; its subsequent
test-only change tolerates encoder floating-point rounding. No physical device
access, flashing or FM1 CPU validation was performed.

## Clouds-derived reverb — 0.4.3-hw1 (published experimental)

The shared C Clouds tank uses 49,200 float pool samples, with no render-time
allocation. Cross-build: image 119884 bytes; data+bss 40572/98304 bytes; pool
312908/344064 bytes (31,156 bytes spare). Identity FM-1_731000403. Package CRC,
loader and instruction checks passed. Final full-gate evidence follows below.
The larger float arena supports 8–96 kHz with no resampler. Device CPU/P/X
comparison with grains enabled and reverb off/on remains required; host cost
is not FM1 headroom. No device access or physical flashing was performed.

Validation on 2 November 2026: the complete `tools/check_milestone.sh` passed:
eleven native suites (including impulse response), native/WASM bit parity,
worklet, mouse/touch panel/workflow/ENV/ROOT/reverb/mixer tests, simulated updater
safety, platform-host traces, firmware CRC/identity/loader/instruction/memory
checks and rebuilt-vs-checked-in package/vocabulary comparisons. Reverb tests
also passed address/undefined-behaviour sanitizers (leak detection off).
No physical device was accessed. Listening and FM1 CPU acceptance remain open.

## Grain refinement — 0.5.0-hw1 (unpublished)

Hardware reserves a 22,650-sample float tank for its fixed 44.1 kHz codec rather
than the native/browser 49,200-sample arena. Delay lengths/topology are unchanged;
22,587 slots are used, and profile impulse output matches bit-for-bit. Saves
106,200 bytes. Cross-build image: 120268 bytes; data+bss 40604/98304; pool
206708/344064 (137356 bytes spare). Identity FM-1_731000500. Package CRC/loader,
instruction and memory checks passed. Surprise uses bounded seeded scatter and
occasional octave/reverse grains, with an eight-grain limit and no runtime
allocation. Physical CPU and listening remain unvalidated. No flashing performed.

Validation recorded for this draft on 2026-11-02: the complete milestone gate
passed, including twelve native suites, compact/native reverb impulse identity,
native/WASM bit parity with Surprise, worklet, mouse/touch workflows, simulated
installer cancellation/identity/corruption/re-enumeration checks, platform-host
traces, real .fwsc CRC/identity/loader/instruction/memory checks, and package and
vocabulary comparisons. Focused grain ASan/UBSan checks also passed (leak checks
off). No subjective listening approval, physical FM1 CPU measurement, publication
or flashing is claimed. USB sample capture remains future work.

## Independent atmosphere palette — 0.6.0-hw1 (unpublished)

Original harmonic/crunch, modulated-noise and morphing-formant atmospheres add
fixed scalar state, with no sample buffers or runtime allocation. Cross-build:
image 125868 bytes; data+bss 40940/98304; pool unchanged at 206708/344064.
Identity FM-1_731000600. Package CRC/loader/instruction/memory checks passed.
The smaller 44.1 kHz reverb allocation is retained. Four concurrent atmospheres
plus dense percussion and reverb still require physical CPU/P/X and listening
acceptance. Host timings and cross-build fit do not establish device headroom.
No device access, flashing, CPU2 or radio activation is performed.

Validation for unpublished 0.6.0-hw1: the complete milestone gate passed on
2026-11-02, including thirteen native suites, all-atmosphere native/WASM bit
parity, worklet, mouse/touch browser workflows, new atmosphere platform-host
traces, simulated updater safety, package identity/CRC/loader/instruction/memory
checks and exact rebuilt package/vocabulary comparisons. Focused atmosphere and
reverse-grain reproducer ASan/UBSan checks passed (leak detection off). Listening
and physical FM1 CPU acceptance remain open; no flashing was performed.
