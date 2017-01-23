#include "pdp_processor.h"

#include <cstdlib>
#include <cstdio>
#include "utils.h"
#include "pdp_memory.h"
#include "pipeline.h"
#include "clocks.h"
#include "wb_buffer.h"


#define CLEAR_MASK                  0000000
#define CLEARB_MASK                 0177400
#define FIRST_BYTE_MASK             0000377
#define SECOND_BYTE_MASK            0177400
#define SIGN_BIT_MASK               0100000


pdp_processor::pdp_processor(pdp_memory *mem, pipeline *p)
{
    memory  = mem;
    pipe    = p;
    cache   = new icache(mem);
    wb_buf  = new wb_buffer(mem);
    instruction_counter = -1;   //  For increment in first use and has value 0
    clock_counter = 0;

    if(!parse_commands())
        error_exit("Processor initialization failure: can't parse commands");
}


string pdp_processor::disasm_curr_instr() {
    com_processing curr_command = parsed_commands[current_instr];
    string disasm = commands_list[curr_command.index].name;
    disasm += " ";
    if(curr_command.second_op_addr_mode != NO_MODE) {
        disasm += mode_list[curr_command.second_op_addr_mode].left;
        if(curr_command.second_op_addr_mode == INDEX ||
           curr_command.second_op_addr_mode == INDEX_DEFERRED)
            disasm += std::to_string(operands[NN].val);

        disasm += mode_list[curr_command.second_op_addr_mode].mid;
        disasm += "R";
        disasm += std::to_string((current_instr >> 6) & 07);
        disasm += mode_list[curr_command.second_op_addr_mode].right;
        disasm += ", ";
    }

    if(curr_command.first_op_addr_mode != NO_MODE &&
       curr_command.first_op_addr_mode != BRANCH) {
        disasm += mode_list[curr_command.first_op_addr_mode].left;
        if(curr_command.first_op_addr_mode == INDEX ||
           curr_command.first_op_addr_mode == INDEX_DEFERRED)
            disasm += std::to_string(operands[NN].val);

        disasm += mode_list[curr_command.first_op_addr_mode].mid;
        disasm += "R";
        disasm += std::to_string((current_instr & 07));
        disasm += mode_list[curr_command.first_op_addr_mode].right;
    } else if(curr_command.first_op_addr_mode == BRANCH) {
        disasm += std::to_string(operands[XX].val);
        //  TODO: check this out and improve
    }

    return disasm;
}


bool pdp_processor::process_instruction() {

    instruction_counter++;

    if(!instruction_fetch())
        return false;

    if(!instruction_decode())
        return false;

    if(!op1_fetch())
        return false;

    if(!op2_fetch())
        return false;

    if(!execute())
        return false;

    if(!write_back())
        return false;

    return true;
}


bool pdp_processor::instruction_fetch() {
    WORD instr_addr = memory->get_reg_data(PC);

    if(!memory->set_reg_data(PC, instr_addr + 2))
        error_exit("Instruction fetch failure: can't increment PC by 2");

    current_instr = cache->get_instr(instr_addr);

    int ifetch_clocks = 0;
    if(cache->is_missed())
        ifetch_clocks = MEMORY_ACCESS;
    else
        ifetch_clocks = CACHE_ACCESS;

    pipe->istage_push(IFETCH_STAGE, {instruction_counter,
                                     clock_counter,
                                     ifetch_clocks,
                                     {instr_addr}
                                    });
    clock_counter += ifetch_clocks;
    return true;
}

bool pdp_processor::instruction_decode() {
    if_byte_flag = current_instr >> 15;
    int idecode_clocks = REGISTER_ACCESS;
    pipe->istage_push(IDECODE_STAGE, {instruction_counter,
                                      clock_counter,
                                      idecode_clocks,
                                      {}
                                     });
    clock_counter += idecode_clocks;
    return true;
}


