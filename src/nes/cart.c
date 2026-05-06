#include "emu6502.h"
#include "nes.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void	extract_title(t_cart *cart, const char *path)
{
	char *slash = strrchr(path, '/');
	char *dot = strrchr(path, '.');
	uint32_t start = slash ? slash - path + 1 : 0;
	uint32_t end = slash ? dot - path : strlen(path);
	uint32_t i = 0;

	while (start < end)
		cart->title[i++] = path[start++];
}

t_cart *read_nes_cart(const char *path)
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
	extract_title(cart, path);
	cart->prg_banks = hdr.prg_rom_size;
	cart->chr_banks = hdr.chr_rom_size;
	if (cart->chr_banks == 0)
		cart->has_chr_ram = true;
	printf("Flags6: 0x%02X\n", hdr.flags6);
	cart->mirroring = hdr.flags6 & 1;
	cart->mapper_id = ((hdr.flags6 >> 4) & 0x0F) | (hdr.flags7 & 0xF0);
	// if (cart->mapper_id != 0)
	// 	return (free(cart), NULL);

	if (hdr.flags6 & BIT_2)
		lseek(fd, 512, SEEK_CUR);

	cart->prg_rom = malloc(cart->prg_banks * 0x4000);
	read(fd, cart->prg_rom, cart->prg_banks * 0x4000);

	if (!cart->has_chr_ram)
	{
		cart->chr_rom = calloc(0x2000 * cart->chr_banks, 1);
		read(fd, cart->chr_rom, cart->chr_banks * 0x2000);
	}

	return cart;
}

void	free_cart(t_cart *cart)
{
	free(cart->prg_rom);
	free(cart->chr_rom);
	free(cart);
}

void	nes_load_cartridge(t_nes *nes, t_cart *cart)
{
	nes->cart = cart;
	nes->ppu.mirroring = cart->mirroring;
	dprintf(nes->cpu.logfd, "mapper: %d prg: %04zX chr: %04zX mirroring: %d is_ram: %d\n",
		 cart->mapper_id, cart->prg_banks, cart->chr_banks, cart->mirroring, cart->has_chr_ram);
	apply_nes_mmap(nes);
	apply_mapper_mmap(nes, cart);
}
