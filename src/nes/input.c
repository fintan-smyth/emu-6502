#include "emulator.h"
#include <raylib.h>

void	handle_player_input(t_emulator *emu)
{
	t_nes *nes = &emu->nes;
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
		// nes->apu.fps_scale = 2;
		g_settings.target_fps = 120;
	}
	if (IsKeyReleased(KEY_LEFT_SHIFT))
	{
		// nes->apu.fps_scale = 1;
		g_settings.target_fps = 60;
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
		getchar();
	}
	if (IsKeyPressed(KEY_V))
	{
		TakeScreenshot("screenshot.png");
	}
	if (IsKeyPressed(KEY_LEFT_BRACKET))
	{
		g_settings.target_fps -= 10;
		if (g_settings.target_fps < 0)
			g_settings.target_fps = 0;
	}
	if (IsKeyPressed(KEY_RIGHT_BRACKET))
	{
		g_settings.target_fps += 10;
	}

	if (IsKeyPressed(KEY_X))
		save_game(emu, QUICKSAVE_SLOT_NUM);
	if (IsKeyPressed(KEY_Z))
		load_save_game(emu, QUICKSAVE_SLOT_NUM);
	if (IsKeyPressed(KEY_B))
	{
		printf("mirroring: %d\n", nes->ppu.mirroring);
		dump_ppu_memory(&nes->ppu);
		exit(0);
	}
	if (IsKeyPressed(KEY_ESCAPE))
	{
		emu->state = STATE_MENU;
		emu->menustate.menutype = MENU_PAUSE;
		emu->menustate.selected_idx = 0;
		set_input_hook(emu, handle_menu_input);
	}
	nes->apu.fps_scale = g_settings.target_fps / 60.0;
}
