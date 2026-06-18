#include "emu6502.h"
#include "nes.h"
#include <raylib.h>

void	handle_player_input(t_nes *nes)
{
	nes->joy_state[0] = 0;
	nes->joy_state[1] = 0;

	if (IsKeyDown(KEY_K))
		nes->joy_state[0] |= JOY_A;
	if (IsKeyDown(KEY_J))
		nes->joy_state[0] |= JOY_B;
	if (IsKeyDown(KEY_U))
		nes->joy_state[0] |= JOY_SELECT;
	if (IsKeyDown(KEY_I))
		nes->joy_state[0] |= JOY_START;
	if (IsKeyDown(KEY_W))
		nes->joy_state[0] |= JOY_UP;
	if (IsKeyDown(KEY_S))
		nes->joy_state[0] |= JOY_DOWN;
	if (IsKeyDown(KEY_A))
		nes->joy_state[0] |= JOY_LEFT;
	if (IsKeyDown(KEY_D))
		nes->joy_state[0] |= JOY_RIGHT;

	if (IsKeyPressed(KEY_LEFT_SHIFT))
	{
		nes->apu.fps_scale = 2;
		SetTargetFPS(nes->settings.fps * 2);
	}
	if (IsKeyReleased(KEY_LEFT_SHIFT))
	{
		nes->apu.fps_scale = 1;
		SetTargetFPS(nes->settings.fps);
	}

	if (IsKeyPressed(KEY_R))
	{
		nes->cpu.sp = 0xFF;
		nes->cpu.cycles = 0;
		// exec_hardware_interrupt(&nes->cpu, 0xFFFC);
		nes->cpu.pending_interrupt = INT_RESET;
	}
	if (IsKeyPressed(KEY_P))
	{
		nes->settings.pattern_palette = (nes->settings.pattern_palette + 1) % 4;
	}
	if (IsKeyPressed(KEY_LEFT_BRACKET))
	{
		nes->settings.fps -= 10;
		if (nes->settings.fps < 0)
			nes->settings.fps = 0;
		nes->apu.fps_scale = nes->settings.fps / 60.0;
		SetTargetFPS(nes->settings.fps);
	}
	if (IsKeyPressed(KEY_RIGHT_BRACKET))
	{
		nes->settings.fps += 10;
		if (nes->settings.fps > 240)
			nes->settings.fps = 240;
		nes->apu.fps_scale = nes->settings.fps / 60.0;
		SetTargetFPS(nes->settings.fps);
	}

	if (IsKeyPressed(KEY_X))
		save_game(nes);
	if (IsKeyPressed(KEY_Z))
		load_save_game(nes);
	if (IsKeyPressed(KEY_B))
	{
		printf("mirroring: %d\n", nes->ppu.mirroring);
		dump_ppu_memory(&nes->ppu);
		exit(0);
	}
}
