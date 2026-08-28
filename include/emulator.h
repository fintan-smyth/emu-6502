#ifndef EMULATOR_H
# define EMULATOR_H

#include "nes.h"
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include "menu.h"

#define XSTR(x) STR(x)
#define STR(x) #x

#define MAX_SAVE_SLOTS 10
#define QUICKSAVE_SLOT_NUM MAX_SAVE_SLOTS

typedef enum
{
	STATE_GAMEPLAY,
	STATE_MENU,
	STATE_EXIT,
}	EmuState;

enum
{
	SCALING_AUTO,
	SCALING_X1,
	SCALING_X2,
	SCALING_X3,
	SCALING_X4,
	SCALING_MAX,
};

struct settings
{
	int32_t	target_fps;
	int32_t	volume;
	int32_t	scaling;
};

extern struct settings g_settings;

typedef struct s_emulator
{
	t_nes			nes;
	AudioStream		stream;
	Texture2D		nes_tex;
	RenderTexture2D	ui_tex;
	t_msg			*msg_queue;
	EmuState		state;
	struct {
		int32_t			selected_idx;
		MenuType		menutype;
	}				menustate;
	void			(*input_hook)(t_emulator *);
}	t_emulator;

void	init_emulator(t_emulator *emu);
void	run_emulator_frame(t_emulator *emu);
void	set_input_hook(t_emulator *emu, void (*hook)(t_emulator *));

size_t	construct_save_folder_path(char *buf, const char *rom_title, size_t bufsize);
size_t	construct_save_file_path(char *buf, const char *rom_title, uint8_t slot_num, size_t bufsize);
void	ensure_dir_exists(const char *path);
void	ensure_save_dir_exists(const char *rom_title);
void	save_game(t_emulator *emu, uint8_t slot_num);
bool	load_save_game(t_emulator *emu, uint8_t slot_num);
void	handle_player_input(t_emulator *emu);

double	calculate_fps(void);
void	draw_text_outlined(const char *str, int x, int y, int fontsize, Color col, int out_size, Color out_col);

void	draw_ui_elements(t_emulator *emu);
void	draw_frame_scaled(t_emulator *emu);

#endif // EMULATOR_H
