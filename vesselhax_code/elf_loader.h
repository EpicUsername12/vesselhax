#ifndef ELF_LDR_H
#define ELF_LDR_H

#include <string.h>
#include "elf_abi.h"
#include "imports.h"

uint32_t get_section(uint8_t *data, char *name, uint32_t *size, uint32_t *addr);
uint8_t *install_section(uint8_t *data, char *name);

#endif