#include <iostream>
#include <bitset>
#include "pdp_memory.h"
#include "pdp.h"
#include "pdp_tester.h"

using::std::cout;
using::std::endl;



int main()
{
//    pdp_memory *memory = new pdp_memory();
//    test_PSW(memory);
//    pdp *machine = new pdp();
//    test_pdp(machine);
//    pdp_tester *tester = new pdp_tester();
//    tester->memory_test();
//    tester->processor_test();
    pdp *machine = new pdp();
    machine->load_program("first_program.pdp");
    machine->print_status();
    machine->print_next_instruction();
    machine->print_next_instruction();
    machine->print_next_instruction();

    return 0;
}

