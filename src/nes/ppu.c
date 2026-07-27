#include "audio_stream.h"
#include "emu6502.h"
#include "nes.h"
#include <raylib.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const uint32_t palette[] = {
	[0x00] = 0x626262FF,
	[0x01] = 0x001C95FF,
	[0x02] = 0x1904ACFF,
	[0x03] = 0x42009DFF,
	[0x04] = 0x61006BFF,
	[0x05] = 0x6E0025FF,
	[0x06] = 0x650500FF,
	[0x07] = 0x491E00FF,
	[0x08] = 0x223700FF,
	[0x09] = 0x004900FF,
	[0x0A] = 0x004F00FF,
	[0x0B] = 0x004816FF,
	[0x0C] = 0x00355EFF,
	[0x0D] = 0x000000FF,
	[0x0E] = 0x000000FF,
	[0x0F] = 0x000000FF,
	[0x10] = 0xABABABFF,
	[0x11] = 0x0C4EDBFF,
	[0x12] = 0x3D2EFFFF,
	[0x13] = 0x7115F3FF,
	[0x14] = 0x9B0BB9FF,
	[0x15] = 0xB01262FF,
	[0x16] = 0xA92704FF,
	[0x17] = 0x894600FF,
	[0x18] = 0x576600FF,
	[0x19] = 0x237F00FF,
	[0x1A] = 0x008900FF,
	[0x1B] = 0x008332FF,
	[0x1C] = 0x006D90FF,
	[0x1D] = 0x000000FF,
	[0x1E] = 0x000000FF,
	[0x1F] = 0x000000FF,
	[0x20] = 0xFFFFFFFF,
	[0x21] = 0x57A5FFFF,
	[0x22] = 0x8287FFFF,
	[0x23] = 0xB46DFFFF,
	[0x24] = 0xDF60FFFF,
	[0x25] = 0xF863C6FF,
	[0x26] = 0xF8746DFF,
	[0x27] = 0xDE9020FF,
	[0x28] = 0xB3AE00FF,
	[0x29] = 0x81C800FF,
	[0x2A] = 0x56D522FF,
	[0x2B] = 0x3DD36FFF,
	[0x2C] = 0x3EC1C8FF,
	[0x2D] = 0x4E4E4EFF,
	[0x2E] = 0x000000FF,
	[0x2F] = 0x000000FF,
	[0x30] = 0xFFFFFFFF,
	[0x31] = 0xBEE0FFFF,
	[0x32] = 0xCDD4FFFF,
	[0x33] = 0xE0CAFFFF,
	[0x34] = 0xF1C4FFFF,
	[0x35] = 0xFCC4EFFF,
	[0x36] = 0xFDCACEFF,
	[0x37] = 0xF5D4AFFF,
	[0x38] = 0xE6DF9CFF,
	[0x39] = 0xD3E99AFF,
	[0x3A] = 0xC2EFA8FF,
	[0x3B] = 0xB7EFC4FF,
	[0x3C] = 0xB6EAE5FF,
	[0x3D] = 0xB8B8B8FF,
	[0x3E] = 0x000000FF,
	[0x3F] = 0x000000FF,
};

