#include "nes.h"
#include "emu6502.h"
#include <stdint.h>

void	init_nes(t_nes *nes)
{
	nes->cpu.parent_device = nes;
	nes->ppu.nes = nes;
	nes->ppu.nmi_pin = &nes->cpu.nmi_pending;
	nes->ppu.screenbuf = calloc(WIN_HEIGHT * WIN_WIDTH, sizeof(uint32_t));
	nes->settings.fps = 60;
}

uint8_t nes_step(t_nes *nes)
{
	uint8_t cycles = cpu_step(&nes->cpu);
	ppu_tick_for(&nes->ppu, cycles * 3);

	return cycles;
}

uint64_t nes_run_for(t_nes *nes, uint64_t n_cycles)
{
	uint64_t cycles = 0;

	while (cycles < n_cycles)
		cycles += nes_step(nes);

	return cycles - n_cycles;
}
