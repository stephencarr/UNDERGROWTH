# Engineering orientation

Start with [PRODUCT.md](PRODUCT.md), [../AGENTS.md](../AGENTS.md), then this guide. Read [HARDWARE.md](HARDWARE.md) before firmware work and [LICENSING.md](../LICENSING.md) before redistributing artifacts. [ROADMAP.md](ROADMAP.md) separates accepted decisions from planned features; [WORKFLOW_RESEARCH.md](WORKFLOW_RESEARCH.md) contains historical proposals, not a complete description of current implementation.

## Architecture and ownership

| Path | Responsibility |
| --- | --- |
| `core/undergrowth.c`, `.h` | Authoritative DSP, sample-clock sequencing, score, draft values, locks, articulation, recording and Undo. |
| `core/controller.c`, `.h` | Shared panel navigation/memory, key labels/light masks, task actions, input/edit semantics, recording scope and screen text. |
| `web/app.js`, `web/worklet.js` | Browser controls and AudioWorklet bridge into C/WASM. |
| `web/device.js`, `web/fm1-panel.*` | Browser panel integration, drawing and input targets. Preserve upstream panel notices. |
| `firmware/app.c`, `.h`, `fm1.toml` | FM1 physical IO, bounded event queue, audio callback, screen/lights and diagnostics. |
| `web/core.wasm`, `index.html` | Generated WASM and standalone browser bundle; rebuild from source. `web/index.html` is the modular entry. |
| `flash.html`, `web/flash.js`, `web/confirmed-updater.js` | Experimental firmware installer, verification and explicit MIDI-pair confirmation. |
| `web/vendor/fm1-base/` | Pinned upstream Web MIDI updater/package/port helpers. |
| `firmware/dependencies.json` | Exact platform/BSP/SDK revisions, SDK file hashes and compiler archive checksum. |
| `firmware/releases/`, `words/` | Published experimental packages, provenance/checksums/notices and installed-release screen-reader vocabulary. |
| `tests/`, `tools/` | Invariant, parity, browser, updater and platform-host checks; build/package tooling. |

Musical state and interaction semantics belong in shared C. Browser/device wrappers adapt IO; they must not acquire an independent sequencer, DSP or menu state machine. The browser's effective playback values are not the player's draft. Track locks, draft values, event articulation and global parameters have different owners.

The render path uses fixed-size state without runtime allocation or IO. Hardware input is queued and consumed in the audio interrupt; drawing stays outside it. Queue overflow requests Panic. USB-loss handling must silence playback and quarantine stale USB input across render blocks; preserving bounded work and note-off behavior matters more than keeping a performance running after a disconnect. The fixed delay buffer uses the platform pool. Desktop timings and WASM reserved memory do not measure FM1 CPU/RAM headroom.

## Local browser and core development

Run commands from the repository root. Requirements are Python 3, a C99 compiler, Node and Zig **0.13.0**. Browser tests additionally require Playwright and Chromium; several scripts explicitly use `/usr/bin/chromium`, so inspect their launch configuration before choosing another installation path.

```sh
# Optional project-local Zig installation:
python3 -m pip install --target .tools ziglang==0.13.0
export ZIG="$PWD/.tools/ziglang/zig"
set -e
sh tools/build.sh
for test in core groove motion root workflow environment controller backbeat; do
  "./build/${test}_test"
done
node tests/parity.mjs
node tests/worklet.mjs
python3 tools/bundle.py
```

These are development checks, not a substitute for the milestone gate. Stop and investigate any failing command. `CC` selects the native compiler; deterministic native/WASM comparisons rely on floating-point contraction being disabled on both builds.

Serve the repository in a separate terminal:

```sh
python3 -m http.server 8000 --bind 127.0.0.1
```

Open `http://localhost:8000/` for the bundle or `/web/` for modular development. Browser scripts use `TEST_URL`, defaulting to `http://127.0.0.1:8000`. Representative interaction checks are `node tests/panel_input.cjs`, `node tests/workflow_browser.cjs` and `node tests/environment_browser.cjs`. They exercise real browser behavior; native tests alone cannot catch touch-event or AudioWorklet integration regressions. Standalone-file loading has separate browser restrictions and is not established by HTTP-server checks.

## Firmware and required milestone gate

Linux x86-64 is the initial cross-build host. Follow [HARDWARE.md](HARDWARE.md) for the complete prerequisites and recovery context:

