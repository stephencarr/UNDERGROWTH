#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
CC=${CC:-cc}
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/core_test.c -lm -o build/core_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/groove_test.c -lm -o build/groove_test
$CC -std=c99 -O2 -ffp-contract=off core/undergrowth.c tests/render.c -lm -o build/render
ZIG=${ZIG:-"$(command -v zig || true)"}
if [ ! -x "$ZIG" ]; then echo 'Set ZIG to a Zig 0.13 compiler executable.' >&2; exit 1; fi
ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$PWD/build/zig-cache}" "$ZIG" cc -target wasm32-freestanding -std=c99 -O2 -ffp-contract=off -nostdlib -s -Wl,--no-entry -Wl,--export=ug_groove -Wl,--export=ug_getgroove -Wl,--export=ug_init -Wl,--export=ug_render -Wl,--export=ug_buffer -Wl,--export=ug_play -Wl,--export=ug_hit -Wl,--export=ug_record -Wl,--export=ug_tempo -Wl,--export=ug_master -Wl,--export=ug_setparam -Wl,--export=ug_getparam -Wl,--export=ug_effective -Wl,--export=ug_setstep -Wl,--export=ug_togglestep -Wl,--export=ug_step -Wl,--export=ug_setlock -Wl,--export=ug_clearlock -Wl,--export=ug_lockmask -Wl,--export=ug_lockvalue -Wl,--export=ug_begin_edit -Wl,--export=ug_undo -Wl,--export=ug_clear -Wl,--export=ug_rejoin -Wl,--export=ug_panic -Wl,--export=ug_state -Wl,--export=ug_state_bytes core/undergrowth.c -o web/core.wasm
