# UNDERGROWTH: first FM1 hardware milestone

**Status: the host, browser and pinned FM1 cross-build gate passed. `build/releases/0.2.1-hw1/` contains an experimental firmware package and its validation record. It has not been flashed or physically validated.**

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
python3 tools/firmware_build.py --release 0.2.1-hw1
```

For the complete milestone gate, set ZIG to Zig 0.13, run a local web server for Chromium tests, and run `sh tools/check_milestone.sh`. The browser tests use `TEST_URL` (localhost by default). If any required dependency is unavailable, the gate fails; there is no placeholder firmware or waived hardware target. The successful Linux build used the SHA-256-verified compiler archive from `firmware/dependencies.json`; the resulting `.fwsc` and `SHA256SUMS` are in `build/releases/0.2.1-hw1/`.

A successful cross-build must include:

- An actual `undergrowth-VERSION.fwsc`, generated from the same controller/core sources as the browser.
- Platform linker-map, RAM/pool headroom, app-slot and instruction checks, with their build report.
- Independent package identity, loader marker and UFW/part CRC validation, including corrupted-package rejection.
- Source/dependency identity, SHA-256 hashes and licence notices. The initial output is explicitly experimental, with `hardware_validated: false`.

The `flash.html` page offers a Chrome/Edge desktop route to the pinned `firmware/releases/undergrowth-0.2.1-hw1.fwsc`. It checks SHA-256, FM1 identity and UFW CRCs before requesting USB MIDI SysEx access. Device identification and backup acknowledgement precede the install command. Its updater is the pinned fm1-base Web MIDI implementation; a port shown under another name can be selected explicitly. The browser gate tests verification and confirmation with simulated MIDI only. The page must be served over HTTPS or localhost; opening it as a local file is not supported. The hosted page is https://stephencarr.github.io/UNDERGROWTH/flash.html; no local server is needed.

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

Replaces the narrow skin/noise blend after the owner’s first audition: Body now blends shaker grains, resonant snares and clustered claps. Edge reaches crushed crackle and metallic fragments; Spread controls internal collision density/spacing and clap flams. Short timbre smoothing applies in shared C without changing authored locks. Static collision/filter state is reset by init and Panic; clouds and tails are bounded. Existing MIDI and recording ownership are unchanged. Native checks cover timbral distinction, extreme corners at 8/96 kHz, finite tails, lock/Undo preservation and dense four-voice output. The required milestone gate and review must pass before publication; no physical sound or CPU claim follows from the cross-build.
