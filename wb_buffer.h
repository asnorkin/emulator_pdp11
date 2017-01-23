#ifndef WB_BUFFER_H
#define WB_BUFFER_H


#include "data_types.h"


#define     WB_BUFFER_SIZE  1


typedef struct wb_item {
    WORD    val;
    ADDR    adr;
} wb_item_t;


class pdp_memory;


class wb_buffer
{
private:
    wb_item_t   buffer[WB_BUFFER_SIZE] = {};

    pdp_memory  *memory;

    int     length;

    bool    overflow_flag;

    bool    flush();

public:
    wb_buffer(pdp_memory *mem);
    bool    push(wb_item_t item);
    bool    was_overflow();
};

#endif // WB_BUFFER_H
