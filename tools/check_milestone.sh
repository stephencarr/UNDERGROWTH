#!/bin/sh
# Mandatory milestone gate; deliberately fails if a real hardware package cannot be built.
set -eu
cd "$(dirname "$0")/.."
sh tools/build.sh
./build/room_memory_native > build/room_native.f32
./build/room_memory_fm1 > build/room_fm1.f32
cmp build/room_native.f32 build/room_fm1.f32
for test in grain core groove motion root workflow environment controller backbeat root_atmosphere reverb reverb_response; do "./build/${test}_test"; done
node tests/parity.mjs
node tests/worklet.mjs
python3 tools/bundle.py
node tests/panel_input.cjs
node tests/workflow_browser.cjs
node tests/environment_browser.cjs
node tests/backbeat_browser.cjs
node tests/root_atmosphere_browser.cjs
node tests/reverb_browser.cjs
node tests/hardware_ux_browser.cjs
node tests/flash_browser.cjs
node tests/confirmed_updater.mjs
node tests/flasher_handover.cjs
python3 tools/firmware_build.py --host
python3 tools/firmware_build.py --release "${UG_RELEASE:-0.5.0-hw1}"
cmp "firmware/build/undergrowth-${UG_RELEASE:-0.5.0-hw1}.fwsc" "firmware/releases/undergrowth-${UG_RELEASE:-0.5.0-hw1}.fwsc"
for vocabulary in firmware/web/words/*.json; do cmp "$vocabulary" "words/$(basename "$vocabulary")"; done