```sh
python3 tools/prepare_firmware.py
# Export FM1_BASE, AC79_SDK and JIELI_TOOLCHAIN using setup's printed paths.
python3 tools/firmware_build.py --host
python3 tools/firmware_build.py --release 0.4.3-hw1
```

Do not update pinned revisions or bypass hash, TLS, package identity, CRC, loader, instruction or memory checks merely to make a build succeed. The compiler download hosts are `pkgman.jieliapp.com` and `jl-update.oss-cn-shenzhen.aliyuncs.com`; environment network policy may need to allow required destinations.

With toolchains, browser dependencies and the local server ready, run:

```sh
sh tools/check_milestone.sh
```

This builds native/WASM, runs core/controller tests, parity/worklet and browser/updater checks, builds the platform host and real FM1 package, and compares firmware/vocabulary with published artifacts. `UG_RELEASE` selects the package version for the gate; the default is currently `0.4.3-hw1`. A new release needs matching checked-in artifacts, manifest, notices and installer references. Inspect the scripts before changing release versions rather than treating the environment variable as a complete release procedure.

Build outputs include `firmware/build/` and `build/releases/<version>/`. The package manifest records source/dependency identities and experimental/hardware-validation status. Missing compiler/dependencies or a failed package gate leaves a functional milestone incomplete. Never substitute a host binary or WASM for firmware.

Documentation-only edits need documentation checks; they do not establish a new hardware milestone or require claims of fresh firmware validation. Report previously recorded validation separately from checks actually run for the change.

## Validation and release discipline

- Select native tests for the affected invariants, browser tests for complete input/recording paths, and `tests/hardware/*.txt` platform-host traces for adapter behavior. Functional milestones still require the full gate.
- Preserve empty/silent startup; first-stamp draft capture; tap-toggle data retention; hold/menu transactions and Undo; rests during lock/motion edits; armed-track ownership; ROOT release; Panic and USB-loss behavior.
- Test both touch and mouse activation. Passing `.click()` does not establish that a real touch invokes an action once.
- Read PR reviews, inline comments and review workflow results, including recent merged work. A finished review job is not evidence of no findings. The milestone contract accepts noncritical debt and requires attention to critical risks; documentation-only edits do not need another review cycle.
- State what was tested locally, published, audibly evaluated and physically tested separately. The checked-in `0.4.3-hw1` report records cross-build success; no physical FM1 acceptance is established.
- Preserve installer identity checks, exact selected MIDI-pair confirmation, backup acknowledgement and cancellation semantics. Never use force-install flags, replace a bootloader, access a device or flash as a side effect of software validation. Physical flashing requires separate explicit authorization.

Mix levels/mutes are separate session performance state; score Undo and recorded locks do not change them. The typed step clipboard and audition context live in the C core. Both wrappers consume shared key roles/light masks. Native/WASM parity includes clipboard paste and mixer changes; `tests/hardware_ux_browser.cjs` and `tests/hardware/ux.txt` cover the corresponding browser/platform paths. See [UX_DESIGN.md](UX_DESIGN.md) for control roles, research and acceptance tasks.

Normal use is session-only with no project saving. Preserve the zero-storage-write host expectation and leave Bluetooth, Wi-Fi and CPU2 off. Do not assign a licence to original code. Retain installed-release vocabulary and third-party notices when updating distribution files.

## Keeping orientation accurate

Update [PRODUCT.md](PRODUCT.md) for user-visible behavior and [ROADMAP.md](ROADMAP.md) for accepted decisions or changed priorities. Update this guide for architecture/tooling changes and [HARDWARE.md](HARDWARE.md) for build or physical acceptance evidence. Keep historical research explicitly historical. Older README sections contain superseded two-lane and recording descriptions; current source/tests and the later FM1 control section are the starting point when investigating a mismatch.

The published product URL is https://stephencarr.github.io/UNDERGROWTH/. A commit, draft PR or successful local build does not by itself verify the deployed site.

The draft ROOT trial expands parameter masks to 32 bits (20 slots), preserving
legacy seven-slot stamping on the other organisms. ROOT's track-enable switch
(slot 17) is excluded from locks/motion. Character occupies 7–10, atmosphere
11–18, with independent atmosphere send at 16. `ug_grain` exports display-only
telemetry. `root_atmosphere_test` covers the cloud and high-bit score semantics;
`root_atmosphere_browser.cjs` exercises real mouse/touch panel paths. The texture
is generated once at init, has no external sample licence, and is held in pool
memory on FM1. The new parameters are included in native/WASM parity.
## Railway browser previews

