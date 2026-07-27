#ifndef EMULATOR_H
# define EMULATOR_H

#include "nes.h"
#include <raylib.h>
#include "menu.h"

typedef enum
{
	GAMEPLAY,
	MENU,
	EXIT,
}	EmuState;

struct settings
{
	int32_t	target_fps;
	int32_t	volume;
};

extern struct settings g_settings;

typedef struct s_emulator
{
	t_nes		nes;
	AudioStream	stream;
	Texture2D	screen_tex;
	t_msg		*msg_queue;
	EmuState	state;
	struct {
		int32_t		selected_idx;
		MenuType	menutype;
	} menustate;
}	t_emulator;

void	init_emulator(t_emulator *emu);
void	update_frame(t_emulator *emu);
void	run_emulator_frame(t_emulator *emu);

void	save_game(t_emulator *emu);
void	load_save_game(t_emulator *emu);
void	handle_player_input(t_emulator *emu);

double	calculate_fps(void);
void	draw_text_outlined(const char *str, int x, int y, int fontsize, Color col, int out_size, Color out_col);

#endif // EMULATOR_H
