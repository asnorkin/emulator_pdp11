#ifndef PDP_H
#define PDP_H

#include "pdp_processor.h"
#include "pdp_memory.h"
#include "data_types.h"


typedef struct pdp_status {
    WORD *  registers;
    BYTE *  RAM;
    BYTE *  VRAM;
    string  disasm_command;
} pdp_status;


class pdp
{
    friend class pdp_tester;
    friend class pdp_memory;

private:
    pdp_processor   *processor;
    pdp_memory      *memory;


    pdp_status *get_pdp_status();

public:

    pdp();
    bool        load_program(char *filename);    
    pdp_status *run_next_instruction();
    bool        reset();
    //int         run_program();

    //  Just for console debugging
    bool        print_next_instruction();
    bool        print_status();

};

/* Questions to Artem:
 *  1 How does function definecom() work?
 *  2 Detail algorithm of command parsing
 */

#endif // PDP_H
