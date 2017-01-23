#ifndef DATA_TYPES_H
#define DATA_TYPES_H


#define KB          1024
#define BYTE_SUP    256
#define WORD_SUP    65536


typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    ADDR;   //  Int for register addressing


typedef struct pipeline_statistics {
    int pipeline_clocks;
    int without_pipeline_clocks;
} pstat_t;

#endif // DATA_TYPES_H

