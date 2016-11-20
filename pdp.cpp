#include "pdp.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>


using std::cout;
using std::endl;
using std::ifstream;
using std::hex;


#define error_exit(msg) do {                    \
                            perror(msg);        \
                            exit(EXIT_FAILURE); \
                        } while (0)

pdp::pdp() {
    memory      = new pdp_memory();
    processor   = new pdp_processor(memory);
    memory->set_reg_data(PC, 0);
}


bool pdp::print_next_instruction() {
    run_next_instruction();

    //while(curr_stat) {
        print_status();

        //curr_stat = run_next_instruction();
    //}


}


bool pdp::print_status() {
    pdp_status *curr_stat = get_pdp_status();

    cout << "##############################" << endl;
    cout << "# " << curr_stat->disasm_command << endl;
    cout << "##############################" << endl;

    cout << "R0 : " << curr_stat->registers[R0] << endl;
    cout << "R1 : " << curr_stat->registers[R1] << endl;
    cout << "R2 : " << curr_stat->registers[R2] << endl;
    cout << "R3 : " << curr_stat->registers[R3] << endl;
    cout << "R4 : " << curr_stat->registers[R4] << endl;
    cout << "R5 : " << curr_stat->registers[R5] << endl;
    cout << "SP : " << curr_stat->registers[SP] << endl;
    cout << "PC : " << curr_stat->registers[PC] << endl;
    cout << "MEMORY : " << endl;
    const int MEM_DUMP_SIZE = 16;
    for(int i = 0; i < MEM_DUMP_SIZE; i += 2) {
        printf("%2d: %02x %02x\n", i, curr_stat->RAM[i], curr_stat->RAM[i+1]);
    }

    WORD psw = curr_stat->RAM[RAM_SIZE-1];

    cout << "NZVC : " << ((psw >> 3) & 1) << ((psw >> 2) & 1)
         << ((psw >> 1) & 1) << (psw & 1) << endl;
}


pdp_status *pdp::run_next_instruction() {
    if(!processor->process_instruction())
        error_exit("Can't process the instruction");

    return get_pdp_status();
}


pdp_status *pdp::get_pdp_status() {
    pdp_status *status = (pdp_status *)calloc(1, sizeof(pdp_status));
    if(!status)
        error_exit("Can't allocate memory for pdp status");

    status->RAM = (memory->get_RAM_snapshot());
    status->registers = (memory->get_registers_snapshot());
    status->VRAM = (memory->get_VRAM_snapshot());
    status->disasm_command = processor->disasm_curr_instr();

    return status;
}


int pdp::run_program() {



    return 0;
}


bool pdp::load_program(char *filename) {
    ifstream infile(filename);

    WORD init_addr = 0, n = 0;
    while(infile >> hex >> init_addr >> n) {
        for (int i = 0; i < n; ++i) {
            WORD x = 0;
            infile >> x;
            memory->w_write(init_addr + 2 * i, x);
        }
    }

    return true;
}
