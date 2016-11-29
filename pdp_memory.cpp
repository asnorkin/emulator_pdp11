#include "pdp_memory.h"

#include <cstdio>
#include <cstdlib>

pdp_memory::pdp_memory()
{

}


bool pdp_memory::reset_VRAM() {
    for(int i = 0; i < VRAM_SIZE; ++i)
        VRAM[i] = 0;

    return true;
}


bool pdp_memory::reset_RAM() {
    for(int i = 0; i < RAM_SIZE; ++i)
        RAM[i] = 0;

    return true;
}


bool pdp_memory::reset_registers() {
    for(int i = 0; i < REGISTERS_NUMBER * 2; ++i)
        RAM[i + RAM_SIZE] = 0;
}


bool pdp_memory::reset() {
     if(!reset_VRAM())
         return false;

     if(!reset_RAM())
         return false;

     if(!reset_registers())
         return false;

     return true;
}


BYTE *pdp_memory::get_RAM_snapshot() {
    BYTE *snapshot = (BYTE *)calloc(RAM_SIZE, sizeof(BYTE));
    if(!snapshot)
        return NULL;

    for(int i = 0; i < RAM_SIZE; ++i)
        snapshot[i] = RAM[i];

    return snapshot;
}


WORD *pdp_memory::get_registers_snapshot() {
    WORD *snapshot = (WORD *)calloc(REGISTERS_NUMBER, sizeof(WORD));
    if(!snapshot)
        return NULL;

    for(int i = 0; i < REGISTERS_NUMBER; ++i)
        snapshot[i] = get_reg_data(i);

    return snapshot;
}


BYTE *pdp_memory::get_VRAM_snapshot() {
    BYTE *snapshot = (BYTE *)calloc(VRAM_SIZE, sizeof(BYTE));
    if(!snapshot)
        return NULL;

    for(int i = 0; i < VRAM_SIZE; ++i)
        snapshot[i] = VRAM[i];

    return snapshot;
}


/************************************************
 *
 *          Register access functions
 *
 ************************************************
 */

ADDR pdp_memory::get_reg_addr(int number) {
    if(number >= REGISTERS_NUMBER)
        return 0;

    return RAM_SIZE + number * 2;
}

WORD pdp_memory::get_reg_data(int number) {
    if(number >= REGISTERS_NUMBER)
        return 0;

    return w_read(RAM_SIZE + number * 2);
}

bool pdp_memory::set_reg_data(int number, WORD data) {
    if(number >= REGISTERS_NUMBER)
        return false;

    w_write(RAM_SIZE + number * 2, data);
    return true;
}

/************************************************
 *
 *          Ram read and write functions
 *
 ************************************************
 */


bool pdp_memory::b_write(ADDR adr, BYTE value) {
    RAM[adr] = value;
    return true;
}

BYTE pdp_memory::b_read(ADDR adr) {
    return RAM[adr];
}

bool pdp_memory::w_write(ADDR adr, WORD value) {
    BYTE first_byte  = (BYTE)value,
         second_byte = (BYTE)(value >> 8);    

    RAM[adr] = first_byte;
    RAM[adr + 1] = second_byte;
    return true;
}

WORD pdp_memory::w_read(ADDR adr) {    
    WORD first_byte  = RAM[adr],
         second_byte = RAM[adr + 1];

    return (second_byte << 8) + first_byte;
}

/************************************************
 *
 *      Program status word access functions
 *
 ************************************************
 */

WORD pdp_memory::get_PSW() {
    return w_read(PSW);
}

int pdp_memory::get_PSW_flag(int flag) {
    if(flag >= PSW_FLAGS_NUMBER)
        return 0;

    return ((w_read(PSW) >> flag) & FIRST_BIT_MASK);
}

bool pdp_memory::set_PSW_flag(int flag) {
    if(flag >= PSW_FLAGS_NUMBER)
        return false;

    WORD pos_number = 1;
    for(int i = 0; i < flag; ++i)
        pos_number *= 2;

    WORD res_PSW = w_read(PSW) | pos_number;
    w_write(PSW, res_PSW);
    return true;
}

bool pdp_memory::clr_PSW_flag(int flag) {
    if(flag >= PSW_FLAGS_NUMBER)
        return false;

    WORD pos_number = 1;
    for(int i = 0; i < flag; ++i)
        pos_number *= 2;

    WORD res_PSW = w_read(PSW) & (0177777 - pos_number);
    w_write(PSW, res_PSW);
    return true;
}
