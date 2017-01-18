#include "pdp_tester.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "data_types.h"
#include "pdp_memory.h"
#include "pipeline.h"
#include "pdp_processor.h"
#include "pdp.h"


#define BREAK_TEST(msg) do{                                     \
                            printf("BREAK_TEST : %s\n", msg);   \
                            exit(EXIT_FAILURE);                 \
                        }while(0)


using std::rand;
using std::cout;
using std::endl;


pdp_tester::pdp_tester() {
    mem_test = new pdp_memory();
    pipe_test = new pipeline();
    proc_test = new pdp_processor(mem_test, pipe_test);
    pdp_test = new pdp();
}


/*
 *
 *  Memory tests
 *
 */

bool pdp_tester::memory_test() {

    cout << "########################################" << endl;
    cout << "#            MEMORY TESTS              #" << endl;
    cout << "########################################" << endl;

    if(mem_test_b_write())
        cout << "MEMORY TEST: b_write OK" << endl;
    else
        cout << "MEMORY TEST: b_write FAILED" << endl;

    if(mem_test_b_read())
        cout << "MEMORY TEST: b_read OK" << endl;
    else
        cout << "MEMORY TEST: b_read FAILED" << endl;

    if(mem_test_w_write())
        cout << "MEMORY TEST: w_write OK" << endl;
    else
        cout << "MEMORY TEST: w_write FAILED" << endl;

    if(mem_test_w_read())
        cout << "MEMORY TEST: w_read OK" << endl;
    else
        cout << "MEMORY TEST: w_read FAILED" << endl;

    if(mem_test_get_reg_addr())
        cout << "MEMORY TEST: get_reg_addr OK" << endl;
    else
        cout << "MEMORY TEST: get_reg_addr FAILED" << endl;

    if(mem_test_get_reg_data())
        cout << "MEMORY TEST: get_reg_data OK" << endl;
    else
        cout << "MEMORY TEST: get_reg_data FAILED" << endl;

    if(mem_test_set_reg_data())
        cout << "MEMORY TEST: set_reg_data OK" << endl;
    else
        cout << "MEMORY TEST: set_reg_data FAILED" << endl;

    if(mem_test_get_PSW_flag())
        cout << "MEMORY_TEST: get_PSW_flag OK" << endl;
    else
        cout << "MEMORY TEST: get_PSW_flag FAILED" << endl;

    if(mem_test_set_PSW_flag())
        cout << "MEMORY_TEST: set_PSW_flag OK" << endl;
    else
        cout << "MEMORY TEST: set_PSW_flag FAILED" << endl;

    if(mem_test_clr_PSW_flag())
        cout << "MEMORY_TEST: clr_PSW_flag OK" << endl;
    else
        cout << "MEMORY TEST: clr_PSW_flag FAILED" << endl;

    if(mem_test_get_PSW())
        cout << "MEMORY_TEST: get_PSW OK" << endl;
    else
        cout << "MEMORY TEST: get_PSW FAILED" << endl;


    cout << "########################################" << endl;

    return true;
}

