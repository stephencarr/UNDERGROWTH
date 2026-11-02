# UNDERGROWTH ENV and reverb handover

Updated 2 November 2026. Resume with the ENV and reverb work below. The user requested a clean agent and clean context; this document is the starting point, not a claim that the audio problem is solved.

## Immediate objective

The user is testing https://stephencarr.github.io/UNDERGROWTH/. They confirm ENV produces audio and sends to reverb, but the result is too subtle. Synth reverb also now feels too subtle at 100% send and 100% Reverb, with Decay needing 100% for a noticeable effect. Their requirement is that maximum reverb should drown the sound, with useful ambience below maximum Decay.

The user authorizes choosing a suitable established reverb: Mutable Instruments Clouds is optional, and community firmware algorithms are equally welcome. Evaluate sound and target-device cost before selecting. No replacement algorithm has been implemented. Do not describe this as another confirmed missing ENV signal or ask again whether PLAY was on: it was.

Two work items remain:

1. Fix the confirmed synth-send ownership problem: ordinary ROOT steps still capture Synth send, allowing a stored zero to override the displayed 100% during playback.
2. Evaluate and implement a reverb that actually supplies the dense, enveloping wash requested, on both synth and ENV. Signal presence and RMS alone are insufficient acceptance criteria.

## Intended workflow agreed with the user

- ROOT synth plays keyboard/sequencer notes. EDIT Sound and Character shape it. Per-step synth macro locks, especially Colour and Decay, are wanted.
- ROOT ENV is an independent continuous granular drone. It does not need steps and does not follow bass-note pitches. Its own pitch, grain position/size/density, scan, level and enable controls own the atmosphere.
- ENV runs with Enabled ON, nonzero Level and PLAY, including on an empty sequence. STOP fades it out. ROOT's track mixer/mute affects both layers. Keyboard playing alone triggers the synth.
- Synth send and ENV send independently feed shared effects. Global Reverb controls the shared return/character. Currently sends feed both delay and reverb in parallel; delay has a separate return control.
- Ordinary ROOT notes must never capture or restore atmosphere settings. The user did not ask for ENV step locks. Legacy implementation has explicit atmosphere lock/motion paths, but these should not be presented as the intended ENV workflow. Decide their eventual ownership deliberately rather than letting implementation dictate UX.
- At full send and full Reverb, the dry sound should be swallowed. Midrange Decay should already give useful ambience. Increasing gain alone is not an adequate substitute for diffusion and musical decay.

## What shipped and what it actually proved

PR #9 added restricted Railway hosting and was merged with explicit user approval. PR #8 implemented the ROOT trial and shared reverb, then merged to main as f572af394c32a0452795d24c01626ff8a351d730, experimental firmware 0.4.1-hw1.

PR #10 fixed accidental atmosphere snapshots and merged to main as 99640c8bcf0b206afe4775ef5595d137dcf62650. This is published experimental 0.4.2-hw1:

https://github.com/stephencarr/UNDERGROWTH/pull/10

Before #10, ug_stamp captured ROOT slots 0–19 except Enabled 17. Entering notes at default ENV level 30% and send 0%, then raising the displayed ENV values while stopped, caused playback to restore the old values. The native reproduction printed displayed send 1.00 / actual send 0.00 and displayed level 1.00 / actual level 0.30, with active grains and PLAY on.

The fix changes ROOT's ordinary snapshot mask to slots 0–10 only (2047). Percussion remains seven slots (127). The same reproduction now reports actual level/send 1.00. It does not change the reverb DSP or synth-send ownership.

Full tools/check_milestone.sh passed after the change: native tests, native/WASM bit parity, mouse/touch browser regression, AudioWorklet, simulated updater, platform-host traces, real firmware CRC/identity/instruction/memory checks. PR #10 review completed with no findings. A later test-only change uses a tolerance for encoder floating-point rounding; a documentation-only commit records validation.

Published Pages build and exact downloaded bytes were verified:

- index.html SHA-256: 1e9a304b415a1fb8f718d9a033764709396bd66037edd9d9b27eb505a6a41ebb (verify against work/verify_published.py; do not use this transcription as a release gate).
- Firmware 0.4.2-hw1 SHA-256: 03e4fecccf99bcf7fc904371801463ff8485bf9a4a09b4b577c7ec9dc31f1df0.
- Firmware identity FM-1_731000402; app 118668 bytes; data+bss 40604/98304; pool 236108/344064.

These checks proved the snapshot fix and build integrity, not listening acceptance. The user subsequently reported the sound still falls short. No physical FM1 flashing, listening or CPU measurement has occurred.

## Confirmed remaining synth-send bug

ROOT Synth send is parameter 3, still included in ordinary note snapshots. Reproduction:

1. Initialize engine, stamp ROOT step 0 at default send 0.
2. Set ROOT parameter 3 to 1 while stopped; set global Reverb to 1.
3. Start playback and render a block.
4. ug_getparam(2,3) returns 1.00, but ug_effective(2,3) returns 0.00.

