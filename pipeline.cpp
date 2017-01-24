#include "pipeline.h"

#include <iostream>
#include <iterator>
#include <numeric>
#include <algorithm>
#include "pdp_processor.h"


using std::cout;
using std::endl;


pipeline::pipeline()
{
}


bool pipeline::istage_push(int stage_number, istage_t stage_info) {
    instructions[stage_number].push_back(stage_info);
    return true;
}


bool pipeline::run(bool verbose) {
    if(!pipeline_lazy_init())
        return false;

    //  Check for all instructions have all pipeline stages
    int instructions_number = instructions[0].size();
    for(int i = 1; i < STAGE_NUMBER; ++i)
        if(instructions[i].size() != instructions_number)
            return false;

    for(int curr_instr = 0; curr_instr < instructions_number; ++curr_instr) {
        if(!perform_instruction(curr_instr))
            return false;
    }

    //  Calculate statistics
    int clk = instructions[WRITEBACK_STAGE].back().start_clock +
              instructions[WRITEBACK_STAGE].back().duration - 1,
        pipe_clk = pipe[0][WRITEBACK_STAGE].back().start_clock +
                   pipe[0][WRITEBACK_STAGE].back().duration - 1;
    statistics.without_pipeline_clocks = clk;
    statistics.pipeline_clocks = pipe_clk;

    if(verbose) {
        instructions_print();
        pipe_print();
    }

    return true;
}


bool pipeline::perform_instruction(int instr) {
    //  If first instruction then just copy it
    if(instr == 0)
        if(!perform_first_instruction())
            return false;
        else
            return true;

    for(int stage_i = 0; stage_i < STAGE_NUMBER; ++stage_i) {
        if(!perform_stage(instr, stage_i))
            return false;
    }

    return true;
}


bool pipeline::perform_first_instruction() {
    for(int i = 0; i < STAGE_NUMBER; ++i) {
        if(pipe[0][i].size() != 0)
            return false;

        pipe[0][i].push_back(instructions[i][0]);
        if(i == WRITEBACK_STAGE)
            wb_buf.push_back({instructions[i][0].hp[0],
                              instructions[i][0].start_clock +
                              instructions[i][0].duration - 1});
    }

    return true;
}


bool pipeline::perform_stage(int instr, int stage) {
    //  For one pipeline

    //  Next free clock is the minimum from
    //  next free clock on particular stageline
    //  and next free clock on particular instruction line
    int ifclk = stage > 0 ? pipe[0][stage - 1][instr].start_clock +
                            pipe[0][stage - 1][instr].duration : -1;
    int stfclk = pipe[0][stage][instr - 1].start_clock +
                 pipe[0][stage][instr - 1].duration;
    int free_clock = std::max(ifclk, stfclk);

    //  Time to flush (if it needs)
    int flushing_clocks = 0;

    //  Some actions individual for each stage
    switch(stage) {
        case OP1FETCH_STAGE: case OP2FETCH_STAGE: {            
            free_clock = pipe[0][IDECODE_STAGE][instr].start_clock +
                         pipe[0][IDECODE_STAGE][instr].duration;
            pufclk_t fclk = get_OFU_free_clock(free_clock,
                                instructions[stage][instr].duration);
            if(!take_OFU(fclk)) {
                return false;
            }
            free_clock = fclk.clock;
            }
            break;

        case EXECUTE_STAGE:
            free_clock = std::max(pipe[0][OP1FETCH_STAGE][instr].start_clock +
                                  pipe[0][OP1FETCH_STAGE][instr].duration,
                                  free_clock);
            break;

        case WRITEBACK_STAGE: {
            if(instructions[stage][instr].hp.size() != 1)
                return false;

            wb_buf.push_back({instructions[stage][instr].hp[0],
                              free_clock + flushing_clocks +
                              instructions[stage][instr].duration - 1});
            if(wb_buf.size() >= WB_BUFFER_SIZE) {
                flushing_clocks = wb_flush(get_all_wb_adr());
            }

            pipe[0][stage].push_back({instr,
                                      free_clock + flushing_clocks,
                                      instructions[stage][instr].duration,
                                      {}
                                     });
            return true;
            }
            break;

        default:
            break;
    }

    //  Read after write checking    
    if(hp_in_wb(instructions[stage][instr].hp) && stage != WRITEBACK_STAGE) {
        //  Clock when wb is finished
        int free_clock_after_wb = get_wb_clock_for_hp(instructions[stage][instr].hp) + 1;

        //  If this time is later we have to wait
        if(free_clock_after_wb > free_clock)
           free_clock = free_clock_after_wb;

        //  We need to flush all hazard pointers from WB buffer
        flushing_clocks += wb_flush(instructions[stage][instr].hp);
    }

    pipe[0][stage].push_back({instr,
                              free_clock + flushing_clocks,
                              instructions[stage][instr].duration,
                              {}
                             });
    return true;
}