bool pdp_processor::op1_fetch() {
    op_type first_op_type = parsed_commands[current_instr].first_op_type;
    int op1fetch_clocks = 0;
    if(parsed_commands[current_instr].first_op_addr_mode != NO_MODE) {
        (this->*parsed_commands[current_instr].get_first_op)(current_instr, first_op_type);
        op1fetch_clocks = opfetch_clocks[parsed_commands[current_instr].first_op_addr_mode];
    }

    pipe->istage_push(OP1FETCH_STAGE, {instruction_counter,
                                       clock_counter,
                                       op1fetch_clocks,
                                       pipe->of_get_hp(parsed_commands[current_instr].first_op_addr_mode,
                                                       operands[first_op_type]),
                                      });
    clock_counter += op1fetch_clocks;
    return true;
}


bool pdp_processor::op2_fetch() {
    op_type second_op_type = parsed_commands[current_instr].second_op_type;
    int op2fetch_clocks = 0;
    if(parsed_commands[current_instr].second_op_addr_mode != NO_MODE) {
        (this->*parsed_commands[current_instr].get_second_op)(current_instr >> 6, second_op_type);
        op2fetch_clocks = opfetch_clocks[parsed_commands[current_instr].second_op_addr_mode];
    }

    pipe->istage_push(OP2FETCH_STAGE, {instruction_counter,
                                       clock_counter,
                                       op2fetch_clocks,
                                       pipe->of_get_hp(parsed_commands[current_instr].second_op_addr_mode,
                                                       operands[second_op_type]),
                                      });
    clock_counter += op2fetch_clocks;
    return true;
}

bool pdp_processor::execute() {
    overflow_free_res = 0;
    WORD idx = parsed_commands[current_instr].index;
    (this->*commands_list[idx].ex_func)();

    //  TODO:   add smth for clock count
    int execute_clocks = 1;
    if(current_instr == 0x1001)
        execute_clocks += 9;
    pipe->istage_push(EXECUTE_STAGE, {instruction_counter,
                                      clock_counter,
                                      execute_clocks,
                                      {}
                                     });
    clock_counter += execute_clocks;

    return true;
}

bool pdp_processor::write_back() {    
    //memory->w_write(operands[result].adr, operands[result].val);
    int wb_clocks = 0;

    //  Register is used in place therefore
    //  wb_clocks is 0
    if(operands[result].adr >= RAM_SIZE) {
        memory->w_write(operands[result].adr, operands[result].res);
        wb_clocks += REGISTER_ACCESS;

    //  Memory write back
    } else {
        wb_buf->push({operands[result].adr, operands[result].res});
        if(wb_buf->was_overflow())
            wb_clocks += MEMORY_ACCESS;
        else
            wb_clocks += CACHE_ACCESS;
    }

    pipe->istage_push(WRITEBACK_STAGE, {instruction_counter,
                                        clock_counter,
                                        wb_clocks,
                                        {operands[result].adr}
                                       });
    clock_counter += wb_clocks;
    return true;
}

bool pdp_processor::parse_commands() {
    if(!init_get_op_array())
        return false;

    for(int i = 0; i < PARSED_COMMANDS_SIZE; ++i) {
        int j = 0;
        while((i & commands_list[j].mask) != commands_list[j].opcode)
            j++;

        parsed_commands[i].index = j;
        addr_mode first_op_mode  = NO_MODE;
        addr_mode second_op_mode = NO_MODE;
        parsed_commands[i].first_op_type    = NO_OPERAND_TYPE;
        parsed_commands[i].second_op_type   = NO_OPERAND_TYPE;

        if(commands_list[j].mask == DOUBLE_OP_MASK) {
            first_op_mode    = static_cast<addr_mode>((i >> 3) & MODE_MASK);
            second_op_mode   = static_cast<addr_mode>((i >> 9) & MODE_MASK);

            parsed_commands[i].first_op_type    = DD;
            parsed_commands[i].second_op_type   = SS;

        } else if(commands_list[j].mask == DOUBLE_REG_OP_MASK) {//May be this is need to change
            first_op_mode    = REGISTER;
            second_op_mode   = static_cast<addr_mode>((i >> 3) & MODE_MASK);

            parsed_commands[i].first_op_type    = DD;
            parsed_commands[i].second_op_type   = R;

            if(commands_list[j].params & HAS_NN)
                parsed_commands[i].first_op_type = NN;

        } else if(commands_list[j].mask == SINGLE_OP_MASK) {
            first_op_mode    = static_cast<addr_mode>((i >> 3) & MODE_MASK);

            parsed_commands[i].first_op_type    = DD;

            if(commands_list[j].params & HAS_NN)
                parsed_commands[i].first_op_type = NN;

        } else if(commands_list[j].mask == BRANCH_MASK) {
            first_op_mode    = BRANCH;

            parsed_commands[i].first_op_type    = XX;
        }

        parsed_commands[i].first_op_addr_mode   = first_op_mode;
        parsed_commands[i].get_first_op         = get_op[first_op_mode];
        parsed_commands[i].second_op_addr_mode  = second_op_mode;
        parsed_commands[i].get_second_op        = get_op[second_op_mode];
    }

    return true;
}


