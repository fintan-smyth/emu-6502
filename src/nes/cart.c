#include "emu6502.h"
#include "nes.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
	cart->cur_chr_bank = 0;
	cart->cur_prg_bank = 0;
	cart->prg_banks = hdr.prg_rom_size;
	cart->chr_banks = hdr.chr_rom_size;
	if (cart->chr_banks == 0)
	{
		cart->chr_banks = 1;
		cart->chr_is_ram = true;
	}
	printf("Flags6: 0x%02X\n", hdr.flags6);
	cart->mirroring = hdr.flags6 & 1;
	cart->mapper_id = ((hdr.flags6 >> 4) & 0x0F) | (hdr.flags7 & 0xF0);
	// if (cart->mapper_id != 0)
	// 	return (free(cart), NULL);

	if (hdr.flags6 & BIT_2)
		lseek(fd, 512, SEEK_CUR);

	cart->prg_rom = malloc(cart->prg_banks * 0x4000);
	cart->chr_mem = calloc(0x2000 * cart->chr_banks, 1);
	read(fd, cart->prg_rom, cart->prg_banks * 0x4000);
	if (!cart->chr_is_ram)
		read(fd, cart->chr_mem, cart->chr_banks * 0x2000);

	return cart;
}

void	free_cart(t_cart *cart)
{
	free(cart->prg_rom);
	free(cart->chr_mem);
	free(cart);
}

uint8_t	cpu_ppu_reg_read(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_cpu	*cpu = arg;
	t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	PPUReg	reg = addr & 0x7;
	uint8_t	data = 0;
	// printf("mapped: %04X reg: %02X\n", addr, reg);

	switch (reg) {
		case (PPUCTRL):
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (PPUMASK):
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (PPUSTATUS):
			data = ppu->registers[PPUSTATUS];
			SET_BIT(ppu->registers[PPUSTATUS], BIT_7, 0);
			ppu->w = 0;
			break;
		case (OAMADDR):
			printf("\e[31;1mError\e[m: %s: Invalid read\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (OAMDATA):
			data = ppu->oam[ppu->oam_addr];
			break;
		case (PPUSCROLL):
			break;
		case (PPUADDR):
			break;
		case (PPUDATA):
			data = ppu->readbuf;
			ppu->readbuf= ppu_read(ppu, ppu->v & 0x3FFF);
			if ((ppu->v & 0x3FFF) >= 0x3F00)
			{
				data = ppu->readbuf;
			}
			// printf("PPUDATA read: 0x%02X ppu->v: 0x%04X\n", data, ppu->v);
			ppu->v += (ppu->registers[PPUCTRL] & BIT_2) ? 32 : 1;
			break;
	}

	return data;
	(void)entry;
}

void	cpu_ppu_reg_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	PPUReg	reg = addr & 0x7;

	switch (reg) {
		case (PPUCTRL): // 0x2000
			SET_BIT(ppu->t, BIT_10, val & BIT_0);
			SET_BIT(ppu->t, BIT_11, val & BIT_1);
			entry->memory[reg] = val;
			return ;
		case (PPUMASK): // 0x2001
			break;
		case (PPUSTATUS): // 0x2002
			printf("\e[31;1mError\e[m: %s: Invalid write\n", get_ppureg_str(reg));
			exit(1);
			break;
		case (OAMADDR): // 0x2003
			ppu->registers[OAMADDR] = val;
			break;
		case (OAMDATA): // 0x2004
			ppu->oam[ppu->oam_addr++] = val;
			break;
		case (PPUSCROLL): // 0x2005
			if (ppu->w == 0)
			{
				ppu->w = 1;
				// Sets x to low 3 bits of val;
				ppu->x = val & 0x07;
				// Sets bits 0-4 of t to high 5 bits of val
				ppu->t = (ppu->t & ~0x1F) | (val >> 3);
			}
			else
			{
				ppu->w = 0;
				// Sets bits 12-14 of t to low 3 bits of val
				ppu->t = (ppu->t & ~(0x07 << 12)) | ((val & 0x07) << 12);
				// Sets bits 5-9 of t to high 5 bits of val
				ppu->t = (ppu->t & ~(0x1F << 5)) | (val >> 3);
			}
			// printf("PPUSCROLL write: 0x%02X ppu->t: 0x%04X\n", val, ppu->t);
			break;
		case (PPUADDR): // 0x2006
			if (ppu->w == 0)
			{
				ppu->w = 1;
				// Sets bits 8-13 of t to low 6 bits of val, bit 14 cleared
				ppu->t = (ppu->t & 0x80FF) | ((val & 0x3F) << 8);
			}
			else
			{
				ppu->w = 0;
				// Sets low byte of to to val
				ppu->t = (ppu->t & 0xFF00) | val;
				ppu->v = ppu->t;
			}
			// printf("PPUADDR write: 0x%02X ppu->t: 0x%04X\n", val, ppu->t);
			break;
		case (PPUDATA): // 0x2007
			ppu_write(ppu, ppu->v & 0x3FFF, val);
			// printf("PPUDATA write: 0x%02X ppu->v: 0x%04X\n", val, ppu->v);
			ppu->v += (ppu->registers[PPUCTRL] & BIT_2) ? 32 : 1;
			break;
	}
	entry->memory[reg] = val;
}

