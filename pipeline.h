#ifndef PIPELINE_H
#define PIPELINE_H

#include <map>
#include <string>
#include <vector>
#include <set>
#include "data_types.h"
#include "wb_buffer.h"


#define PIPELINE_OFU_NUMBER     2
#define PIPELINE_ALU_NUMBER     3
#define PIPELINE_NUMBER         1


//  Instruction pipeline stage info
typedef struct istage {

    //  instruction running number
    int inumber;

    //  First clock and duration of pipeline instruction performing
    int start_clock;
    int duration;

    //  Hazard pointers
    std::vector<ADDR> hp;

} istage_t;


typedef struct wb_operation {
    ADDR    adr;
    int     finish_clock;
} wbop_t;


typedef enum istage_name {
    IFETCH_STAGE    =   0,
    IDECODE_STAGE   =   1,
    OP1FETCH_STAGE  =   2,
    OP2FETCH_STAGE  =   3,
    EXECUTE_STAGE   =   4,
    WRITEBACK_STAGE =   5,
    STAGE_NUMBER    =   6

} istage_name_t;


typedef struct pipeline_unit_free_clock {
    int unit_number;
    int clock;
    int duration;
} pufclk_t;


class pdp_processor;
typedef struct op operand;


class pipeline
{
private:

    pdp_processor *proc;

    std::map< int, std::vector<istage_t> > instructions = {
        { IFETCH_STAGE,     {} },
        { IDECODE_STAGE,    {} },
        { OP1FETCH_STAGE,   {} },
        { OP2FETCH_STAGE,   {} },
        { EXECUTE_STAGE,    {} },
        { WRITEBACK_STAGE,  {} },
    };


    std::string istage_names[STAGE_NUMBER] = {
        "Instruction fetch",
        "Inctruction decode",
        "Operand 1 fetch",
        "Operand 2 fetch",
        "Execution",
        "Write back"
    };

    //  Pipeline statistics
    pstat_t statistics = {
        0,
        0
    };

    //      Access indicators
    std::set<int>    OFU[PIPELINE_OFU_NUMBER];  //  Operand fetch unit
    std::set<int>    ALU[PIPELINE_ALU_NUMBER];


    pufclk_t    get_OFU_free_clock(int start_clock, int duration);
    bool        take_OFU(pufclk_t fclk);
    bool        OFU_is_available(pufclk_t fclk);

    //  Addresses in write back buffer
    std::vector<wbop_t> wb_buf = {};

    //  Pipelines
    std::map< int, std::vector<istage_t> > pipe[PIPELINE_NUMBER];

    bool        pipeline_lazy_init();


    bool        perform_instruction(int instr);
    bool        perform_first_instruction();
    bool        perform_stage(int instr, int stage);
    bool        perform_OP1F(int instr, int free_clock);
    bool        perform_OP2F(int instr, int free_clock);
    bool        perform_WB(int instr, int free_clock);


    bool        hp_in_wb(std::vector<ADDR> &hp);
    int         get_wb_clock_for_hp(std::vector<ADDR> &hp);
    int         wb_flush(std::vector<ADDR> &fp);
    std::vector<ADDR> &get_all_wb_adr();

public:
    pipeline();

    bool        istage_push(int stage_number, istage_t stage_info);
    bool        run(bool verbose = false);
    void        instructions_print();
    void        pipe_print(int pnumber = 0);
    void        print_statistics();
    void        print_OFU();
    pstat_t     get_statistics();

    std::vector<ADDR>   of_get_hp(int addr_mode, operand op);
};

#endif // PIPELINE_H