bool pdp_processor::init_get_op_array() {
    get_op[REGISTER]                =   &pdp_processor::get_reg_op;
    get_op[REGISTER_DEFERRED]       =   &pdp_processor::get_reg_def_op;
    get_op[AUTOINCREMENT]           =   &pdp_processor::get_autoinc_op;
    get_op[AUTOINCREMENT_DEFERRED]  =   &pdp_processor::get_autoinc_def_op;
    get_op[AUTODECREMENT]           =   &pdp_processor::get_autodec_op;
    get_op[AUTODECREMENT_DEFERRED]  =   &pdp_processor::get_autodec_def_op;
    get_op[INDEX]                   =   &pdp_processor::get_index_op;
    get_op[INDEX_DEFERRED]          =   &pdp_processor::get_index_def_op;
    get_op[BRANCH]                  =   &pdp_processor::get_branch_op;

    return true;
}


void pdp_processor::get_reg_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    if(reg_number == PC)
        error_exit("reg mode cannot be used with PC");
    else if(reg_number == SP)
        error_exit("reg_mode cannot be used with SP");

    operands[op_num].adr = memory->get_reg_addr(reg_number);
    operands[op_num].val = memory->get_reg_data(reg_number);
}


void pdp_processor::get_reg_def_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    if(reg_number == PC)
        error_exit("reg_def mode cannot be used with PC");

    ADDR mem_addr  = memory->get_reg_data(reg_number);

    operands[op_num].adr = mem_addr;
    operands[op_num].val = memory->w_read(mem_addr);
}


void pdp_processor::get_autoinc_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    ADDR mem_addr  = memory->get_reg_data(reg_number);

    operands[op_num].adr  = mem_addr;
    operands[op_num].val  = memory->w_read(mem_addr);

    if((if_byte_flag) && (reg_number < 6))
        memory->set_reg_data(reg_number, mem_addr + 1);
    else
        memory->set_reg_data(reg_number, mem_addr + 2);
}


void pdp_processor::get_autoinc_def_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    ADDR mem_addr  = memory->get_reg_data(reg_number);

    operands[op_num].adr = memory->w_read(mem_addr);
    operands[op_num].val = memory->w_read(operands[op_num].adr);

    memory->set_reg_data(reg_number, mem_addr + 2);
}


void pdp_processor::get_autodec_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    if(reg_number == PC)
        error_exit("autodec mode cannot be used with PC");

    ADDR mem_addr = memory->get_reg_data(reg_number);

    if((if_byte_flag) && (reg_number < 6))
        mem_addr -= 1;
    else
        mem_addr -= 2;
    memory->set_reg_data(reg_number, mem_addr);

    operands[op_num].adr = mem_addr;
    operands[op_num].val = memory->w_read(mem_addr);
}


void pdp_processor::get_autodec_def_op(WORD instr, int op_num) {
    int reg_number = instr & REG_NUMBER_MASK;
    if(reg_number == PC)
        error_exit("autodec_def mode cannot be used with PC");
    else if(reg_number == SP)
        error_exit("autodec_def mode cannot be used with SP");

    ADDR mem_addr  = memory->get_reg_data(reg_number) - 2;

    memory->set_reg_data(reg_number, mem_addr);

    operands[op_num].adr  = memory->w_read(mem_addr);
    operands[op_num].val  = memory->w_read(operands[op_num].adr);
}


void pdp_processor::get_index_op(WORD instr, int op_num) {
    operands[NN].val = memory->w_read(memory->get_reg_data(PC));
    memory->set_reg_data(PC, memory->get_reg_data(PC) + 2);

    int reg_number = instr & REG_NUMBER_MASK;
    ADDR mem_addr = memory->get_reg_data(reg_number);
    mem_addr += operands[NN].val;

    operands[op_num].adr = mem_addr;
    operands[op_num].val = memory->w_read(mem_addr);
}


