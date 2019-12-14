#ifndef _NAND_H_
#define _NAND_H_

#include <stdint.h>
#include <stdbool.h>


//
bool nand_dump_start(const char *name, uint8_t partition_id, int64_t free_space);

#endif