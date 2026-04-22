#ifndef NES_H
# define NES_H

#include "emu6502.h"
#include <stdint.h>

struct nes_header {
	uint8_t name[4];
	uint8_t prg_rom_size;
	uint8_t chr_rom_size;
	uint8_t flags6;
	uint8_t flags7;
	uint8_t prg_ram_size;
	uint8_t flags9;
	uint8_t flags10;
	uint8_t padding[5];
};

typedef struct s_cart
{
	uint8_t	*prg_rom;
	uint8_t	*chr_rom;
	size_t	prg_size;
	size_t	chr_size;
	uint8_t	mapper_id;
	uint8_t	mirroring;
} t_cart;

typedef struct s_ppu
{
	uint8_t	registers[8];
} t_ppu;

typedef struct s_nes
{
	t_cpu	cpu;
	t_ppu	ppu;
	t_cart	*cart;
	uint8_t	memory[0x800];
}	t_nes;

t_cart	*read_nes(const char *path);
void	free_cart(t_cart *cart);
void	nes_load_cartridge(t_nes *nes, t_cart *cart);

#endif
