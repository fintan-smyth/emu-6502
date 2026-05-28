#include "emu6502.h"
#include "nes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void	uxrom_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = cpu->parent_device;
	t_cart	*cart = nes->cart;
	uint8_t bank_id = val & 0x0F;

	if (bank_id >= cart->prg_rom_banks)
	{
		printf("Invalid bank id! Higher than number of banks\n");
		exit(2);
	}
	map_memory(nes->cpu.pagetable, 0x8000, 16, &cart->prg_rom[0x4000 * (bank_id)], NULL, uxrom_write_handler);
	nes->mapper.uxrom.cur_prg_bank = bank_id;
	(void)entry;
	(void)addr;
}

void	mmc1_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val);

void	mmc1_update_mappings(t_nes *nes, t_cart *cart)
{
	uint8_t	mirror_mode = nes->mapper.mmc1.registers[MMC1_CTRL] & 0x3;
	uint8_t	prg_mode = (nes->mapper.mmc1.registers[MMC1_CTRL] >> 2) & 0x3;
	uint8_t	chr_mode = (nes->mapper.mmc1.registers[MMC1_CTRL] >> 4) & 0x1;
	uint8_t prg_bank = nes->mapper.mmc1.registers[MMC1_PRG] & 0x0F;
	uint8_t chr_bank0 = nes->mapper.mmc1.registers[MMC1_CHR0] & 0x1F;
	uint8_t chr_bank1 = nes->mapper.mmc1.registers[MMC1_CHR1] & 0x1F;
	// printf("\e[34;1m#######################\e[m\n");
	// printf("\e[34;1m# \e[mMMC1 MAPPING UPDATE \e[34;1m#\n");
	// printf("\e[34;1m#######################\e[m\n");
	// printf("MIR_MODE:\t%d PRG_MODE:\t%d CHR_MODE\t%d\n", mirror_mode, prg_mode, chr_mode);
	// printf("PRG_BANK:\t%d CHR0_BNK:\t%d CHR1_BNK\t%d\n", prg_bank, chr_bank0, chr_bank1);
	// printf("\n");


	switch (mirror_mode) {
		case (0):
			map_ppu_nametables(&nes->ppu, NULL, MIRROR_SINGLE_LOW);
			break;
		case (1):
			map_ppu_nametables(&nes->ppu, NULL, MIRROR_SINGLE_HIGH);
			break;
		case (2):
			map_ppu_nametables(&nes->ppu, NULL, MIRROR_VERTICAL);
			break;
		case (3):
			map_ppu_nametables(&nes->ppu, NULL, MIRROR_HORIZONTAL);
			break;
	}

	switch (prg_mode) {
		case (2):
			map_memory(nes->cpu.pagetable, 0x8000, 16, &cart->prg_rom[0x4000 * (cart->prg_rom_banks - 1)], NULL, mmc1_write_handler);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[0x4000 * prg_bank], NULL, mmc1_write_handler);
			break;
		case (3):
			map_memory(nes->cpu.pagetable, 0x8000, 16, &cart->prg_rom[0x4000 * prg_bank], NULL, mmc1_write_handler);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[0x4000 * (cart->prg_rom_banks - 1)], NULL, mmc1_write_handler);
			break;
		default:
			map_memory(nes->cpu.pagetable, 0x8000, 32, &cart->prg_rom[0x4000 * (prg_bank & 0x0E)], NULL, mmc1_write_handler);
			break;
	}


	void	(*write_handler)(struct pt_entry *, void *, uint16_t, uint8_t) = cart->chr_ram_banks > 0 ? passthrough_write : NULL;
	uint8_t	*memory = cart->chr_ram_banks > 0 ? cart->chr_ram : cart->chr_rom;

	switch (chr_mode) {
		case (0):
			map_memory(nes->ppu.pagetable, 0x0000, 8, &memory[0x1000 * (chr_bank0 & 0x0E)], NULL, write_handler);
			break;
		case (1):
			map_memory(nes->ppu.pagetable, 0x0000, 4, &memory[0x1000 * chr_bank0], NULL, write_handler);
			map_memory(nes->ppu.pagetable, 0x1000, 4, &memory[0x1000 * chr_bank1], NULL, write_handler);
			break;
	}
}

void	mmc1_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = cpu->parent_device;
	t_cart	*cart = nes->cart;

	catchup_with_cpu(nes);
	if (val & BIT_7)
	{
		// Reset shift reg
		nes->mapper.mmc1.shift = 0x10;
		// Set PRG bank mode 3
		nes->mapper.mmc1.registers[MMC1_CTRL] |= 0x0C;
		return ;
	}

	bool write5 = nes->mapper.mmc1.shift & 0x1;
	nes->mapper.mmc1.shift = (nes->mapper.mmc1.shift >> 1) | ((val & 0x1) << 4);
	if (write5)
	{
		uint8_t reg_select = (addr >> 13) & 0x3;
		nes->mapper.mmc1.registers[reg_select] = nes->mapper.mmc1.shift;
		nes->mapper.mmc1.shift = 0x10;
		mmc1_update_mappings(nes, cart);
	}
	(void)entry;
}

