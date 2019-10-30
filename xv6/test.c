#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main() {
    int frames[16384];
    int fr;
    for (fr = 0; fr < 16384; fr++) {
        frames[fr] = -5;
    }
    int pids[16384];
    int numframes = 0;  // Probably not correct

    dump_physmem(frames, pids, numframes);
    
    return 0;
}