#include "equiv-os.h"
#include "user-progs/byte-array-prog-vec.h"

// run a single program respawning.
void init(pctx_t *ctx) {
    sys_exec(ctx, &prog_vec, 0x2e18f06c)->respawn = R;
}
