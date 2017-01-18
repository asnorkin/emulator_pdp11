#ifndef ICACHE_H
#define ICACHE_H

#include "data_types.h"
#include "pdp_memory.h"

/*
 *  Instruction cache for pdp processor
 */


#define ICACHE_SIZE             16
#define ICACHE_ASSC             2


//  Cache statistics
typedef struct ic_stat {
    int hits;
    int misses;
} ic_stat_t;


class icache
{
private:
    WORD    cache[ICACHE_SIZE] = {};

    //  Memory pointer for forward communication
    pdp_memory *memory;

    //  PC of zero inctruction in cache
    WORD    zero_pc;

    //  Statistics
    ic_stat_t stat;

    //  Lazy init
    bool    is_init;

    //  True if last call was a miss
    bool    missed_flag;

    bool    fill_cache(ADDR pc);

public:
    icache(pdp_memory *memory);

    WORD        get_instr(ADDR pc);
    ic_stat_t   get_stat();
    bool        is_missed();
};

#endif // ICACHE_H