void	mmc3_clock_a12(t_nes *nes, bool a12_high, size_t cpu_cycle)
{
	if (!a12_high)
	{
		if (nes->mapper.mmc3.a12_state == true)
		{
			nes->mapper.mmc3.a12_low_cycle = cpu_cycle;
			nes->mapper.mmc3.a12_state = false;
		}
	}
	else if (nes->mapper.mmc3.a12_state == false)
	{
		nes->mapper.mmc3.a12_state = true;
		if (cpu_cycle - nes->mapper.mmc3.a12_low_cycle >= 3)
		{
			// printf("VALID CLOCK! Counter before: %d, Latch: %d\n", nes->mapper.mmc3.irq_counter, nes->mapper.mmc3.irq_latch);
			if (nes->mapper.mmc3.irq_counter == 0 || nes->mapper.mmc3.irq_reload)
			{
				nes->mapper.mmc3.irq_counter = nes->mapper.mmc3.irq_latch;
				nes->mapper.mmc3.irq_reload = false;
			}
			else
				nes->mapper.mmc3.irq_counter--;

			if (nes->mapper.mmc3.irq_counter == 0 && nes->mapper.mmc3.irq_enabled)
			{
				// printf("\e[33;1m### Mapper IRQ going high! ###\e[m\n");
				nes->mapper_irq= true;
			}
		}
	}
}

uint8_t	mmc3_ppu_read_handler(struct pt_entry *entry, void *arg, uint16_t addr)
{
	t_ppu *ppu = arg;
	t_nes *nes = ppu->nes;
	// printf("addr: 0x%04X ", addr);
	// fflush(stdout);
	uint8_t val = entry->memory[addr & 0x3FF];
	// printf(" val: 0x%02X\n", val);

	bool a12_high = (addr & 0x1000) != 0;

	// if (ppu->cycle > 264)
	{
		// printf("\e[36;1mADDR\e[m: 0x%04X ", addr);
		// printf("\e[31;1mCYCLE\e[m: %3d ", ppu->cycle);
		// printf("\e[32;1mSCANLINE\e[m: %3d ", ppu->scanline);
		// printf("\e[34;1mA12\e[m: %s\n", a12_high ? "\e[32;1mHIGH\e[m" : "\e[35;1mLOW\e[m");
	}

	size_t effective_cycles = ppu->total_cycles / 3;
	mmc3_clock_a12(nes, a12_high, effective_cycles);

	return val;
}

void	mmc3_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val);