void pdp_processor::get_index_def_op(WORD instr, int op_num) {
    operands[NN].val = memory->w_read(memory->get_reg_data(PC));
    memory->set_reg_data(PC, memory->get_reg_data(PC) + 2);

    int reg_number = instr & REG_NUMBER_MASK;
    ADDR mem_addr = memory->get_reg_data(reg_number);
    mem_addr += operands[NN].val;

    operands[op_num].adr = memory->w_read(mem_addr);
    operands[op_num].val = memory->w_read(operands[op_num].adr);
}


void pdp_processor::get_branch_op(WORD instr, int op_num) {
    operands[op_num].val = instr & OFFSET_MASK;
    operands[op_num].adr = 0;
}


pdp_processor::command pdp_processor::get_command(int index) {
    return commands_list[index];
}


pdp_processor::com_processing pdp_processor::get_parsed_command(int index) {
    return parsed_commands[index];
}


bool pdp_processor::reset() {
    if(!reset_operands())
        return false;

    if(!reset_if_byte_flag())
        return false;

    if(!reset_result())
        return false;

    if(!reset_curr_instr())
        return false;

    return true;
}


bool pdp_processor::reset_operands() {
    for(int i = 0; i < OPERANDS_NUMBER; ++i) {
        operands[i].val = 0;
        operands[i].adr = 0;
        operands[i].res = 0;
    }

    return true;
}


bool pdp_processor::reset_if_byte_flag() {
    if_byte_flag = 0;
    return true;
}


bool pdp_processor::reset_result() {
    result = DD;
    return true;
}


bool pdp_processor::reset_curr_instr() {
    current_instr = 0;
    return true;
}


ic_stat_t pdp_processor::get_icstat() {
    return cache->get_stat();
}


/*
 *  Flag setting
 */
bool pdp_processor::set_flags(int c, int v, int z, int n) {
    //  Gets operand types
    op_type op1 = parsed_commands[current_instr].first_op_type;
    op_type op2 = parsed_commands[current_instr].second_op_type;
    op_type res = result;

    //  Z flag
    if(z == -1) {
        bool Z_is_set = (operands[res].res == 0) ? true : false;
        if(Z_is_set)
            memory->set_PSW_flag(Z);
        else
            memory->clr_PSW_flag(Z);

    } else if(z == 1)
        memory->set_PSW_flag(Z);
      else if(z == 0)
        memory->clr_PSW_flag(Z);

    //  N flag
    if(n == -1) {
        bool N_is_set = (operands[res].res & SIGN_BIT_MASK) ? true : false;
        if(N_is_set)
            memory->set_PSW_flag(N);
        else
            memory->clr_PSW_flag(N);

    } else if(n == 1)
        memory->set_PSW_flag(N);
      else if(n == 0)
        memory->clr_PSW_flag(N);

    //  C flag
    if(c == -1) {
        //  TODO: make something
    } else if(c == 1)
        memory->set_PSW_flag(C);
      else if(c == 0)
        memory->clr_PSW_flag(C);

    //  V flag
    if(v == -1) {
        //  Sign bit checking
        int ovfr_sb = overflow_free_res & SIGN_BIT_MASK;
        int  res_sb = operands[res].res & SIGN_BIT_MASK;
        if(ovfr_sb != res_sb)
            memory->set_PSW_flag(V);
        //  Bounds checking
        else if(overflow_free_res > 0177777)
            memory->set_PSW_flag(V);
        else
            memory->clr_PSW_flag(V);

    } else if(v == 1)
        memory->set_PSW_flag(V);
      else if(v == 0)
        memory->clr_PSW_flag(V);

finish:
    return true;
}


/*
 *
 *  PDP processor instructions
 *
 */


void pdp_processor::ex_clr() {
    operands[DD].res = operands[DD].val & CLEAR_MASK;
    result = DD;
    set_flags(0, 0, 1, 0);

//    memory->clr_PSW_flag(C);
//    memory->clr_PSW_flag(V);
//    memory->set_PSW_flag(Z);
//    memory->clr_PSW_flag(N);
}

