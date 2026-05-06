#include "nes.h"
#include <stdint.h>

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
			map_ppu_nametables(&nes->ppu, MIRROR_SINGLE_LOW);
			break;
		case (1):
			map_ppu_nametables(&nes->ppu, MIRROR_SINGLE_HIGH);
			break;
		case (2):
			map_ppu_nametables(&nes->ppu, MIRROR_VERTICAL);
			break;
		case (3):
			map_ppu_nametables(&nes->ppu, MIRROR_HORIZONTAL);
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

void	init_mapper_mmap(t_nes *nes, t_cart *cart)
{
	switch (cart->mapper_id) {
		case (MMC1):
			nes->mapper.mmc1.shift = 0x10;
			nes->mapper.mmc1.registers[MMC1_CTRL] = 0x0C;
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
		default:
			printf("Mapper not handled!\n");
			exit(1);
			break;
	}
}
