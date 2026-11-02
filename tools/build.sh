#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
CC=${CC:-cc}
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c core/controller.c tests/controller_test.c -lm -o build/controller_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/core_test.c -lm -o build/core_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/groove_test.c -lm -o build/groove_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/motion_test.c -lm -o build/motion_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/workflow_test.c -lm -o build/workflow_test
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/environment_test.c -lm -o build/environment_test
$CC -std=c99 -O2 -ffp-contract=off core/undergrowth.c core/controller.c tests/render.c -lm -o build/render
$CC -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Werror -Wno-misleading-indentation core/undergrowth.c tests/root_test.c -lm -o build/root_test
ZIG=${ZIG:-"$(command -v zig || true)"}
if [ ! -x "$ZIG" ]; then echo 'Set ZIG to a Zig 0.13 compiler executable.' >&2; exit 1; fi
ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$PWD/build/zig-cache}" "$ZIG" cc -target wasm32-freestanding -std=c99 -O2 -ffp-contract=off -nostdlib -s -Wl,--no-entry -Wl,--export=ug_ui_edit -Wl,--export=ug_ui_meter -Wl,--export=ug_ui_init -Wl,--export=ug_ui_button -Wl,--export=ug_ui_turn -Wl,--export=ug_ui_key -Wl,--export=ug_ui_select -Wl,--export=ug_ui_gesture -Wl,--export=ug_ui_tick -Wl,--export=ug_ui_state -Wl,--export=ug_ui_refresh -Wl,--export=ug_ui_line -Wl,--export=ug_velocity -Wl,--export=ug_getvelocity -Wl,--export=ug_keyvelocity -Wl,--export=ug_arm -Wl,--export=ug_recmode -Wl,--export=ug_draftnote -Wl,--export=ug_draftvalue -Wl,--export=ug_stamp -Wl,--export=ug_erase -Wl,--export=ug_note -Wl,--export=ug_notevalue -Wl,--export=ug_key -Wl,--export=ug_motion -Wl,--export=ug_groove -Wl,--export=ug_getgroove -Wl,--export=ug_init -Wl,--export=ug_render -Wl,--export=ug_buffer -Wl,--export=ug_play -Wl,--export=ug_hit -Wl,--export=ug_record -Wl,--export=ug_tempo -Wl,--export=ug_master -Wl,--export=ug_setparam -Wl,--export=ug_getparam -Wl,--export=ug_effective -Wl,--export=ug_setstep -Wl,--export=ug_togglestep -Wl,--export=ug_step -Wl,--export=ug_setlock -Wl,--export=ug_clearlock -Wl,--export=ug_lockmask -Wl,--export=ug_lockvalue -Wl,--export=ug_begin_edit -Wl,--export=ug_undo -Wl,--export=ug_clear -Wl,--export=ug_rejoin -Wl,--export=ug_panic -Wl,--export=ug_state -Wl,--export=ug_state_bytes core/undergrowth.c core/controller.c -o web/core.wasm

chmod 644 web/core.wasm