Railway's UNDERGROWTH project uses persistent `preview-base` for the selected
prototype branch, with automatic PR environments enabled. The current ROOT
preview follows `codex/root-character-atmosphere`. PR #9 carries the shared
hosting helpers for future branches from `main`; ROOT already includes them.
GitHub Pages remains
the accepted instrument. Railway only packages the checked-in browser bundle;
the normal milestone gate must build matching WASM/firmware before a functional
change is accepted. Hosting configuration does not replace musical validation.

`railway.json` builds with `tools/prepare_preview.py` and runs the standard-library
Python server in `tools/serve_preview.py` on `$PORT`. Only `/`, `/index.html` and
`/version.json` are public routes; the root bundle embeds its scripts and WASM.
The version badge identifies branch, commit and PR when Railway supplies them.
There is no repository directory serving, directory listing or preview flasher.
Unknown paths, including source, credentials and firmware paths, return 404.

Run `python3 tests/preview_server_test.py` to verify the access boundary. PR
previews update on branch pushes and Railway removes their environments when
PRs close or merge. A Railway-provided domain on the base service enables
automatic domains for PR environments. Verify deployment SUCCESS, version.json
and a real browser audio/ROOT workflow before handing someone a preview URL.

Accepted/deferred preview review debt (PR #9): the bundled UX design guide link
still points to a path outside the public route allowlist and returns 404. The
allowlist stays restricted; repairing that navigation is noncritical follow-up.

Shared FX live in the score's five global values: reverb return/decay/damping/
width and delay return. `ug_setfx` / `ug_getfx` expose them to both adapters;
they cannot be changed during a take. The Clouds-derived room in `core/room.h`
uses 49,200 float pool samples and ten rate-scaled circular lines (8–96 kHz),
with four input allpasses and a modulated cross-coupled tank. No render allocation.
Wet zero fades and clears the arena once before bypass. Panic clears it too.
Sends feed delay and reverb in parallel; global settings have no instrument locks.
`reverb_response_test` checks impulse density and decay independently of source
RMS; `reverb_test` covers routing, ownership, balance, extremes and lifecycle.
See [reverb evaluation](REVERB_EVALUATION.md) for source/licensing, cost and audio.

ROOT atmosphere independence (0.4.3-hw1): ordinary ROOT stamping captures slots
0–10 only. Slots 11–18 are not implicitly locked by note entry. The continuous
drone runs with no steps and retains its level/send when a bass sequence plays.

ROOT send ownership (0.4.3-hw1): ordinary stamping captures slots 0–10 except
Synth send 3 (mask 2039). Sound/Character and articulation remain per trig;
ENV remains independent. Intentional send locks and motion still work. The
ROOT FX screen shows LIVE effective sends during playback, and turning a send
starts from that sounding value. Stopped controls edit the draft. Expanded
Synth/ENV send sliders also show effective values in playback; other accepted
Expanded feedback debt remains. `tools/reverb_audition.py` renders unnormalised
ROOT/empty-sequence ENV WAVs and metrics for human audition.

## CHORD candidate — 0.5.0-hw1

The core now exposes five tracks; `ug_state(23)` is the count, with CHORD takeover,
trigger count and motion mask at 24–26. `core/chord.h` implements two banks of four
tones using original sine/polyBLEP saw/pulse synthesis and stable two-pole tone
filtering. No extra pool buffer or render allocation. Slots 0–6 are Tone, Colour,
Decay, Send, Attack, Release, Sweep; 7 root, 8 palette slot, 9 inversion, 10 spread,
11 gate, 13–15 intervals above the root. Normal stamping excludes Send and unused
slot 12 (mask 61431). Harmony is latched per hit; continuous sound controls smooth.

Eight palettes live inside `Score`, including Undo. Selecting a slot expands its
actual intervals/voicing into draft or explicit trig locks; playback never consults
a mutable palette reference. `ug_chordkey` owns live/MIDI root gates and recording;
`ug_chordtone` exposes read-only sounding-tone telemetry. Both adapters use shared
controller pages; fifth-track mixing uses HOME SELECT page 4 and lower key 5.
Tests include `chord_test`, `chord_browser.cjs`, `tests/hardware/chord.txt` and
active chord events in native/WASM parity. See CHORD.md for current limits.
