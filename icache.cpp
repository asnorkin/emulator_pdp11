#include "icache.h"
#include <iostream>

using std::cout;
using std::endl;


icache::icache(pdp_memory *memory) {
    is_init = false;
    this->memory = memory;
    stat.hits   = 0;
    stat.misses = 0;
    missed_flag = false;
}


WORD icache::get_instr(ADDR pc) {
    if(pc % 2 != 0)
        return 0;

    //  First call
    if(!is_init) {
        stat.misses += 1;
        missed_flag = true;
        fill_cache(pc);
        is_init = true;

    //  Cache hit
    } else if(zero_pc <= pc && pc < zero_pc + ICACHE_SIZE * 2) {
        stat.hits += 1;
        missed_flag = false;

    //  Cache miss
    } else {
        stat.misses += 1;
        missed_flag = true;
        fill_cache(pc);
    }

    //  Debug
    std::cout << "# hits   " << stat.hits << std::endl;
    std::cout << "# misses " << stat.misses << std::endl;

    int instr_idx = (pc - zero_pc) / 2;
    return cache[instr_idx];
}


bool icache::fill_cache(ADDR pc) {
    zero_pc = pc - pc % (ICACHE_SIZE * 2);
    for(int i = 0; i < ICACHE_SIZE; ++i)
        cache[i] = memory->w_read(pc + i * 2);

    return true;
}


ic_stat_t icache::get_stat() {
    return stat;
}


bool icache::is_missed() {
    return missed_flag;
}
