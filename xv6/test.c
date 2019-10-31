#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main() {
    int *frames = malloc(sizeof(int)*16384);
    int *pids = malloc(sizeof(int)*16384);
    int numframes = 16384;  // Probably not correct
    printf(1, "Here");
    dump_physmem(frames, pids, numframes);
    printf(1, "HHere");
    for (int x = 0; x < 16384; x++) {
        if (frames[x] != -1) {
            printf(1, "Frame: %d\n", frames[x]);
        }
    }
    exit();
}