const Color palette_alt[] = {
	[0x00] = { .r = 0x62, .g = 0x62, .b = 0x62, .a = 0xFF },
	[0x01] = { .r = 0x00, .g = 0x1C, .b = 0x95, .a = 0xFF },
	[0x02] = { .r = 0x19, .g = 0x04, .b = 0xAC, .a = 0xFF },
	[0x03] = { .r = 0x42, .g = 0x00, .b = 0x9D, .a = 0xFF },
	[0x04] = { .r = 0x61, .g = 0x00, .b = 0x6B, .a = 0xFF },
	[0x05] = { .r = 0x6E, .g = 0x00, .b = 0x25, .a = 0xFF },
	[0x06] = { .r = 0x65, .g = 0x05, .b = 0x00, .a = 0xFF },
	[0x07] = { .r = 0x49, .g = 0x1E, .b = 0x00, .a = 0xFF },
	[0x08] = { .r = 0x22, .g = 0x37, .b = 0x00, .a = 0xFF },
	[0x09] = { .r = 0x00, .g = 0x49, .b = 0x00, .a = 0xFF },
	[0x0A] = { .r = 0x00, .g = 0x4F, .b = 0x00, .a = 0xFF },
	[0x0B] = { .r = 0x00, .g = 0x48, .b = 0x16, .a = 0xFF },
	[0x0C] = { .r = 0x00, .g = 0x35, .b = 0x5E, .a = 0xFF },
	[0x0D] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x0E] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x0F] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x10] = { .r = 0xAB, .g = 0xAB, .b = 0xAB, .a = 0xFF },
	[0x11] = { .r = 0x0C, .g = 0x4E, .b = 0xDB, .a = 0xFF },
	[0x12] = { .r = 0x3D, .g = 0x2E, .b = 0xFF, .a = 0xFF },
	[0x13] = { .r = 0x71, .g = 0x15, .b = 0xF3, .a = 0xFF },
	[0x14] = { .r = 0x9B, .g = 0x0B, .b = 0xB9, .a = 0xFF },
	[0x15] = { .r = 0xB0, .g = 0x12, .b = 0x62, .a = 0xFF },
	[0x16] = { .r = 0xA9, .g = 0x27, .b = 0x04, .a = 0xFF },
	[0x17] = { .r = 0x89, .g = 0x46, .b = 0x00, .a = 0xFF },
	[0x18] = { .r = 0x57, .g = 0x66, .b = 0x00, .a = 0xFF },
	[0x19] = { .r = 0x23, .g = 0x7F, .b = 0x00, .a = 0xFF },
	[0x1A] = { .r = 0x00, .g = 0x89, .b = 0x00, .a = 0xFF },
	[0x1B] = { .r = 0x00, .g = 0x83, .b = 0x32, .a = 0xFF },
	[0x1C] = { .r = 0x00, .g = 0x6D, .b = 0x90, .a = 0xFF },
	[0x1D] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x1E] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x1F] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x20] = { .r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
	[0x21] = { .r = 0x57, .g = 0xA5, .b = 0xFF, .a = 0xFF },
	[0x22] = { .r = 0x82, .g = 0x87, .b = 0xFF, .a = 0xFF },
	[0x23] = { .r = 0xB4, .g = 0x6D, .b = 0xFF, .a = 0xFF },
	[0x24] = { .r = 0xDF, .g = 0x60, .b = 0xFF, .a = 0xFF },
	[0x25] = { .r = 0xF8, .g = 0x63, .b = 0xC6, .a = 0xFF },
	[0x26] = { .r = 0xF8, .g = 0x74, .b = 0x6D, .a = 0xFF },
	[0x27] = { .r = 0xDE, .g = 0x90, .b = 0x20, .a = 0xFF },
	[0x28] = { .r = 0xB3, .g = 0xAE, .b = 0x00, .a = 0xFF },
	[0x29] = { .r = 0x81, .g = 0xC8, .b = 0x00, .a = 0xFF },
	[0x2A] = { .r = 0x56, .g = 0xD5, .b = 0x22, .a = 0xFF },
	[0x2B] = { .r = 0x3D, .g = 0xD3, .b = 0x6F, .a = 0xFF },
	[0x2C] = { .r = 0x3E, .g = 0xC1, .b = 0xC8, .a = 0xFF },
	[0x2D] = { .r = 0x4E, .g = 0x4E, .b = 0x4E, .a = 0xFF },
	[0x2E] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x2F] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x30] = { .r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
	[0x31] = { .r = 0xBE, .g = 0xE0, .b = 0xFF, .a = 0xFF },
	[0x32] = { .r = 0xCD, .g = 0xD4, .b = 0xFF, .a = 0xFF },
	[0x33] = { .r = 0xE0, .g = 0xCA, .b = 0xFF, .a = 0xFF },
	[0x34] = { .r = 0xF1, .g = 0xC4, .b = 0xFF, .a = 0xFF },
	[0x35] = { .r = 0xFC, .g = 0xC4, .b = 0xEF, .a = 0xFF },
	[0x36] = { .r = 0xFD, .g = 0xCA, .b = 0xCE, .a = 0xFF },
	[0x37] = { .r = 0xF5, .g = 0xD4, .b = 0xAF, .a = 0xFF },
	[0x38] = { .r = 0xE6, .g = 0xDF, .b = 0x9C, .a = 0xFF },
	[0x39] = { .r = 0xD3, .g = 0xE9, .b = 0x9A, .a = 0xFF },
	[0x3A] = { .r = 0xC2, .g = 0xEF, .b = 0xA8, .a = 0xFF },
	[0x3B] = { .r = 0xB7, .g = 0xEF, .b = 0xC4, .a = 0xFF },
	[0x3C] = { .r = 0xB6, .g = 0xEA, .b = 0xE5, .a = 0xFF },
	[0x3D] = { .r = 0xB8, .g = 0xB8, .b = 0xB8, .a = 0xFF },
	[0x3E] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
	[0x3F] = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF },
};

const char *get_ppureg_str(PPUReg reg)
{
	switch (reg) {
		case (PPUCTRL):
			return "PPUCTRL";
		case (PPUMASK):
			return "PPUMASK";
		case (PPUSTATUS):
			return "PPUSTATUS";
		case (OAMADDR):
			return "OAMADDR";
		case (OAMDATA):
			return "OAMDATA";
		case (PPUSCROLL):
			return "PPUSCROLL";
		case (PPUADDR):
			return "PPUADDR";
		case (PPUDATA):
			return "PPUDATA";
	}
	return NULL;
}

uint8_t	ppu_palette_page_read(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_ppu *ppu = arg;

	addr &= 0x3FF;
	if (addr >= 0x300)
	{
		addr &= 0x1F;
		if (addr == 0x10)
			addr = 0x00;
		else if (addr == 0x14)
			addr = 0x04;
		else if (addr == 0x18)
			addr = 0x08;
		else if (addr == 0x1C)
			addr = 0x0C;
		return ppu->palette[addr];
	}

	return entry->memory[addr];
}

void	ppu_palette_page_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_ppu *ppu = arg;

	addr &= 0x3FF;
	if (addr >= 0x300)
	{
		addr &= 0x1F;
		if (addr == 0x10)
			addr = 0x00;
		else if (addr == 0x14)
			addr = 0x04;
		else if (addr == 0x18)
			addr = 0x08;
		else if (addr == 0x1C)
			addr = 0x0C;
		ppu->palette[addr] = val;
		return ;
	}

	entry->memory[addr] = val;
}

