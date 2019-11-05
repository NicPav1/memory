#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main() {
    int *frames = malloc(sizeof(int)*16384);
    int *pids = malloc(sizeof(int)*16384);
    int numframes = 16384;  // Probably not correct
    dump_physmem(frames, pids, numframes);
    for (int x = 0; x < 200; x++) {
        if (frames[x] != -1) {
            printf(1, "Frame: %d  PID: %d\n", frames[x], pids[x]);
        }
    }
    exit();
}