uint8_t cpu_io_page_read(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = (t_nes *)cpu->parent_device;
	// t_ppu	*ppu = &((t_nes *)cpu->parent_device)->ppu;
	uint8_t	out = 0;

	addr &= 0xFFF;
	if (addr > 0x17)
		return out;
	
	IOReg reg = addr & 0xFFF;
	switch (reg) {
		case (SND_CHN):
			return out;
		case (JOY1):
			if (nes->joy_strobe)
				nes->joy_shift[0] = nes->joy_state[0];

			out = nes->joy_shift[0] & 0x01;

			if (!nes->joy_strobe)
				nes->joy_shift[0] >>= 1;
			return out;
		case (JOY2):
			if (nes->joy_strobe)
				nes->joy_shift[1] = nes->joy_state[1];

			out = nes->joy_shift[1] & 0x01;

			if (!nes->joy_strobe)
				nes->joy_shift[1] >>= 1;
			return out;
		default:
			return out;
	}
	(void)entry;
}

void cpu_io_page_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu		*cpu = arg;
	t_nes		*nes = (t_nes *)cpu->parent_device;
	t_ppu		*ppu = &nes->ppu;
	uint16_t	dma_src = 0;

	addr &= 0xFFF;
	if (addr > 0x17)
		return ;

	IOReg reg = addr & 0xFFF;
	switch (reg) {
		case (SQ1_VOL):
			return;
		case (SQ1_SWEEP):
			return;
		case (SQ1_LO):
			return;
		case (SQ1_HI):
			return;
		case (SQ2_VOL):
			return;
		case (SQ2_SWEEP):
			return;
		case (SQ2_LO):
			return;
		case (SW2_HI):
			return;
		case (TRI_LINEAR):
			return;
		case (UNUSED_09):
			return;
		case (TRI_LO):
			return;
		case (TRI_HI):
			return;
		case (NOISE_VOL):
			return;
		case (UNUSED_0D):
			return;
		case (NOISE_LO):
			return;
		case (NOISE_HI):
			return;
		case (DMC_FREQ):
			return;
		case (DMC_RAW):
			return;
		case (DMC_START):
			return;
		case (DMC_LEN):
			return;
		case (OAMDMA):
			cpu->cycle_events |= CYCLE_DMA;
			dma_src = val << 8;
			for (int i = 0; i < 256; i++)
				ppu->oam[ppu->oam_addr++] = read_byte(cpu, dma_src + i);
			// printf("DMA initiated! page: 0x%02X\n", val);
			return;
		case (SND_CHN):
			return;
		case (JOY1):
			nes->joy_strobe = (val & 0x01);
			if (nes->joy_strobe)
			{
				nes->joy_shift[0] = nes->joy_state[0];
				nes->joy_shift[1] = nes->joy_state[1];
			}
			return;
		case (JOY2):
			return;
	}
	(void)entry;
}

void	setup_nes_mappings(t_nes *nes)
{
	t_cpu *cpu = &nes->cpu;

	// Map work cpu work ram
	map_memory(cpu->pagetable, 0, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x800, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1000, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x1800, 2, nes->ram, NULL, passthrough_write);
	map_memory(cpu->pagetable, 0x4000, 1, NULL, cpu_io_page_read, cpu_io_page_write);

	// Map ppu ctrl registers
	map_memory(cpu->pagetable, 0x2000, 8, nes->ppu.registers, cpu_ppu_reg_read, cpu_ppu_reg_write);

	// Map ppu to vram based on mirroring
	map_ppu_nametables(&nes->ppu, nes->ppu.mirroring);
}

void	mapper_2_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = cpu->parent_device;
	t_cart	*cart = nes->cart;
	uint8_t bank_id = val & 0x0F;

	if (bank_id >= cart->prg_banks)
	{
		printf("Invalid bank id! Higher than number of banks\n");
		exit(2);
	}
	map_memory(nes->cpu.pagetable, 0x8000, 16, &cart->prg_rom[0x4000 * (bank_id)], NULL, mapper_2_write_handler);
	(void)entry;
	(void)addr;
}

void	setup_mapper_pagetables(t_nes *nes, t_cart *cart)
{
	setup_nes_mappings(nes);
	switch (cart->mapper_id) {
		case (0):
			map_memory(nes->cpu.pagetable, 0x8000, 16, cart->prg_rom, NULL, NULL);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[cart->prg_banks == 1 ? 0 : 0x4000], NULL, NULL);
			map_memory(nes->ppu.pagetable, 0x0000, 8, cart->chr_mem, NULL, NULL);
			break;
		case (2):
			map_memory(nes->cpu.pagetable, 0x8000, 16, cart->prg_rom, NULL, mapper_2_write_handler);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[0x4000 * (cart->prg_banks - 1)], NULL, mapper_2_write_handler);
			map_memory(nes->ppu.pagetable, 0x0000, 8, cart->chr_mem, NULL, passthrough_write);
			break;
		default:
			printf("Mapper not handled!\n");
			exit(1);
			break;
	}
}

void	nes_load_cartridge(t_nes *nes, t_cart *cart)
{
	nes->cart = cart;
	nes->ppu.mirroring = cart->mirroring;
	dprintf(nes->cpu.logfd, "prg: %04zX chr: %04zX mirroring: %d is_ram: %d\n", cart->prg_banks, cart->chr_banks, cart->mirroring, cart->chr_is_ram);
	setup_mapper_pagetables(nes, cart);
}