The FX display shows draft values outside motion mode, so it can report 100% while actual playback applies 0%. Editing during playback sets manual takeover; Rejoin or Stop releases it. This can make repeated tests seem inconsistent.

Consider making ordinary note entry exclude sends while preserving deliberate synth macro locks, and making displayed send values reflect what is actually heard. Inspect manual takeover, Rejoin, audition, explicit locks, motion and rest behavior before choosing exact semantics. Do not silently remove intentional user automation as a shortcut.

## Current DSP and controls

Authoritative shared code is core/undergrowth.c and core/controller.c. Browser JS is IO/drawing only. Parameters for ROOT:

| Slots | Meaning |
| --- | --- |
| 0–6 | Tone, Colour, Decay, Synth send, Glide, Weight, Attack |
| 7–10 | Structure, Intensity, Feedback, Motion |
| 11–14 | Grain position, size, density, pitch |
| 15–18 | ENV level, ENV send, Enabled, Scan |
| 19 | Unused |

ENV page 1 exposes slots 11–14; page 2 exposes Level, Scan, Enabled, ENV send. FX page 1 is layer sends; page 2 global Reverb, Decay, Damping, Width; page 3 Delay return. Global effect parameters are score.fx[5], defaults {0,.55,.45,.8,.75}. They have score Undo and are guarded during recording; global FX automation is future work.

ug_effective priority is audition step, manual takeover, current step lock, then draft. ENV DSP currently also reads ug_effective for its controls. Enabled 17 reads track base directly and cannot be locked. Ordinary notes no longer create atmosphere masks, but explicit legacy APIs can still do so.

The grain sampler has eight fixed grains, 4096 float texture samples, no render-time allocation. The original generated C2 texture mixes fundamental, third harmonic and filtered noise with an envelope. It is quite tonal; a static low drone can make reverb less perceptually obvious than transient-rich audio. This is a hypothesis about listening, not a confirmed root cause.

Current reverb is an original stereo Schroeder room: four parallel combs per side, then two allpasses per side, 30000 float pool samples. Delays scale from 48 kHz lengths {1499,1601,1747,1867,1522,1624,1770,1890,149,421,166,438}. Comb feedback is .55 + .40*Decay; damping coefficient .8 - .75*Damping. Input scale .16; return scale 8*Reverb. Width uses mid/side. Buffer writes are bounded, Panic clears tails, controls smooth over about 5 ms.

At high Reverb, wash = Reverb^4 attenuates each layer's dry signal according to its effective send and also suppresses the separate delay return. At 100% effective send and Reverb, direct audio should be essentially absent. This is why verifying effective send matters before tuning the algorithm.

Existing tests checked full-wet/dry RMS (+1.69 dB ROOT, -0.60 dB ENV under their fixtures), silence, tails, finite extremes and suppression of the direct attack. These tests passed even though the user disliked the result. Add representative audio comparisons and evaluate diffusion/decay across the useful range, not merely nonzero output.

## Reverb research already available

Read-only upstream references are in ignored work/reverb-reference/:

- clouds_dsp_fx_reverb.h
- clouds_dsp_fx_fx_engine.h
- rings_dsp_fx_reverb.h
- rings_dsp_fx_fx_engine.h

Source: Mutable Instruments eurorack, commit f4cfa7697095a28d292bff758beb51deb49b5a86. Clouds uses a Griesinger/Dattorro-style topology with input diffusion, cross-coupled branches, and modulated delays. Its FxEngine<16384, FORMAT_12_BIT> uses uint16 storage, approximately 32 KiB at 32 kHz. Rings uses 32768 uint16 entries, approximately 64 KiB at 48 kHz. Those memory figures are not CPU benchmarks or a drop-in guarantee at other sample rates.

The files carry MIT notices, Copyright 2014 Emilie Gillet. Verify and retain applicable notices before porting. Do not relicense the whole project: original-project licensing is undecided. Community firmware alternatives have not yet been surveyed. Clouds is a candidate, not a selected implementation.

## Recommended next work

1. Read AGENTS.md, docs/PRODUCT.md, docs/ENGINEERING.md, docs/UX_DESIGN.md, docs/ROADMAP.md, docs/HARDWARE.md and LICENSING.md. Older sections contain historical descriptions; use this handover and current code to resolve the latest ENV requirements.
2. Fetch main and create a fresh working branch. Preserve the published snapshot fix.
3. Reproduce synth send mismatch using actual panel paths. Design clear send ownership and display behavior; test stopped edits, playback edits, Rejoin and Stop/Play.
4. Compare a small set of established reverbs, including community firmware if suitable. Check licensing, sample-rate assumptions, fixed memory and CPU cost before porting into shared C.
5. Produce isolated dry/wet ROOT and ENV recordings across Decay settings and with an empty ENV sequence. Check actual effective sends, dry suppression, tail shape, density, clipping and stereo behavior. Do not claim subjective listening from an RMS test.
6. Build the browser and matching next experimental firmware (0.4.3-hw1 unless another release has appeared). Run the mandatory full gate and obtain/read the PR review. Publish only the complete milestone; verify deployed bytes. User listening remains the acceptance test for the wash.

