#ifndef PDP_TESTER_H
#define PDP_TESTER_H

#pragma once


#define MEM_TEST_ITER    100
#define PROC_TEST_ITER   100

//#include "pdp_memory.h"
//#include "pdp_processor.h"
//#include "pdp.h"


class pdp_memory;
class pdp_processor;
class pdp;


class pdp_tester
{
private:
    pdp_memory *mem_test;
    pdp_processor *proc_test;
    pdp *pdp_test;


public:
    pdp_tester();


    /*
     *
     *  Memory tests
     *
     */

    bool    memory_test();


    bool    mem_test_b_write();
    bool    mem_test_b_read();
    bool    mem_test_w_write();
    bool    mem_test_w_read();

    bool    mem_test_get_reg_addr();
    bool    mem_test_get_reg_data();
    bool    mem_test_set_reg_data();

    bool    mem_test_get_PSW_flag();
    bool    mem_test_set_PSW_flag();
    bool    mem_test_clr_PSW_flag();

    bool    mem_test_get_PSW();

    /*
     *
     *  Processor tests
     *
     */

    bool processor_test();


    bool    proc_test_get_reg_op();
    bool    proc_test_get_reg_def_op();
    bool    proc_test_get_autoinc_op();
    bool    proc_test_get_autoinc_def_op();
    bool    proc_test_get_autodec_op();
    bool    proc_test_get_autodec_def_op();
    bool    proc_test_get_index_op();
    bool    proc_test_get_index_def_op();
    bool    proc_test_get_branch_op();

    bool    proc_test_instruction_fetch();
    bool    proc_test_write_back();

    bool    proc_test_disasm_curr_instr();

//  Too difficult to test:
//    bool    proc_test_instruction_decode();
//    bool    execute(WORD instr);


    bool    proc_test_ex_clr();

    /*
     *
     *  PDP tests
     *
     */
};

#endif // PDP_TESTER_H