bool pipeline::hp_in_wb(std::vector<ADDR> &hp) {
    if(hp.size() == 0 || wb_buf.size() == 0)
        return false;

    std::vector<ADDR>::iterator it;
    for(it = hp.begin(); it != hp.end(); ++it) {
        std::vector<wbop_t>::iterator jt;
        for(jt = wb_buf.begin(); jt != wb_buf.end(); ++jt) {
            if(*it == (*jt).adr) {
                return true;
            }
        }
    }

    return false;
}


int pipeline::get_wb_clock_for_hp(std::vector<ADDR> &hp) {
    if(hp.size() == 0 || wb_buf.size() == 0)
        return -1;

    std::vector<ADDR> clocks = {};

    std::vector<ADDR>::iterator it;
    for(it = hp.begin(); it != hp.end(); ++it) {
        //cout << "hp[" << *it << "]";
        std::vector<wbop_t>::iterator jt;
        for(jt = wb_buf.begin(); jt != wb_buf.end(); ++jt) {
            if(*it == (*jt).adr) {
                clocks.push_back((*jt).finish_clock);
                //cout << "clk[" << (*jt).finish_clock << "]";
            }
        }
    }

    // return max from clocks
    return (*std::max_element(clocks.begin(), clocks.end()));
}


int pipeline::wb_flush(std::vector<ADDR> &fp) {
    if(fp.size() == 0 || wb_buf.size())
        return 0;

    int flushing_clocks = 0;
    std::vector<ADDR>::iterator it;
    for(it = fp.begin(); it != fp.end(); ++it) {
        std::vector<wbop_t>::iterator jt;
        for(jt = wb_buf.begin(); jt != wb_buf.end(); ++jt) {
            if(*it == (*jt).adr) {
                wb_buf.erase(jt);
                if((*jt).adr >= RAM_SIZE)
                    flushing_clocks += REGISTER_ACCESS;
                else
                    flushing_clocks += MEMORY_ACCESS;
            }
        }
    }

    return flushing_clocks;
}


std::vector<ADDR> &pipeline::get_all_wb_adr() {
    std::vector<ADDR> addresses = {};
    std::vector<wbop_t>::iterator it;
    for(it = wb_buf.begin(); it != wb_buf.end(); ++it)
        addresses.push_back((*it).adr);

    return addresses;
}


void pipeline::instructions_print() {
    cout << "****   Pipeline without pipeline    ****" << endl;
    for(int i = 0; i < STAGE_NUMBER; ++i) {
        cout << "###    " << istage_names[i] << endl;
        for(int j = 0; j < instructions[i].size(); ++j) {
            cout << "#  " << instructions[i][j].inumber <<
                    " : (" << instructions[i][j].start_clock <<
                    ", " << instructions[i][j].duration << ")" << endl;
        }
    }
    cout << "*****************************************" << endl;
}


