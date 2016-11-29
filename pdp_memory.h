#ifndef PDP_MEMORY_H
#define PDP_MEMORY_H

#include "data_types.h"
#include "pdp_tester.h"


#define REG_SIZE                16
#define USER_MEM_SIZE           56*KB // TODO: change it
#define RAM_SIZE                64*KB
#define VRAM_SIZE               16*KB

#define PSW                     0177776

#define FIRST_BIT_MASK          0000001

typedef enum registers {
    R0                  = 0,
    R1                  = 1,
    R2                  = 2,
    R3                  = 3,
    R4                  = 4,
    R5                  = 5,
    SP                  = 6,
    PC                  = 7,
    REGISTERS_NUMBER    = 8
} registers;

typedef enum PSW_flags {
    C   = 0,
    V   = 1,
    Z   = 2,
    N   = 3,
    T   = 4,
    PSW_FLAGS_NUMBER = 5
} PSW_flags;



class pdp_memory
{
    friend class pdp_tester;

private:
    BYTE    RAM[RAM_SIZE + REG_SIZE] = {};
    BYTE    VRAM[VRAM_SIZE] = {};

public:
    pdp_memory();

    //  Register access funcs
    ADDR    get_reg_addr(int number);
    WORD    get_reg_data(int number);
    bool    set_reg_data(int number, WORD data);

    //  Memory access funcs
    bool    b_write(ADDR adr, BYTE value);
    BYTE    b_read(ADDR adr);
    bool    w_write(ADDR adr, WORD value);
    WORD    w_read(ADDR adr);

    //  Processor status word flags
    int     get_PSW_flag(int flag);
    bool    set_PSW_flag(int flag);
    bool    clr_PSW_flag(int flag);

    //  Reset funcs
    bool    reset();
    bool    reset_VRAM();
    bool    reset_RAM();
    bool    reset_registers();

    BYTE *  get_RAM_snapshot();
    BYTE *  get_VRAM_snapshot();
    WORD *  get_registers_snapshot();

    WORD    get_PSW();
};

#endif // MEMORY_H