void pdp_processor::ex_clrb() {
    operands[DD].res = operands[DD].val & CLEARB_MASK;
    result = DD;
    set_flags(0, 0, 1, 0);

//    memory->clr_PSW_flag(C);
//    memory->clr_PSW_flag(V);
//    memory->set_PSW_flag(Z);
//    memory->clr_PSW_flag(N);
}

void pdp_processor::ex_com() {
    operands[DD].res = ~operands[DD].val;
    result = DD;
    set_flags(1, 0);

//    memory->set_PSW_flag(C);
//    memory->clr_PSW_flag(V);
//    if(operands[DD].res == 0)
//        memory->set_PSW_flag(Z);
//    if(operands[DD].res >> 15 == 1)
//        memory->set_PSW_flag(N);
}

void pdp_processor::ex_comb() {
    WORD first_byte  = ~operands[DD].val & FIRST_BYTE_MASK; //  Change first byte
    WORD second_byte = operands[DD].val & SECOND_BYTE_MASK; //  Don't change second
    operands[DD].res = first_byte + second_byte;
    result = DD;

//    memory->set_PSW_flag(C);
//    memory->clr_PSW_flag(V);
//    if(operands[DD].res == 0)
//        memory->set_PSW_flag(Z);
//    if(operands[DD].res >> 15 == 1)
//        memory->set_PSW_flag(N);
    set_flags(1, 0);
}

void pdp_processor::ex_inc() {
    overflow_free_res = operands[DD].val + 1;
    operands[DD].res = overflow_free_res;
    result = DD;
    set_flags(-2);
//    if(operands[DD].val == 0077777)
//        memory->set_PSW_flag(V);

//    operands[DD].val++;
//    result = DD;

//    if(operands[DD].val >> 15 ==  1)
//        memory->set_PSW_flag(N);

//    if(operands[DD].val == 0)
//        memory->set_PSW_flag(Z);
}

void pdp_processor::ex_incb() {
//    WORD first_byte = operands[DD].val & FIRST_BYTE_MASK;
//    WORD second_byte = operands[DD].val & SECOND_BYTE_MASK;

//    if(first_byte == 0000177)   // May be it needs to compare with 0377
//        memory->set_PSW_flag(V);

//    first_byte++;
//    operands[DD].val = first_byte + second_byte;
//    result = DD;

//    if(operands[DD].val >> 7 == 1)
//        memory->set_PSW_flag(N);

//    if(operands[DD].val == 0)
//        memory->set_PSW_flag(Z);
}

void pdp_processor::ex_dec() {
//    if(operands[DD].val == 0100000)
//        memory->set_PSW_flag(V);

    overflow_free_res = operands[DD].val - 1;
    operands[DD].res = overflow_free_res;
    result = DD;
    set_flags(-2);

//    if(operands[DD].val >> 15 ==  1)
//        memory->set_PSW_flag(N);

//    if(operands[DD].val == 0)
//        memory->set_PSW_flag(Z);
}

void pdp_processor::ex_decb() {
//    WORD first_byte = operands[DD].val & FIRST_BYTE_MASK;
//    WORD second_byte = operands[DD].val & SECOND_BYTE_MASK;

//    if(first_byte == 0000177)   //  May be it needs to be compared with 377
//        memory->set_PSW_flag(V);

//    first_byte++;
//    operands[DD].val = first_byte + second_byte;
//    result = DD;

//    if(operands[DD].val >> 7 == 1)
//        memory->set_PSW_flag(N);

//    if(operands[DD].val == 0)
//        memory->set_PSW_flag(Z);
}

void pdp_processor::ex_neg() {
    operands[DD].res = 0 - operands[DD].val;
    result = DD;

    //  Carry flag
    int c_flag = 0;
    if(operands[DD].res != 0)
        c_flag = 1;

    set_flags(c_flag);

//    if(operands[DD].val < 0)
//        memory->set_PSW_flag(N);

//    if(operands[DD].val == 0) {
//        memory->set_PSW_flag(Z);
//        memory->clr_PSW_flag(C);
//    } else
//        memory->set_PSW_flag(C);

//    if(operands[DD].val == 0100000)
//        memory->set_PSW_flag(V);
//    else
//        memory->clr_PSW_flag(V);
}