void	map_ppu_pattern_tables(t_nes *nes, t_cart *cart)
{
	void	(*write_handler)(struct pt_entry *, void *, uint16_t, uint8_t) = (cart->chr_rom_banks > 0) ? NULL : passthrough_write;
	uint8_t	*memory = cart->chr_rom_banks > 0 ? cart->chr_rom : cart->chr_ram;

	map_memory(nes->ppu.pagetable, 0x0000, 8, memory, NULL, write_handler);
}

void	logging_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	printf("addr: 0x%04X val: 0x%02X\n", addr, val);
	entry->memory[addr & 0x3FF] = val;
	(void)arg;
}

void map_ppu_nametables(t_ppu *ppu, void *read_handler, int mirror_mode)
{
	switch (mirror_mode) {
		case (MIRROR_HORIZONTAL):
			map_memory(ppu->pagetable, 0x2000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2400, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2800, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2C00, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3400, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3800, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3C00, 1, &ppu->vram[0x400], ppu_palette_page_read, ppu_palette_page_write);
			break ;
		case (MIRROR_VERTICAL):
			map_memory(ppu->pagetable, 0x2000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2400, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2800, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2C00, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3400, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3800, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3C00, 1, &ppu->vram[0x400], ppu_palette_page_read, ppu_palette_page_write);
			break ;
		case (MIRROR_SINGLE_LOW):
			map_memory(ppu->pagetable, 0x2000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2400, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2800, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2C00, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3000, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3400, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3800, 1, ppu->vram, read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3C00, 1, ppu->vram, read_handler, passthrough_write);
			break ;
		case (MIRROR_SINGLE_HIGH):
			map_memory(ppu->pagetable, 0x2000, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2400, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2800, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x2C00, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3000, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3400, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3800, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			map_memory(ppu->pagetable, 0x3C00, 1, &ppu->vram[0x400], read_handler, passthrough_write);
			break ;
		default:
			printf("\e[31;1mERROR\e[m: invalid mirror mode\n");
			exit(2);
	}
}

uint8_t	ppu_read(t_ppu *ppu, uint16_t addr)
{
	uint8_t	pageno = addr >> 10;
	struct pt_entry *entry = &ppu->pagetable[pageno];

	if (addr < 0x3F00)
		ppu->addrbus = addr;

	if (entry->read_handler)
		return entry->read_handler(entry, ppu, addr);

	return entry->memory[addr & 0x03FF];
}

void	ppu_write(t_ppu *ppu, uint16_t addr, uint8_t val)
{
	uint8_t	pageno = addr >> 10;
	struct pt_entry *entry = &ppu->pagetable[pageno];

	if (addr < 0x3F00)
		ppu->addrbus = addr;

	if (entry->write_handler)
		entry->write_handler(entry, ppu, addr, val);
}

void	fetch_tile_row(t_ppu *ppu, uint8_t *buf, uint8_t table, uint8_t tile_id, uint8_t row)
{
	uint16_t addr = tile_id * 16 + row + (table * 0x1000);
	uint8_t lo = ppu_read(ppu, addr);
	uint8_t hi = ppu_read(ppu, addr + 8);
	for (int i = 0; i < 8; i++)
	{
		uint8_t shift = 7 - i;
		buf[i] = ((lo >> shift) & 0x1) | (((hi >> shift) & 0x1) << 1);
	}
}

void	fetch_sprite_row_8x16(t_ppu *ppu, t_sprite *sprite, uint8_t row)
{
	uint16_t	addr;
	uint8_t		table = sprite->tile_id & 0x1;
	uint8_t		tile_id = sprite->tile_id & 0xFE;

	if (sprite->attr & BIT_7)
	{
		if (row < 8)
			addr = (table * 0x1000) | ((tile_id + 1) << 4) | (7 - row);
		else
			addr = (table * 0x1000) | (tile_id << 4) | (7 - (row - 8));
	}
	else
	{
		if (row < 8)
			addr = (table * 0x1000) | (tile_id << 4) | row;
		else
			addr = (table * 0x1000) | ((tile_id + 1) << 4) | (row - 8);
	}

	uint8_t lo = ppu_read(ppu, addr);
	uint8_t hi = ppu_read(ppu, addr + 8);
	for (int i = 0; i < 8; i++)
	{
		uint8_t shift = 7 - i;
		sprite->pixels[i] = ((lo >> shift) & 0x1) | (((hi >> shift) & 0x1) << 1);
	}
}

uint8_t	get_palette_id(t_ppu *ppu)
{
	uint16_t	coarse_x = ppu->v & 0x1F;
	uint16_t	coarse_y = (ppu->v >> 5) & 0x1F;

	uint16_t	attr_addr = 0x23C0 | (ppu->v & 0x0C00) | ((ppu->v >> 4) & 0x38) | ((ppu->v >> 2) & 0x7);
	uint8_t		attr_byte = ppu_read(ppu, attr_addr);

	// printf("attr_addr: 0x%04X attr_byte: 0x%02X\n", attr_addr, attr_byte);

	uint8_t shift = ((coarse_y & 0x02) << 1) | (coarse_x & 0x02);
	return (attr_byte >> shift) & 0x03;
}

uint8_t get_bg_pixel_color(t_ppu *ppu, uint8_t pixel, uint8_t palette_id, bool *bg_opaque, uint32_t x_pos)
{
	uint8_t	col_index;
	bool draw_bg = (ppu->registers[PPUMASK] & PPUMASK_BG) && ((ppu->registers[PPUMASK] & PPUMASK_BG_LEFT) || (x_pos > 7));

	if (pixel == 0 || !draw_bg)
	{
		col_index = ppu_read(ppu, 0x3F00);
		*bg_opaque = false;
		// printf("transparent col index: %d\n", col_index);
	}
	else
	{
		col_index = ppu_read(ppu, 0x3F00 | (palette_id << 2) | pixel);
		*bg_opaque = true;
	}
	// uint32_t col = palette[col_index & 0x3F];
	return col_index;
}

void	get_sprite_pixel(t_ppu *ppu, uint32_t x_pos, bool bg_opaque, uint8_t *col_index)
{
	if (!(ppu->registers[PPUMASK] & PPUMASK_SP) || (!(ppu->registers[PPUMASK] & PPUMASK_SP_LEFT) && (x_pos < 8)))
		return ;

	for (uint8_t i = 0; i < ppu->secondary_count; i++)
	{
		t_sprite *sprite = &ppu->secondary_oam[i];

		uint32_t x_diff = x_pos - sprite->x;
		if (x_diff < 8)
		{
			if (sprite->attr & BIT_6)
				x_diff = 7 - x_diff;

			// if (sprite->sprite_0)
			// {
			// 	printf("\e[32;1mSPRITE 0\e[m: x: %3d y: %3d tile_id: %3d attr: 0x%02X\n", sprite->x, sprite->y, sprite->tile_id, sprite->attr);
			// 	printf("scanline: %u cycle: %u diff: %u bg_opaque: %u\n", ppu->scanline, ppu->cycle, x_diff, bg_opaque);
			// 	for (uint8_t i = 0; i < 8; i++)
			// 		printf("[%d] ", sprite->pixels[x_diff]);
			// 	printf("\n\n");
			// }
			uint8_t pixel = sprite->pixels[x_diff];
			if (pixel == 0)
				continue ;

			// if (ppu->scanline == 0)
			// 	printf("DRAW: pos: (%d, %d) col: %d\n", x_pos, sprite->y, pixel);

			if (sprite->oam_index == 0 && bg_opaque && x_pos != 255)
			{
				ppu->registers[PPUSTATUS] |= SPRITE_0_HIT;
				// printf("SPRITE_0 HIT! pos: (%d, %d)\n", x_pos, ppu->scanline);
				// printf("\e[31;1mSprite 0 Hit!\e[m:  scanline %3d cycle %3d\n", ppu->scanline, ppu->cycle);
			}

			if (!(sprite->attr & BIT_5) || !bg_opaque)
			{
				uint8_t palette_id = sprite->attr & 0x03;
				*col_index = ppu_read(ppu, 0x3F10 | (palette_id << 2) | pixel);
			}

			break ;
		}
	}
}

void	ppu_draw_tile_row(t_ppu *ppu)
{
	uint8_t pixbuf[8];
	uint8_t tile_id = ppu_read(ppu, 0x2000 | (ppu->v & 0xFFF));
	uint8_t palette_id = get_palette_id(ppu);
	uint8_t table_select = (ppu->registers[PPUCTRL] >> 4) & 0x1;
	uint8_t fine_y = (ppu->v >> 12) & 0x7;

	fetch_tile_row(ppu, pixbuf, table_select, tile_id, fine_y);

	for (uint32_t i = 0; i < 8; i++)
	{
		bool		bg_opaque;
		uint32_t	x = (ppu->cycle - 8) + i - ppu->x;

		if (x > 255)
			continue ;

		uint8_t col_index = get_bg_pixel_color(ppu, pixbuf[i], palette_id, &bg_opaque, x);

		get_sprite_pixel(ppu, x, bg_opaque, &col_index);

		if (ppu->registers[PPUMASK] & PPUMASK_GREY)
			col_index &= 0x30;
		// uint32_t col = palette[col_index & 0x3F];
		Color col = palette_alt[col_index & 0x3F];
		if (ppu->scanline < 240)
			// draw_pixel(ppu, x, ppu->scanline, col);
			draw_pixel(ppu, x, ppu->scanline, *(uint32_t *)&col);
	}
}

void	ppu_increment_x(t_ppu *ppu)
{
	if ((ppu->v & 0x1F) == 31)
	{
		ppu->v &= ~0x1F;
		ppu->v ^= 0x400;
	}
	else
		ppu->v++;
}

void	ppu_increment_y(t_ppu *ppu)
{
	if ((ppu->v & 0x7000) != 0x7000)
	{
    	ppu->v += 0x1000;
	}
	else
	{
		ppu->v &= ~0x7000;

		int y = (ppu->v & 0x03E0) >> 5;
		if (y == 29)
		{
			y = 0;
			ppu->v ^= 0x0800;
		}
		else if (y == 31)
		{
			y = 0;
		}
		else
		{
			y += 1;
		}
		ppu->v = (ppu->v & ~0x03E0) | (y << 5);
	}
}

void	get_sprite_data(t_ppu *ppu, t_sprite *sprite, uint32_t oam_index)
{
	uint32_t i = oam_index * 4;
	sprite->y = ppu->oam[i];
	sprite->tile_id = ppu->oam[i + 1];
	sprite->attr = ppu->oam[i + 2];
	sprite->x = ppu->oam[i + 3];
	sprite->oam_index = oam_index;
}

void	find_scanline_sprites(t_ppu *ppu)
{
	ppu->secondary_count = 0;
	memset(ppu->secondary_oam, 0, sizeof(t_sprite) * 8);
	const bool is_8x16_mode = ppu->registers[PPUCTRL] & BIT_5;
	const uint32_t min_diff = is_8x16_mode ? 16 : 8;

	for (int32_t i = ppu->oam_addr; i < 64; i++)
	{
		uint8_t y = ppu->oam[i * 4];
		if (y >= 0xEF)
			continue ;
		uint32_t diff = ppu->scanline - y;
		if (diff < min_diff)
		{
			if (ppu->secondary_count < 8)
			{
				t_sprite *sprite = &ppu->secondary_oam[ppu->secondary_count];
				get_sprite_data(ppu, sprite, i);

				if (is_8x16_mode)
					fetch_sprite_row_8x16(ppu, sprite, diff);
				else
				{
					if (sprite->attr & BIT_7)
						diff = 7 - diff;
					fetch_tile_row(ppu, sprite->pixels, (ppu->registers[PPUCTRL] >> 3) & 0x1, sprite->tile_id, diff);
				}

				ppu->secondary_count++;
				// if (y == 0)
				// 	printf("FETCH: pos: (%d, %d)\n", sprite->x, ppu->scanline);
			}
			else
			{
				SET_BIT(ppu->registers[PPUSTATUS], SPRITE_OVERFLOW, true);
				return ;
			}
		}
	}
}

void	ppu_render_cycle_batched(t_ppu *ppu)
{
	if (ppu->cycle % 8 == 0 && ppu->cycle <= 264)
	{
		ppu_draw_tile_row(ppu);
		// if (ppu->cycle < 264)
			ppu_increment_x(ppu);
		if (ppu->cycle == 264)
			ppu_increment_y(ppu);
	}

	if (ppu->cycle == 265)
	{
		ppu->v = (ppu->v & ~0x41F) | (ppu->t & 0x41F);
		find_scanline_sprites(ppu);

		// if (ppu->nes->cart->mapper_id == MMC3)
		// {
		// 	bool sprite_a12 = (ppu->registers[PPUCTRL] & 0x08) != 0;
		// 	size_t cpu_cycle = ppu->total_cycles / 3;
		// 	mmc3_clock_a12(ppu->nes, sprite_a12, cpu_cycle);
		// }
	}
	if (ppu->cycle >= 265 && ppu->cycle <= 320)
	{
		// printf("OAMADDR reset scanline: %d cycle: %d\n", ppu->scanline, ppu->cycle);
		ppu->oam_addr = 0;
	}

	// if (ppu->cycle == 320 && ppu->nes->cart->mapper_id == MMC3)
	// {
	// 	bool bg_a12 = (ppu->registers[PPUCTRL] & 0x10) != 0;
	// 	size_t cpu_cycle = ppu->total_cycles / 3;
	// 	mmc3_clock_a12(ppu->nes, bg_a12, cpu_cycle);
	// }

	if (ppu->scanline == 261 && ppu->cycle >= 280 && ppu->cycle <= 304)
	{
		ppu->v = (ppu->v & ~0x7BE0) | (ppu->t & 0x7BE0);
	}
}

static inline void refill_shift_registers(t_ppu *ppu)
{
	ppu->bg_shift.pattern_lo = (ppu->bg_shift.pattern_lo & 0xFF00) | ppu->bg_shift.next_pt_lo;
	ppu->bg_shift.pattern_hi = (ppu->bg_shift.pattern_hi & 0xFF00) | ppu->bg_shift.next_pt_hi;
	ppu->bg_shift.attr_lo = (ppu->bg_shift.attr_lo & 0xFF00) | ((ppu->bg_shift.next_attr_byte & 0x1) ? 0xFF : 0x00);
	ppu->bg_shift.attr_hi = (ppu->bg_shift.attr_hi & 0xFF00) | ((ppu->bg_shift.next_attr_byte & 0x2) ? 0xFF : 0x00);
}

static inline void fix_attr_byte(t_ppu *ppu)
{
	uint8_t shift_amount = (ppu->v & 0x02) | ((ppu->v >> 4) & 0x04);
	ppu->bg_shift.next_attr_byte = (ppu->bg_shift.next_attr_byte >> shift_amount) & 0x03;
}

void ppu_output_pixel(t_ppu *ppu)
{
	uint8_t		pixel_x = ppu->cycle - 1;
	uint8_t		pixel_y = ppu->scanline;
	uint16_t	bit_select = 0x8000 >> ppu->x;

	uint8_t		pixel_lo = (ppu->bg_shift.pattern_lo & bit_select) ? 1 : 0;
	uint8_t		pixel_hi = (ppu->bg_shift.pattern_hi & bit_select) ? 1 : 0;
	uint8_t		bg_col = (pixel_hi << 1) | pixel_lo;

	uint8_t		attr_lo = (ppu->bg_shift.attr_lo & bit_select) ? 1 : 0;
	uint8_t		attr_hi = (ppu->bg_shift.attr_hi & bit_select) ? 1 : 0;
	uint8_t		bg_palette = (attr_hi << 1) | attr_lo;

	bool draw_bg = (ppu->registers[PPUMASK] & PPUMASK_BG) && ((ppu->registers[PPUMASK] & PPUMASK_BG_LEFT) || (pixel_x > 7));

	if (!draw_bg)
		bg_col = 0;

	uint8_t	sprite_col = 0;
	uint8_t	sprite_palette = 0;
	bool	sprite_priority = false;
	bool	sprite_0 = false;

	bool draw_sprite = (ppu->registers[PPUMASK] & PPUMASK_SP) && ((ppu->registers[PPUMASK] & PPUMASK_SP_LEFT) || (pixel_x > 7));

	if (draw_sprite)
	{
		for (int i = 0; i < 8; i++)
		{
			if (ppu->sprite_shift.x_counter[i] == 0)
			{
				pixel_lo = (ppu->sprite_shift.pattern_lo[i] & 0x80) ? 1 : 0;
				pixel_hi = (ppu->sprite_shift.pattern_hi[i] & 0x80) ? 1 : 0;
				sprite_col = (pixel_hi << 1) | pixel_lo;

				if (sprite_col != 0)
				{
					sprite_palette = (ppu->sprite_shift.attr[i] & 0x03) + 4;
					sprite_priority = (ppu->sprite_shift.attr[i] & SPRITE_PRIORITY) == 0;
					sprite_0 = ppu->sprite_shift.sprite_0[i];
					break;
				}
			}
		}
	}

	uint8_t final_col = 0;
	uint8_t final_palette = 0;
	uint8_t pixel_mux = ((bg_col > 0) << 1) | (sprite_col > 0);

	switch (pixel_mux) {
		case (0):
			break;
		case (1):
			final_col = sprite_col;
			final_palette = sprite_palette;
			break;
		case (2):
			final_col = bg_col;
			final_palette = bg_palette;
			break;
		case (3):
			if (sprite_priority)
			{
				final_col = sprite_col;
				final_palette = sprite_palette;
			}
			else
			{
				final_col = bg_col;
				final_palette = bg_palette;
			}

			if (sprite_0 && pixel_x != 255)
				ppu->registers[PPUSTATUS] |= SPRITE_0_HIT;
			break;
	}

	uint8_t col_index;
	if (final_col == 0)
		col_index = ppu_read(ppu, 0x3F00);
	else
		col_index = ppu_read(ppu, 0x3F00 | (final_palette << 2) | final_col);

	if (ppu->registers[PPUMASK] & PPUMASK_GREY)
		col_index &= 0x30;

	Color col = palette_alt[col_index & 0x3F];
	if (pixel_y < 240)
		draw_pixel(ppu, pixel_x, pixel_y, *(uint32_t *)&col);
}

void	ppu_fetch_bg(t_ppu *ppu, uint16_t cycle, bool is_shifting)
{
	uint8_t sub_cycle = (cycle - 1) % 8;
	uint16_t addr;

	switch (sub_cycle) {
		case (0):
			if (is_shifting)
				refill_shift_registers(ppu);
			addr = 0x2000 | (ppu->v & 0xFFF);
			ppu->bg_shift.next_nt_byte = ppu_read(ppu, addr);
			break;
		case (2):
			addr = 0x23C0 | (ppu->v & 0x0C00) | ((ppu->v >> 4) & 0x38) | ((ppu->v >> 2) & 0x7);
			ppu->bg_shift.next_attr_byte = ppu_read(ppu, addr);
			fix_attr_byte(ppu);
			break;
		case (4):
			addr = ((ppu->registers[PPUCTRL] & PPUCTRL_BG_PTABLE) ? 0x1000 : 0x0000)
				| (ppu->bg_shift.next_nt_byte << 4)
				| ((ppu->v >> 12) & 0x07);
			ppu->bg_shift.next_pt_lo = ppu_read(ppu, addr);
			break;
		case (6):
			addr = ((ppu->registers[PPUCTRL] & PPUCTRL_BG_PTABLE) ? 0x1000 : 0x0000)
				| (ppu->bg_shift.next_nt_byte << 4)
				| ((ppu->v >> 12) & 0x07);
			addr += 8;
			ppu->bg_shift.next_pt_hi = ppu_read(ppu, addr);
			break;
		case (7):
			ppu_increment_x(ppu);
			break;
	}
}

void	ppu_evaluate_sprites(t_ppu *ppu, uint16_t cycle)
{
	if (cycle <= 64)
	{
		if (cycle % 2 == 0)
		{
			uint8_t	byte_idx = (cycle - 1) / 2;
			uint8_t	sprite_idx = byte_idx / 4;
			uint8_t	offset = byte_idx % 4;

			switch (offset) {
				case (0):
					ppu->secondary_oam[sprite_idx].y = 0xFF;
					break;
				case (1):
					ppu->secondary_oam[sprite_idx].tile_id = 0xFF;
					break;
				case (2):
					ppu->secondary_oam[sprite_idx].attr = 0xFF;
					break;
				case (3):
					ppu->secondary_oam[sprite_idx].x = 0xFF;
					ppu->secondary_oam[sprite_idx].oam_index = 0xFF;
					break;
			}
		}

		if (cycle == 64)
		{
			ppu->oam_ptr = 0;
			ppu->sec_oam_ptr = 0;
			ppu->oam_byte_offset = 0;
			ppu->evalstate = FIND_SPRITES;
		}

		return;
	}

	uint8_t		oam_data = ppu->oam[(ppu->oam_ptr << 2) + ppu->oam_byte_offset];
	uint8_t		sprite_height = ppu->registers[PPUCTRL] & PPUCTRL_SPRITE_SIZE ? 16 : 8;
	t_sprite	*cur_sprite;

	switch (ppu->evalstate) {
		case (FIND_SPRITES):
			cur_sprite = &ppu->secondary_oam[ppu->sec_oam_ptr];
			cur_sprite->y = oam_data;
			if (ppu->scanline >= oam_data && ppu->scanline < (uint8_t)(oam_data + sprite_height))
			{
				cur_sprite->oam_index = ppu->oam_ptr;
				ppu->oam_byte_offset++;
				ppu->evalstate = COPY_DATA;
			}
			else
			{
				ppu->oam_ptr++;
				if (ppu->oam_ptr == 64)
				{
					ppu->oam_ptr = 0;
					ppu->evalstate = COMPLETE;
				}
			}
			break;
		case (COPY_DATA):
			cur_sprite = &ppu->secondary_oam[ppu->sec_oam_ptr];
			switch (ppu->oam_byte_offset) {
				case (1): cur_sprite->tile_id = oam_data; break;
				case (2): cur_sprite->attr = oam_data; break;
				case (3): cur_sprite->x = oam_data; break;
			}
			ppu->oam_byte_offset++;

			if (ppu->oam_byte_offset == 4)
			{
				ppu->oam_byte_offset = 0;
				ppu->sec_oam_ptr++;
				ppu->oam_ptr++;

				if (ppu->oam_ptr == 64)
				{
					ppu->oam_ptr = 0;
					ppu->evalstate = COMPLETE;
				}
				else if (ppu->sec_oam_ptr == 8)
					ppu->evalstate = OVERFLOW;
				else
					ppu->evalstate = FIND_SPRITES;
			}
			break;
		case (OVERFLOW):
			if (ppu->scanline >= oam_data && ppu->scanline < (uint16_t)(oam_data + sprite_height))
			{
				ppu->registers[PPUSTATUS] |= SPRITE_OVERFLOW;

				ppu->oam_byte_offset++;
				if (ppu->oam_byte_offset == 4)
				{
					ppu->oam_byte_offset = 0;
					ppu->oam_ptr++;
				}
			}
			else
			{
				ppu->oam_ptr++;
				ppu->oam_byte_offset = (ppu->oam_byte_offset + 1) & 0x03; // Sprite overflow bug for accuracy
			}

			if (ppu->oam_ptr == 64)
			{
				ppu->oam_ptr = 0;
				ppu->evalstate = COMPLETE;
			}
			break;
		case (COMPLETE):
			break;
	}
}

static inline uint8_t flip_byte(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

    return b;
}

static inline uint16_t	calculate_sprite_addr(t_ppu *ppu, t_sprite *sprite, bool high_byte)
{
	uint16_t	addr;
	uint16_t	tile_idx;
	uint8_t		row;
	bool		is_8x16 = (ppu->registers[PPUCTRL] & PPUCTRL_SPRITE_SIZE) != 0;
	bool		v_flip = (sprite->attr & SPRITE_V_FLIP) != 0;

	if (!is_8x16)
	{
		addr = (ppu->registers[PPUCTRL] & PPUCTRL_SPRITE_PTABLE) ? 0x1000 : 0x0000;
		tile_idx = sprite->tile_id;

		row = (ppu->scanline - sprite->y) & 0x07;
		if (v_flip)
			row = 7 - row;
	}
	else
	{
		addr = (sprite->tile_id & 0x01) ? 0x1000 : 0x0000;
		tile_idx = sprite->tile_id & 0xFE;

		row = (ppu->scanline - sprite->y) & 0x0F;
		if (v_flip)
			row = 15 - row;

		if (row >= 8)
		{
			tile_idx++;
			row &= 0x07;
		}
	}

	addr = addr | (tile_idx << 4) | row;
	if (high_byte)
		addr += 8;

	return addr;
}

void ppu_fetch_sprites(t_ppu *ppu, uint16_t cycle)
{
	uint8_t		sprite_idx = (cycle - 257) / 8;
	uint8_t		sub_cycle = (cycle - 257) % 8;
	uint16_t	addr;

	t_sprite *cur_sprite = &ppu->secondary_oam[sprite_idx];
	switch (sub_cycle) {
		case (0):
		case (2):
			addr = 0x2000 | (ppu->v & 0xFFF);
			ppu_read(ppu, addr);
			break;
		case (4):
			addr = calculate_sprite_addr(ppu, cur_sprite, false);
			ppu->sprite_shift.tmp_pattern_lo = ppu_read(ppu, addr);
			break;
		case (6):
			addr = calculate_sprite_addr(ppu, cur_sprite, true);
			ppu->sprite_shift.tmp_pattern_hi = ppu_read(ppu, addr);
			break;
		case (7):
			if (cur_sprite->oam_index > 64)
			{
				ppu->sprite_shift.pattern_lo[sprite_idx] = 0;
				ppu->sprite_shift.pattern_hi[sprite_idx] = 0;
			}
			else if (cur_sprite->attr & SPRITE_H_FLIP)
			{
				ppu->sprite_shift.pattern_lo[sprite_idx] = flip_byte(ppu->sprite_shift.tmp_pattern_lo);
				ppu->sprite_shift.pattern_hi[sprite_idx] = flip_byte(ppu->sprite_shift.tmp_pattern_hi);
			}
			else
			{
				ppu->sprite_shift.pattern_lo[sprite_idx] = ppu->sprite_shift.tmp_pattern_lo;
				ppu->sprite_shift.pattern_hi[sprite_idx] = ppu->sprite_shift.tmp_pattern_hi;
			}
			ppu->sprite_shift.attr[sprite_idx] = cur_sprite->attr;
			ppu->sprite_shift.x_counter[sprite_idx] = cur_sprite->x;
			ppu->sprite_shift.sprite_0[sprite_idx] = (cur_sprite->oam_index == 0);
			break;
	}
}

static inline void ppu_shift_bg(t_ppu *ppu)
{
	ppu->bg_shift.pattern_lo <<= 1;
	ppu->bg_shift.pattern_hi <<= 1;
	ppu->bg_shift.attr_lo <<= 1;
	ppu->bg_shift.attr_hi <<= 1;
}

static inline void ppu_shift_sprites(t_ppu *ppu)
{
	for (int i = 0; i < 8; i++)
	{
		if (ppu->sprite_shift.x_counter[i] > 0)
			ppu->sprite_shift.x_counter[i]--;
		else
		{
			ppu->sprite_shift.pattern_lo[i] <<= 1;
			ppu->sprite_shift.pattern_hi[i] <<= 1;
		}
	}
}

void	ppu_render_cycle(t_ppu *ppu)
{
	uint16_t	cycle = ppu->cycle;
	bool		shifting_bg = (cycle >= 2 && cycle <= 257) || (cycle >= 322 && cycle <= 337);
	bool		fetching_bg = (cycle <= 256) || (cycle >= 321 && cycle <= 340);
	bool		fetching_sprites = (cycle >= 257 && cycle <= 320);
	bool		hblank = cycle > 256;

	if (shifting_bg)
		ppu_shift_bg(ppu);

	if (!hblank)
	{
		ppu_output_pixel(ppu);
		ppu_evaluate_sprites(ppu, cycle);
		ppu_shift_sprites(ppu);
	}

	if (fetching_bg)
		ppu_fetch_bg(ppu, cycle, shifting_bg);
	else if (fetching_sprites)
		ppu_fetch_sprites(ppu, cycle);

	if (cycle == 256)
		ppu_increment_y(ppu);
	else if (cycle == 257)
		ppu->v = (ppu->v & ~0x41F) | (ppu->t & 0x41F);
	else if (ppu->scanline == 261 && cycle >= 280 && cycle <= 304)
		ppu->v = (ppu->v & ~0x7BE0) | (ppu->t & 0x7BE0);
}

static inline void ppu_handle_mapper_behaviour(t_ppu *ppu)
{
	t_nes *nes = ppu->nes;

	switch (nes->cart->mapper_id) {
		case (MMC3):
			if (ppu->addrbus & 0x1000)
			{
				if (nes->mapper.mmc3.a12_low_cycles >= 9)
				{
					mmc3_clock_a12(nes);
				}
				nes->mapper.mmc3.a12_low_cycles = 0;
			}
			else
			{
				nes->mapper.mmc3.a12_low_cycles++;
			}
			break;
		default:
			break;
	}
}

void ppu_tick(t_ppu *ppu)
{
	bool	is_rendering = ppu->registers[PPUMASK] & (PPUMASK_BG | PPUMASK_SP);

	// if (ppu->scanline == 0 && ppu->cycle == 0)
	// {
	// 	memset(ppu->sprite_shift.pattern_lo, 0, 8);
	// 	memset(ppu->sprite_shift.pattern_hi, 0, 8);
	// }
	if ((ppu->scanline < 240 || ppu->scanline == 261) && is_rendering && ppu->cycle > 0)
	{
		// ppu_render_cycle_batched(ppu);
		ppu_render_cycle(ppu);
	}

	if (ppu->scanline == 241 && ppu->cycle == 1)
	{
		ppu->registers[PPUSTATUS] |= VBLANK_ACTIVE;
		// if (ppu->registers[PPUCTRL] & VBLANK_ENABLE)
		// 	*ppu->nmi_pin = true;

		ppu->nes->frame_ready = true;
	}

	if (ppu->scanline == 261 && ppu->cycle == 1)
	{
		ppu->registers[PPUSTATUS] &= ~VBLANK_ACTIVE;
		ppu->registers[PPUSTATUS] &= ~SPRITE_0_HIT;
		ppu->registers[PPUSTATUS] &= ~SPRITE_OVERFLOW;
	}

	bool nmi_active = (ppu->registers[PPUCTRL] & PPUCTRL_VBLANK_ENABLE)
						&& (ppu->registers[PPUSTATUS] & VBLANK_ACTIVE);

	if (nmi_active && !ppu->nmi_state_prev)
	{
		// printf("NMI firing: scanline %3d cycle %3d cpu_cycle: %lu\n", ppu->scanline, ppu->cycle, ppu->nes->cpu.cycles);
		*ppu->nmi_pin = true;
	}

	ppu->nmi_state_prev = nmi_active;

	ppu_handle_mapper_behaviour(ppu);
	ppu->cycle++;
	if (ppu->cycle == 341)
	{
		ppu->cycle = 0;
		ppu->scanline++;
		if (ppu->scanline == 262)
			ppu->scanline = 0;
	}
	ppu->total_cycles++;
}

void	ppu_tick_for(t_ppu *ppu, uint32_t n_ticks)
{
	for (uint32_t i = 0; i < n_ticks; i++)
		ppu_tick(ppu);
}