bool pdp_tester::mem_test_b_write() {
    for(int i = 0; i < MEM_TEST_ITER; ++i) {
        ADDR adr = rand() % (RAM_SIZE + REG_SIZE);
        BYTE val = rand() % BYTE_SUP;
        mem_test->b_write(adr, val);

        if(mem_test->RAM[adr] != val)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_b_read() {
    for(int i = 0; i < MEM_TEST_ITER; ++i) {
        ADDR adr = rand() % (RAM_SIZE + REG_SIZE);
        BYTE val = rand() % BYTE_SUP;
        mem_test->RAM[adr] = val;

        if(mem_test->b_read(adr) != val)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_w_write() {
    for(int i = 0; i < MEM_TEST_ITER; ++i) {
        ADDR adr = rand() % (RAM_SIZE + REG_SIZE);
        WORD val = rand() % BYTE_SUP;
        mem_test->w_write(adr, val);

        if(mem_test->RAM[adr] != mem_test->b_read(adr) ||
           mem_test->RAM[adr+1] != mem_test->b_read(adr+1))
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_w_read() {
    for(int i = 0; i < MEM_TEST_ITER; ++i) {
        ADDR adr = rand() % (RAM_SIZE + REG_SIZE);
        WORD val = rand() % BYTE_SUP;
        mem_test->w_write(adr, val);

        if(mem_test->w_read(adr) != val)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_get_reg_addr() {
    for(int i = 0; i < REGISTERS_NUMBER ; ++i) {
        if(mem_test->get_reg_addr(i) != RAM_SIZE + i * 2)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_get_reg_data() {
    for(int i = 0; i < REGISTERS_NUMBER ; ++i) {
        WORD val = rand() % WORD_SUP;
        mem_test->w_write(mem_test->get_reg_addr(i), val);

        if(mem_test->get_reg_data(i) != val)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_set_reg_data() {
    for(int i = 0; i < REGISTERS_NUMBER ; ++i) {
        WORD val = rand() % WORD_SUP;
        mem_test->set_reg_data(i, val);

        if(mem_test->get_reg_data(i) != val)
            return false;
    }

    return true;
}


bool pdp_tester::mem_test_get_PSW_flag() {
    WORD set_f = 1;
    for(int f = 0; f < PSW_FLAGS_NUMBER; ++f) {
        mem_test->w_write(PSW, set_f);
        if(mem_test->get_PSW_flag(f) != 1)
            return false;

        mem_test->w_write(PSW, 0);
        set_f *= 2;
    }

    return true;
}


bool pdp_tester::mem_test_set_PSW_flag() {
    WORD set_f = 1;
    for(int f = 0; f < PSW_FLAGS_NUMBER; ++f) {
        mem_test->set_PSW_flag(f);
        if(mem_test->w_read(PSW) != set_f)
            return false;

        mem_test->w_write(PSW, 0);
        set_f *= 2;
    }

    return true;
}


bool pdp_tester::mem_test_clr_PSW_flag() {
    WORD set_f = 1;
     for(int f = 0; f < PSW_FLAGS_NUMBER; ++f) {
        mem_test->w_write(PSW, set_f);
        mem_test->clr_PSW_flag(f);

        if(mem_test->w_read(PSW) != 0)
            return false;

        set_f *= 2;
    }

    return true;
}


bool pdp_tester::mem_test_get_PSW() {
    for(int i = 0; i < MEM_TEST_ITER; ++i) {
        WORD val = rand() % WORD_SUP;
        mem_test->w_write(PSW, val);

        if(mem_test->get_PSW() != val)
            return false;
    }

    return true;
}


/*
 *
 *  Processor tests
 *
 */

bool pdp_tester::processor_test() {
    cout << "########################################" << endl;
    cout << "#          PROCESSOR TESTS             #" << endl;
    cout << "########################################" << endl;


    if(proc_test_get_reg_op())
        cout << "PROCESSOR TEST: get_reg_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_reg_op FAILED" << endl;

    if(proc_test_get_reg_def_op())
        cout << "PROCESSOR_TEST: get_reg_def_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_reg_def_op FAILED" << endl;

    if(proc_test_get_autoinc_op())
        cout << "PROCESSOR_TEST: get_autoinc_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_autoinc_op FAILED" << endl;

    if(proc_test_get_autoinc_def_op())
        cout << "PROCESSOR_TEST: get_autoinc_def_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_autoinc_def_op FAILED" << endl;

    if(proc_test_get_autodec_op())
        cout << "PROCESSOR_TEST: get_autodec_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_autodec_op FAILED" << endl;

    if(proc_test_get_autodec_def_op())
        cout << "PROCESSOR_TEST: get_autodec_def_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_autodec_def_op FAILED" << endl;

    if(proc_test_get_index_op())
        cout << "PROCESSOR_TEST: get_index_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_index_op FAILED" << endl;

    if(proc_test_get_index_def_op())
        cout << "PROCESSOR_TEST: get_index_def_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_index_def_op FAILED" << endl;

    if(proc_test_get_branch_op())
        cout << "PROCESSOR_TEST: get_branch_op OK" << endl;
    else
        cout << "PROCESSOR TEST: get_branch_op FAILED" << endl;

    if(proc_test_instruction_fetch())
        cout << "PROCESSOR TEST: instruction_fetch OK" << endl;
    else
        cout << "PROCESSOR TEST: instruction_fetch FAILED" << endl;

    if(proc_test_write_back())
        cout << "PROCESSOR TEST: write_back OK" << endl;
    else
        cout << "PROCESSOR TEST: write back FAILED" << endl;

    if(proc_test_ex_clr())
        cout << "PROCESSOR TEST: ex_clr OK" << endl;
    else
        cout << "PROCESSOR TEST: ex_clr FAILED" << endl;

    //proc_test_process_instruction();
    proc_test_disasm_curr_instr();

    cout << "########################################" << endl;

    return true;
}


bool pdp_tester::proc_test_instruction_fetch() {
    for(int i = 0; i < PROC_TEST_ITER; ++i) {
        WORD instr = rand() % WORD_SUP;
        mem_test->w_write(mem_test->get_reg_data(PC), instr);
        proc_test->instruction_fetch();

        if(proc_test->current_instr != instr)
            return false;
    }

    return true;
}


bool pdp_tester::proc_test_write_back() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < PROC_TEST_ITER; ++j) {
            proc_test->result = static_cast<op_type>(i);
            WORD adr = rand() % WORD_SUP;
            WORD val = rand() % WORD_SUP;
            proc_test->operands[static_cast<op_type>(i)].adr = adr;
            proc_test->operands[static_cast<op_type>(i)].val = val;

            proc_test->write_back();

            if(mem_test->w_read(adr) != val)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_reg_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < SP; ++j) {
            WORD val = rand() % WORD_SUP;
            mem_test->set_reg_data(j, val);
            proc_test->get_reg_op(j, i);

            if(proc_test->operands[i].adr != mem_test->get_reg_addr(j) ||
               proc_test->operands[i].val != val)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_reg_def_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < PC; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr = rand() % WORD_SUP;
            mem_test->set_reg_data(j, adr);
            mem_test->w_write(adr, val);
            proc_test->get_reg_def_op(j, i);

            if(proc_test->operands[i].adr != adr ||
               proc_test->operands[i].val != val ||
               (j == PC && mem_test->get_reg_data(PC) != adr + 2))
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_autoinc_op() {
    proc_test->if_byte_flag = 0;
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < SP; ++j) {
            for(int ibf = 0; ibf < 2; ++ibf) {
                proc_test->if_byte_flag = ibf;

                WORD val = rand() % WORD_SUP;
                ADDR adr = rand() % WORD_SUP;
                mem_test->set_reg_data(j, adr);
                mem_test->w_write(adr, val);
                proc_test->get_autoinc_op(j, i);

                if(proc_test->operands[i].adr != adr ||
                   proc_test->operands[i].val != val ||
                   mem_test->get_reg_data(j) != adr + 2 - ibf)
                    return false;
            }
        }

        proc_test->if_byte_flag = 0;
        for(int j = SP; j < REGISTERS_NUMBER; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr = rand() % WORD_SUP;
            mem_test->set_reg_data(j, adr);
            mem_test->w_write(adr, val);            
            proc_test->get_autoinc_op(j, i);            

            if(proc_test->operands[i].adr != adr ||
               proc_test->operands[i].val != val ||
               mem_test->get_reg_data(j) != adr + 2)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_autoinc_def_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < REGISTERS_NUMBER; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr_of_val = rand() % WORD_SUP;
            ADDR adr_of_adr = rand() % WORD_SUP;
            mem_test->set_reg_data(j, adr_of_adr);
            mem_test->w_write(adr_of_adr, adr_of_val);
            mem_test->w_write(adr_of_val, val);
            proc_test->get_autoinc_def_op(j, i);

            if(proc_test->operands[i].adr != adr_of_val ||
               proc_test->operands[i].val != val ||
               mem_test->get_reg_data(j) != adr_of_adr + 2)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_autodec_op() {
    proc_test->if_byte_flag = 0;
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < SP; ++j) {
            for(int ibf = 0; ibf < 2; ++ibf) {
                proc_test->if_byte_flag = ibf;

                WORD val = rand() % WORD_SUP;
                ADDR adr = rand() % WORD_SUP;
                mem_test->set_reg_data(j, adr);
                mem_test->w_write(adr - 2 + ibf, val);
                proc_test->get_autodec_op(j, i);

                if(proc_test->operands[i].adr != adr - 2 + ibf ||
                   proc_test->operands[i].val != val ||
                   mem_test->get_reg_data(j) != adr - 2 + ibf)
                    return false;
            }
        }

        proc_test->if_byte_flag = 0;
        for(int j = SP; j < PC; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr = rand() % WORD_SUP;
            mem_test->set_reg_data(j, adr);
            mem_test->w_write(adr - 2, val);
            proc_test->get_autodec_op(j, i);

            if(proc_test->operands[i].adr != adr - 2 || proc_test->operands[i].val != val ||
               mem_test->get_reg_data(j) != adr - 2)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_autodec_def_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < SP; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr_of_val = rand() % WORD_SUP;
            ADDR adr_of_adr = rand() % WORD_SUP;
            mem_test->set_reg_data(j, adr_of_adr + 2);
            mem_test->w_write(adr_of_adr, adr_of_val);
            mem_test->w_write(adr_of_val, val);
            proc_test->get_autodec_def_op(j, i);

            if(proc_test->operands[i].adr != adr_of_val || proc_test->operands[i].val != val
               || mem_test->get_reg_data(j) != adr_of_adr)
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_index_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < REGISTERS_NUMBER; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr = rand() % WORD_SUP;
            WORD bias = rand() % (WORD_SUP - adr);
            ADDR bias_adr = rand() % WORD_SUP;
            mem_test->set_reg_data(PC, bias_adr);
            mem_test->w_write(bias_adr, bias);

            if(j == PC)
                adr = bias_adr + 2;
            else
                mem_test->set_reg_data(j, adr);

            mem_test->w_write(adr + bias, val);
            proc_test->get_index_op(j, i);

            if(proc_test->operands[i].adr != adr + bias || proc_test->operands[i].val != val ||
               (j == PC && mem_test->get_reg_data(PC) != adr))
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_index_def_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int j = 0; j < REGISTERS_NUMBER; ++j) {
            WORD val = rand() % WORD_SUP;
            ADDR adr_of_val = rand() % WORD_SUP;
            ADDR adr_of_adr = rand() % WORD_SUP;
            WORD bias = rand() % (WORD_SUP - adr_of_adr);
            ADDR bias_adr = rand() % WORD_SUP;
            mem_test->set_reg_data(PC, bias_adr);
            mem_test->w_write(bias_adr, bias);

            if(j == PC)
                adr_of_adr = bias_adr + 2;
            else
                mem_test->set_reg_data(j, adr_of_adr);

            mem_test->w_write(adr_of_adr + bias, adr_of_val);
            mem_test->w_write(adr_of_val, val);
            proc_test->get_index_def_op(j, i);

            if(proc_test->operands[i].adr != adr_of_val || proc_test->operands[i].val != val ||
               (j == PC && mem_test->get_reg_data(PC) != adr_of_adr))
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_get_branch_op() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        for(int i = 0; i < PROC_TEST_ITER; ++i) {
            WORD instr = rand() % WORD_SUP;
            proc_test->get_branch_op(instr, i);

            if(proc_test->operands[i].val != (instr & OFFSET_MASK))
                return false;
        }
    }

    return true;
}


bool pdp_tester::proc_test_disasm_curr_instr() {
    WORD instr = 0010000;
    WORD reg1 = 1;
    WORD reg2 = 2;
    WORD mod1 = 5;
    WORD mod2 = 3;
    instr += (mod1 << 9) + (reg1 << 6) + (mod2 << 3) + reg2;
    proc_test->current_instr = instr;
    string str = proc_test->disasm_curr_instr();
    cout << str << endl;
}


/*
 *  Instructions tests
 */

bool pdp_tester::proc_test_ex_clr() {
    for(int i = 0; i < BRANCH; ++i) {
        for(int j = 0; j < SP; j++) {
            WORD instr = 0005000;
            instr += i * 010 + j;
            WORD val = rand() % WORD_SUP;
            proc_test->operands[DD].val = val;

            if(proc_test->operands[DD].val != 0 &&
               (mem_test->get_PSW() & 017) != 004)
                return false;
        }
    }

    return true;
}