void pipeline::pipe_print(int pnumber) {
    cout << "****   Pipeline    ****" << endl;
    for(int i = 0; i < STAGE_NUMBER; ++i) {
        cout << "###    " << istage_names[i] << endl;
        for(int j = 0; j < pipe[pnumber][i].size(); ++j) {
            cout << "#  " << pipe[pnumber][i][j].inumber <<
                    " : (" << pipe[pnumber][i][j].start_clock <<
                    ", " << pipe[pnumber][i][j].duration << ")" << endl;
        }
    }
    cout << "************************" << endl;
}


bool pipeline::pipeline_lazy_init() {
    for(int i = 0; i < PIPELINE_NUMBER; ++i)
        pipe[i] = {
                { IFETCH_STAGE,     {} },
                { IDECODE_STAGE,    {} },
                { OP1FETCH_STAGE,   {} },
                { OP2FETCH_STAGE,   {} },
                { EXECUTE_STAGE,    {} },
                { WRITEBACK_STAGE,  {} },
            };

    for(int i = 0; i < PIPELINE_OFU_NUMBER; ++i)
        OFU[i] = {};

    for(int i = 0; i < PIPELINE_ALU_NUMBER; ++i)
        ALU[i] = {};

    wb_buf = {};

    return true;
}


std::vector<ADDR> pipeline::of_get_hp(int addr_mode, operand op) {
    std::vector<ADDR> hp = {};

    switch(addr_mode) {
        case REGISTER:
            hp.push_back(op.adr);
            break;

        case REGISTER_DEFERRED:
            hp.push_back(op.adr);
            break;

        case AUTOINCREMENT:
            hp.push_back(op.adr);
            break;

        case AUTOINCREMENT_DEFERRED:
            hp.push_back(op.adr);
            hp.push_back(op.val);
            break;

        case AUTODECREMENT:
            hp.push_back(op.adr);
            break;

        case AUTODECREMENT_DEFERRED:
            hp.push_back(op.adr);
            hp.push_back(op.val);
            break;

        case INDEX:
            hp.push_back(op.adr);
            hp.push_back(op.val);
            break;

        case INDEX_DEFERRED:
            hp.push_back(op.adr);
            hp.push_back(op.val);
            break;

        case BRANCH:
            break;

        default:
            break;
    }

    return hp;
}


pstat_t pipeline::get_statistics() {
    return statistics;
}


void pipeline::print_statistics() {
    cout << "###    Pipeline statistics   ###" << endl;
    cout << "# Clocks without pipeline: " <<
            statistics.without_pipeline_clocks << endl;
    cout << "# Clocks with    pipeline: " <<
            statistics.pipeline_clocks << endl;
}


pufclk_t pipeline::get_OFU_free_clock(int start_clock, int duration) {
    int clk = start_clock;

    while(true) {
        for(int i = 0; i < PIPELINE_OFU_NUMBER; ++i)
            if(OFU_is_available({i, clk, duration}))
                return {i, clk, duration};

        clk++;
    }
}


bool pipeline::OFU_is_available(pufclk_t fclk) {
    for(int i = 0; i < fclk.duration; ++i)
        if(OFU[fclk.unit_number].find(fclk.clock + i) !=
           OFU[fclk.unit_number].end())
            return false;

    return true;
}


bool pipeline::take_OFU(pufclk_t fclk) {
    if(!OFU_is_available(fclk))
        return false;

    if(fclk.duration == 0)
        return true;

    for(int i = 0; i < fclk.duration; ++i)
        OFU[fclk.unit_number].insert(fclk.clock + i);

    return true;
}


void pipeline::print_OFU() {
    for(int i = 0; i < PIPELINE_OFU_NUMBER; ++i) {
        cout << endl << "###  OFU[" << i << "]  ##" << endl;
        std::set<int>::iterator it;
        for(it = OFU[i].begin(); it != OFU[i].end(); ++it) {
            cout << *it << " ";
        }
    }
}