void pdp_processor::ex_negb() {
    WORD first_byte = operands[DD].val & FIRST_BYTE_MASK;
    WORD second_byte = operands[DD].val & SECOND_BYTE_MASK;

    first_byte = 0 - first_byte;
    operands[DD].res = first_byte + second_byte;

    int c_flag = 0;
    if(first_byte != 0)
        c_flag = 1;

    set_flags(c_flag);
    result = DD;
//    if(first_byte < 0)
//        memory->set_PSW_flag(N);

//    if(first_byte == 0) {
//        memory->set_PSW_flag(Z);
//        memory->clr_PSW_flag(C);
//    } else
//        memory->set_PSW_flag(C);

//    if(first_byte == 0400)
//        memory->set_PSW_flag(V);
//    else
//        memory->clr_PSW_flag(V);
}

void pdp_processor::ex_tst() {

    return;
}

void pdp_processor::ex_tstb() {
    return;
}

void pdp_processor::ex_asr() {
    return;
}

void pdp_processor::ex_asrb() {
    return;
}

void pdp_processor::ex_asl() {
    return;
}

void pdp_processor::ex_aslb() {
    return;
}

void pdp_processor::ex_ror() {
    return;
}

void pdp_processor::ex_rorb() {
    return;
}

void pdp_processor::ex_rol() {
    return;
}

void pdp_processor::ex_rolb() {
    return;
}

void pdp_processor::ex_swap() {
    return;
}

void pdp_processor::ex_adc() {
    return;
}

void pdp_processor::ex_adcb() {
    return;
}

void pdp_processor::ex_sbc() {
    return;
}

void pdp_processor::ex_sbcb() {
    return;
}

void pdp_processor::ex_sxt() {
    return;
}

void pdp_processor::ex_mov() {
    operands[DD].res = operands[SS].val;
    result = DD;
    set_flags(-2, 0);

//    if(operands[SS].val < 0)
//        memory->set_PSW_flag(N);
//    else
//        memory->clr_PSW_flag(N);

//    if(operands[SS].val == 0)
//        memory->set_PSW_flag(Z);
//    else
//        memory->clr_PSW_flag(Z);

//    memory->clr_PSW_flag(V);
}

void pdp_processor::ex_movb() {     
    operands[DD].res = operands[SS].val & FIRST_BYTE_MASK;
    set_flags(-2, 0);
    result = DD;

//    if((operands[SS].val & FIRST_BYTE_MASK) < 0)
//        memory->set_PSW_flag(N);
//    else
//        memory->clr_PSW_flag(N);

//    if((operands[SS].val & FIRST_BYTE_MASK) == 0)
//        memory->set_PSW_flag(Z);
//    else
//        memory->clr_PSW_flag(Z);

//    memory->clr_PSW_flag(V);
}

void pdp_processor::ex_cmp() {
    overflow_free_res = operands[SS].val - operands[DD].val;
    operands[DD].res = overflow_free_res;
    result = DD;

    int c_flag = 0;
    if(overflow_free_res > 0177777)
        c_flag = 1;

    set_flags(c_flag);

//    if(result < 0)
//        memory->set_PSW_flag(N);
//    else
//        memory->clr_PSW_flag(N);

//    if(result == 0)
//        memory->set_PSW_flag(Z);
//    else
//        memory->clr_PSW_flag(Z);

//    //  TODO: do something better
//    if((operands[SS].val / 2 + operands[SS].val % 2) -
//       (operands[DD].val / 2 + operands[DD].val % 2) > 0177777)
//        memory->set_PSW_flag(V);
//    else
//        memory->clr_PSW_flag(V);

//    //  TODO: do C flag handling
}

void pdp_processor::ex_cmpb() {
    WORD result = (operands[SS].val & FIRST_BYTE_MASK) -
                  (operands[DD].val & FIRST_BYTE_MASK);

    return;
}

void pdp_processor::ex_add() {
    return;
}

void pdp_processor::ex_sub() {
    return;
}

void pdp_processor::ex_bit() {
    return;
}

void pdp_processor::ex_bitb() {
    return;
}