void	mmc3_update_mappings(t_nes *nes, t_cart *cart)
{
	union mapper *mapper = &nes->mapper;

	if (mapper->mmc3.prg_mode == 0)
	{
		map_memory(nes->cpu.pagetable, 0x8000, 8, &cart->prg_rom[0x2000 * mapper->mmc3.registers[6]], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xA000, 8, &cart->prg_rom[0x2000 * mapper->mmc3.registers[7]], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xC000, 8, &cart->prg_rom[(0x4000 * cart->prg_rom_banks) - 0x4000], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xE000, 8, &cart->prg_rom[(0x4000 * cart->prg_rom_banks) - 0x2000], NULL, mmc3_write_handler);
	}
	else
	{
		map_memory(nes->cpu.pagetable, 0x8000, 8, &cart->prg_rom[(0x4000 * cart->prg_rom_banks) - 0x4000], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xA000, 8, &cart->prg_rom[0x2000 * mapper->mmc3.registers[7]], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xC000, 8, &cart->prg_rom[0x2000 * mapper->mmc3.registers[6]], NULL, mmc3_write_handler);
		map_memory(nes->cpu.pagetable, 0xE000, 8, &cart->prg_rom[(0x4000 * cart->prg_rom_banks) - 0x2000], NULL, mmc3_write_handler);
	}

	map_ppu_nametables(&nes->ppu, mmc3_ppu_read_handler, nes->ppu.mirroring);

	void	(*write_handler)(struct pt_entry *, void *, uint16_t, uint8_t) = cart->chr_ram_banks > 0 ? passthrough_write : NULL;
	uint8_t	*memory = cart->chr_ram_banks > 0 ? cart->chr_ram : cart->chr_rom;
	if (mapper->mmc3.chr_mode == 0)
	{
		map_memory(nes->ppu.pagetable, 0x0000, 2, &memory[0x400 * (mapper->mmc3.registers[0] & ~0x01)], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x0800, 2, &memory[0x400 * (mapper->mmc3.registers[1] & ~0x01)], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1000, 1, &memory[0x400 * mapper->mmc3.registers[2]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1400, 1, &memory[0x400 * mapper->mmc3.registers[3]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1800, 1, &memory[0x400 * mapper->mmc3.registers[4]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1C00, 1, &memory[0x400 * mapper->mmc3.registers[5]], mmc3_ppu_read_handler, write_handler);
	}
	else
	{
		map_memory(nes->ppu.pagetable, 0x0000, 1, &memory[0x400 * mapper->mmc3.registers[2]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x0400, 1, &memory[0x400 * mapper->mmc3.registers[3]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x0800, 1, &memory[0x400 * mapper->mmc3.registers[4]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x0C00, 1, &memory[0x400 * mapper->mmc3.registers[5]], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1000, 2, &memory[0x400 * (mapper->mmc3.registers[0] & ~0x01)], mmc3_ppu_read_handler, write_handler);
		map_memory(nes->ppu.pagetable, 0x1800, 2, &memory[0x400 * (mapper->mmc3.registers[1] & ~0x01)], mmc3_ppu_read_handler, write_handler);
	}
}

void	mmc3_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu	*cpu = arg;
	t_nes	*nes = cpu->parent_device;
	t_cart	*cart = nes->cart;

	bool	even_addr = (addr & 0x1) == 0;
	uint8_t	bank = (addr >> 13) & 0x03;

	catchup_with_cpu(nes);
	switch (bank) {
		case (0):
			if (even_addr)
			{
				nes->mapper.mmc3.target_reg = val & 0x07;
				nes->mapper.mmc3.prg_mode = (val & 0x40) != 0;
				nes->mapper.mmc3.chr_mode = (val & 0x80) != 0;
			}
			else
			{
				if (nes->mapper.mmc3.target_reg >= 6)
					val &= 0x3F;
				nes->mapper.mmc3.registers[nes->mapper.mmc3.target_reg] = val;
			}
			mmc3_update_mappings(nes, cart);
			break;
		case (1):
			if (even_addr)
			{
				printf("Setting mirroring...\n");
				nes->ppu.mirroring = (val & 1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL;
				// nes->ppu.mirroring = (val & 1) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;
				map_ppu_nametables(&nes->ppu, mmc3_ppu_read_handler, nes->ppu.mirroring);
			}
			else
			{
				; // RAM protect: not important
			}
			break;
		case (2):
			if (even_addr)
				nes->mapper.mmc3.irq_latch = val;
			else
			{
				nes->mapper.mmc3.irq_counter = 0;
				nes->mapper.mmc3.irq_reload = true;
			}
			break;
		case (3):
			if (even_addr)
			{
				nes->mapper.mmc3.irq_enabled = false;
				nes->mapper_irq = false;
				// printf("Acknowledging mapper IRQ\n");
			}
			else
				nes->mapper.mmc3.irq_enabled = true;
			break;
	}
}

void	init_mapper_mmap(t_nes *nes, t_cart *cart)
{
	nes->mapper_irq = false;
	switch (cart->mapper_id) {
		case (MMC1):
			nes->mapper.mmc1.shift = 0x10;
			nes->mapper.mmc1.registers[MMC1_CTRL] = 0x0C;
			break;
		case (MMC3):
			nes->mapper.mmc3.a12_low_cycle = 0;
			break;
		default:
			break;
	}
}

void	refresh_mapper_mmap(t_nes *nes, t_cart *cart)
{
	switch (cart->mapper_id) {
		case (NROM):
			map_memory(nes->cpu.pagetable, 0x8000, 16, cart->prg_rom, NULL, NULL);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[cart->prg_rom_banks == 1 ? 0 : 0x4000], NULL, NULL);
			break;
		case (MMC1):
			if (cart->prg_ram_banks > 0)
				map_memory(nes->cpu.pagetable, 0x6000, 8, cart->prg_ram, NULL, passthrough_write);
			mmc1_update_mappings(nes, cart);
			break;
		case (UxROM):
			map_memory(nes->cpu.pagetable, 0x8000, 16, &cart->prg_rom[0x4000 * nes->mapper.uxrom.cur_prg_bank], NULL, uxrom_write_handler);
			map_memory(nes->cpu.pagetable, 0xC000, 16, &cart->prg_rom[0x4000 * (cart->prg_rom_banks - 1)], NULL, uxrom_write_handler);
			break;
		case (MMC3):
			if (cart->prg_ram_banks > 0)
				map_memory(nes->cpu.pagetable, 0x6000, 8, cart->prg_ram, NULL, passthrough_write);
			mmc3_update_mappings(nes, cart);
			break;
		default:
			printf("Mapper not handled! :^(\n");
			exit(1);
			break;
	}
}