The user wants action without repeated permission prompts. The prior agent agreed to evaluate and implement a suitable reverb and fix synth-send ownership. This handover request pauses implementation for a fresh agent; no new DSP changes were made after 0.4.2.

## Repository and tooling

Repository: /workspace/UNDERGROWTH, remote https://github.com/stephencarr/UNDERGROWTH (private). Before this handover, working branch codex/root-character-atmosphere was clean at aa4ca742fa2de077d06233e475c3d4b71e49a8aa, already merged by PR #10. Main merge commit is listed above. Scratch files under ignored work/ may not survive a new workspace.

Useful scratch files: work/reproduce_env_route.c, work/reproduce_env_empty.c, work/reverb_trace.c, work/verify_published.py, work/env-fix-gate.log and work/env-fix-firmware.log. Regression files committed in tests/reverb_test.c, tests/reverb_browser.cjs and tests/environment_test.c.

Build commands from repo root:

```sh
export ZIG="$PWD/.tools/ziglang/zig"
export JIELI_TOOLCHAIN="$PWD/.tools/hardware/toolchain/jieli-linux-toolchains-20250324.1"
sh tools/build.sh
python3 tools/bundle.py
python3 tools/firmware_build.py --release 0.4.3-hw1
# Publish matching release artifacts and update installer version/hash first.
sh tools/check_milestone.sh
```

Current gate defaults to 0.4.2-hw1. For a new release update tools/check_milestone.sh, tools/firmware_build.py, flash.html, web/flash.js, tests/flash_browser.cjs and current release docs. Preserve old firmware. Copy the new .fwsc, manifest and build report into firmware/releases; update SHA256SUMS and installer hashes. The full gate compares rebuilt firmware with the checked-in package.

Zig is 0.13.0. Pinned dependencies are in .tools/hardware/fm1-base and .tools/hardware/sdk. Chromium is /usr/bin/chromium; require('playwright') works in Node. Browser tests need a server on localhost:8000. A server may already be running; inspect before starting another. In this environment network/browser/local-server commands required exec_command with additional network permissions. Never bypass proxy or TLS verification; Python HTTPS downloads worked for Pages while a Chromium proxy certificate attempt failed.

GitHub CLI is authenticated for reads/pushes; connector tools support PR creation, review request and merge. Request copilot-pull-request-reviewer[bot], wait for the workflow, then read both review body and inline findings. Reviews can take several minutes. The repository explicitly accepts noncritical debt; do not expand scope chasing zero comments. Docs-only changes do not need a fresh functional review or firmware build.

## Hosting state

Accepted product: https://stephencarr.github.io/UNDERGROWTH/ . Main merges trigger Pages. Published 0.4.2 bytes were verified again after the latest complaint.

Railway preview: https://undergrowth-preview-preview-base.up.railway.app . It still follows codex/root-character-atmosphere. Last checked deployment 08f51a41-3c96-4519-b37d-72cfa976ff75 was SUCCESS at aa4ca74. Both services therefore had the latest functional build; the user's selected URL is Pages.

Railway project 839a9b25-bfe7-45b0-84c3-f0af370d40f2; environment preview-base 3f95df2a-12cc-40c9-9ce7-15d1630d2775; service 0c9eca7b-d3dc-45e8-b67e-2e4129beb664. Use the Railway skill and remote MCP for platform reads.

A prior staged source-branch switch to main, patch f3cbdde7-abd0-4fe9-ab86-729c85ac8433, was cancelled without approval. Do not accept that old patch or bundle it into another change. Inspect any pending staged changes before deployment mutations. Automatic PR previews are enabled; do not close/reopen PRs merely to force preview creation.

Restricted hosting only serves /, /index.html and /version.json from prepared build/preview assets. Do not expose the private repository through an unrestricted static server. The public preview omits firmware installation links; Pages retains the experimental installer.

## Constraints and deferred debt

- Shared C owns DSP, sequencing and control semantics. No JS-only synth or sequencer fixes.
- Every functional milestone requires a real validated .fwsc and browser build. No physical flash/device access without separate authorization.
- Keep Bluetooth, Wi-Fi and CPU2 off. Start empty and silent. Session-only: reload loses the pattern.
- Desktop timing is not FM1 CPU headroom. Physical CPU/P/X meter comparisons remain outstanding.
- Preserve updater identity, checksum, confirmation and recovery safeguards. No force install or bootloader changes.
- Accepted noncritical debt includes Expanded grid/key-mode confusion, ENV Enabled take-Undo ownership, global FX/trig Undo grouping, and Expanded controls displaying draft instead of effective playback values. Address only what is necessary for the current send/ENV clarity work; do not bundle unrelated cleanup.
- No subagents unless the user or applicable instructions explicitly request delegation.

## Suggested first message from the next agent

“I’ve read the handover. I’ll separate the synth-send override from the reverb sound, then compare established algorithms on isolated ROOT and ENV audio. The goal is an obvious wash at full settings and useful ambience below maximum Decay, with matching browser and firmware builds.”
