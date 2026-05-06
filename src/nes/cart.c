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

void	parse_nes_1_header(t_cart *cart, int fd, struct nes_1_header *hdr)
{
	cart->header_type = 1;
	cart->prg_rom_banks = hdr->prg_rom_size;
	cart->chr_rom_banks = hdr->chr_rom_size;
	cart->mirroring = hdr->flags6 & 1;
	cart->mapper_id = ((hdr->flags6 >> 4) & 0x0F) | (hdr->flags7 & 0xF0);

	if (hdr->flags6 & BIT_2)
		lseek(fd, 512, SEEK_CUR);

	cart->prg_rom = malloc(cart->prg_rom_banks * 0x4000);
	read(fd, cart->prg_rom, cart->prg_rom_banks * 0x4000);

	if (cart->chr_rom_banks > 0)
	{
		cart->chr_rom = calloc(0x2000 * cart->chr_rom_banks, 1);
		read(fd, cart->chr_rom, cart->chr_rom_banks * 0x2000);
	}
	else
	{
		cart->chr_ram_banks = 2;
		cart->chr_ram = calloc(cart->chr_ram_banks, 0x1000);
	}
}

void	parse_nes_2_header(t_cart *cart, int fd, struct nes_2_header *hdr)
{
	cart->header_type = 2;
	cart->prg_rom_banks = hdr->prg_rom_size | ((hdr->rom_msb & 0x0F) << 8);
	cart->chr_rom_banks = hdr->chr_rom_size | ((hdr->rom_msb & 0xF0) << 4);;

	uint8_t prg_ram_shift = ((hdr->prg_ram_size & 0x0F) ? hdr->prg_ram_size : (hdr->prg_ram_size >> 4)) & 0x0F;
	uint8_t chr_ram_shift = ((hdr->chr_ram_size & 0x0F) ? hdr->chr_ram_size : (hdr->chr_ram_size >> 4)) & 0x0F;
	cart->prg_ram_banks = (64ul << prg_ram_shift) / 0x2000;
	cart->chr_ram_banks = (64ul << chr_ram_shift) / 0x1000;

	cart->mapper_id = ((hdr->flags6 >> 4) & 0x0F) | (hdr->flags7 & 0xF0) | ((hdr->mapper_extra & 0x0F) << 8);
	cart->submapper = (hdr->mapper_extra >> 4) & 0x0F;
	cart->mirroring = hdr->flags6 & 1;

	if (hdr->flags6 & BIT_2)
		lseek(fd, 512, SEEK_CUR);

	cart->prg_rom = malloc(cart->prg_rom_banks * 0x4000);
	read(fd, cart->prg_rom, cart->prg_rom_banks * 0x4000);

	if (cart->prg_ram_banks > 0)
		cart->prg_ram = calloc(cart->prg_ram_banks, 0x2000);
	if (cart->chr_rom_banks > 0)
	{
		cart->chr_rom = calloc(cart->chr_rom_banks, 0x2000);
		read(fd, cart->chr_rom, cart->chr_rom_banks * 0x2000);
	}
	if (cart->chr_ram_banks > 0)
		cart->chr_ram = calloc(cart->chr_ram_banks, 0x1000);
}

t_cart *read_nes_cart(const char *path)
{
	int fd = open(path, O_RDONLY);

	if (fd <= 0)
		return NULL;

	uint8_t buf[16];
	struct nes_1_header hdr;

	int ret = read(fd, buf, 16);
	if (ret < 16)
		return NULL;

	uint8_t magic[4] = { 'N', 'E', 'S', 0x1a};
	if (memcmp(magic, buf, 4) != 0)
		return NULL;

	t_cart *cart = calloc(1, sizeof(*cart));
	extract_title(cart, path);

	if ((buf[7] & 0x0C) == 0x08)
		parse_nes_2_header(cart, fd, (struct nes_2_header *)buf);
	else
		parse_nes_1_header(cart, fd, (struct nes_1_header *)buf);

	close(fd);

	return cart;
}

void	free_cart(t_cart *cart)
{
	free(cart->prg_rom);
	free(cart->chr_rom);
	free(cart->prg_ram);
	free(cart->chr_ram);
	free(cart);
}

const char *get_mapper_str(MapperID mapper)
{
	switch (mapper) {
		case (NROM):
			return "NROM";
		case (MMC1):
			return "MMC1";
		case (UxROM):
			return "UxROM";
		case (CNROM):
			return "CNROM";
		case (MMC3):
			return "MMC3";
		case (MMC5):
			return "MMC5";
		default:
			return "";
	}
}

void	nes_load_cartridge(t_nes *nes, t_cart *cart)
{
	nes->cart = cart;
	nes->ppu.mirroring = cart->mirroring;
	dprintf(nes->cpu.logfd, "title: %s\nheader: %s\nmapper: %d (%s)\nsubmapper: %d\nprg_rom: 0x%04zX\nchr_rom: 0x%04zX\nprg_ram: 0x%04zX\nchr_ram: 0x%04zX\nmirroring: %d\n",
		 cart->title,
		 cart->header_type == 1 ? "iNES" : "NES 2.0",
		 cart->mapper_id,
		 get_mapper_str(cart->mapper_id),
		 cart->submapper,
		 cart->prg_rom_banks * 0x4000,
		 cart->chr_rom_banks * 0x2000,
		 cart->prg_ram_banks * 0x2000,
		 cart->chr_ram_banks * 0x1000,
		 cart->mirroring
	);
	// exit(0);
	apply_nes_mmap(nes);
	init_mapper_mmap(nes, cart);
	refresh_mapper_mmap(nes, cart);
}
