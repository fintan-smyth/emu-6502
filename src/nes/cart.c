#include "emu6502.h"
#include "nes.h"
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

t_cart *read_nes(const char *path)
{
	int fd = open(path, O_RDONLY);

	if (fd <= 0)
		return NULL;

	struct nes_header hdr;

	int ret = read(fd, &hdr, 16);
	if (ret < 16)
		return NULL;

	uint8_t magic[4] = { 'N', 'E', 'S', 0x1a};
	if (memcmp(magic, hdr.name, 4) != 0)
		return NULL;

	t_cart *cart = calloc(1, sizeof(*cart));
	cart->prg_size = hdr.prg_rom_size * 0x4000;
	cart->chr_size = hdr.chr_rom_size * 0x2000;
	cart->mirroring = hdr.flags6 & 1;
	cart->mapper_id = ((hdr.flags6 >> 4) & 0x0F) | (hdr.flags7 & 0xF0);
	if (cart->mapper_id != 0)
		return (free(cart), NULL);

	if (hdr.flags6 & BIT_2)
		lseek(fd, 512, SEEK_CUR);

	cart->prg_rom = malloc(cart->prg_size);
	cart->chr_rom = malloc(cart->chr_size);
	read(fd, cart->prg_rom, cart->prg_size);
	read(fd, cart->chr_rom, cart->chr_size);

	return cart;
}

void	free_cart(t_cart *cart)
{
	free(cart->prg_rom);
	free(cart->chr_rom);
	free(cart);
}

void	setup_nes_mappings(t_nes *nes)
{
	t_cpu *cpu = &nes->cpu;

	map_memory(cpu->pagetable, 0, 2, nes->memory, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x800, 2, nes->memory, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1000, 2, nes->memory, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1800, 2, nes->memory, NULL, passthrough_write);

	//TODO: Add PPU and other memory mapped I/O
}

void	setup_mapper_pagetables(t_nes *nes, t_cart *cart)
{
	setup_nes_mappings(nes);
	switch (cart->mapper_id) {
		case (0):
			map_memory(nes->cpu.pagetable, 0x8000, 16, cart->prg_rom, NULL, NULL);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[cart->prg_size == 0x4000 ? 0 : 0x4000], NULL, NULL);
		default:
			break;
	}
}

void	nes_load_cartridge(t_nes *nes, t_cart *cart)
{
	nes->cart = cart;
	setup_mapper_pagetables(nes, cart);
}
