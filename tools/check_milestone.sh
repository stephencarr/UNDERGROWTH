#!/bin/sh
# Mandatory milestone gate; deliberately fails if a real hardware package cannot be built.
set -eu
cd "$(dirname "$0")/.."
sh tools/build.sh
for test in core groove motion root workflow environment controller; do "./build/${test}_test"; done
node tests/parity.mjs
node tests/worklet.mjs
python3 tools/bundle.py
node tests/panel_input.cjs
node tests/workflow_browser.cjs
node tests/environment_browser.cjs
node tests/flash_browser.cjs
python3 tools/firmware_build.py --host
python3 tools/firmware_build.py --release "${UG_RELEASE:-0.1.1-hw1}"
cmp "firmware/build/undergrowth-${UG_RELEASE:-0.1.1-hw1}.fwsc" "firmware/releases/undergrowth-${UG_RELEASE:-0.1.1-hw1}.fwsc"