void pdp_processor::ex_bic() {
    WORD init_val_DD = operands[DD].val;
    operands[DD].val &= (~operands[SS].val);
    result = DD;

    if((init_val_DD >> 15 == 0) && (operands[DD].val >> 15 == 1))
        memory->set_PSW_flag(N);
    else
        memory->clr_PSW_flag(N);

    if(operands[DD].val == 0)
        memory->set_PSW_flag(Z);
    else
        memory->clr_PSW_flag(Z);

    memory->clr_PSW_flag(V);
}

void pdp_processor::ex_bicb() {
    WORD first_byte_SS = operands[SS].val & FIRST_BYTE_MASK;
    WORD first_byte_DD = operands[DD].val & FIRST_BYTE_MASK;
    WORD second_byte_DD = operands[DD].val & SECOND_BYTE_MASK;
    WORD init_first_byte_DD = first_byte_DD;

    first_byte_DD &= (~first_byte_SS);
    operands[DD].val = first_byte_DD + second_byte_DD;
    result = DD;

    if((init_first_byte_DD >> 7 == 0) && (first_byte_DD >> 7 == 1))
        memory->set_PSW_flag(N);
    else
        memory->clr_PSW_flag(N);

    if(first_byte_DD == 0)
        memory->set_PSW_flag(Z);
    else
        memory->clr_PSW_flag(Z);

    memory->clr_PSW_flag(V);
}

void pdp_processor::ex_bis() {
    return;
}

void pdp_processor::ex_bisb() {
    return;
}

void pdp_processor::ex_mul() {
    return;
}

void pdp_processor::ex_div() {
    return;
}

void pdp_processor::ex_ash() {
    return;
}

void pdp_processor::ex_ashc() {
    return;
}

void pdp_processor::ex_xor() {
    return;
}

void pdp_processor::ex_br() {
    ADDR prev_PC = memory->get_reg_data(PC);
    memory->set_reg_data(PC, prev_PC + 2 * operands[XX].val);
    return;
}

void pdp_processor::ex_bne() {
    if(memory->get_PSW_flag(Z) == 1)
        return;

    ADDR prev_PC = memory->get_reg_data(PC);
    int offset = operands[XX].val & ((1 << 7) - 1);
    if((operands[XX].val & (1 << 7)))
        offset *= -1;

    memory->set_reg_data(PC, prev_PC + 2 * offset);
    return;
}

void pdp_processor::ex_beq() {
    if(memory->get_PSW_flag(Z) == 0)
        return;

    ADDR prev_PC = memory->get_reg_data(PC);
    memory->set_reg_data(PC, prev_PC + 2 * operands[XX].val);
    return;
}

void pdp_processor::ex_bpl() {
    return;
}

void pdp_processor::ex_bmi() {
    return;
}

void pdp_processor::ex_bvc() {
    return;
}

void pdp_processor::ex_bvs() {
    return;
}

void pdp_processor::ex_bcc() {
    return;
}

void pdp_processor::ex_bcs() {
    return;
}

void pdp_processor::ex_bge() {
    return;
}

void pdp_processor::ex_blt() {
    return;
}

void pdp_processor::ex_bgt() {
    return;
}

void pdp_processor::ex_ble() {
    return;
}

void pdp_processor::ex_bhi() {
    return;
}

void pdp_processor::ex_blos() {
    return;
}

void pdp_processor::ex_bhis() {
    return;
}

void pdp_processor::ex_blo() {
    return;
}

void pdp_processor::ex_jmp() {
    return;
}

void pdp_processor::ex_jsr() {
    return;
}

void pdp_processor::ex_rts() {
    return;
}

void pdp_processor::ex_mark() {
    return;
}

void pdp_processor::ex_sob() {
    return;
}

void pdp_processor::ex_emt() {
    return;
}

void pdp_processor::ex_trap() {
    return;
}

void pdp_processor::ex_bpt() {
    return;
}

void pdp_processor::ex_iot() {
    return;
}

void pdp_processor::ex_rti() {
    return;
}

void pdp_processor::ex_rtt() {
    return;
}

void pdp_processor::ex_halt() {
    return;
}

void pdp_processor::ex_wait() {
    return;
}

void pdp_processor::ex_reset() {
    return;
}

void pdp_processor::ex_mfpi() {
    return;
}

void pdp_processor::ex_mtpi() {
    return;
}
