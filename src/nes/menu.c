#include "emulator.h"
#include "menu.h"
#include "nes.h"
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

t_msg	*new_msg(const char *str, double display_time)
{
	t_msg *new = malloc(sizeof(*new));

	strncpy(new->buf, str, 255);
	new->display_time = display_time;
	new->submitted_time = GetTime();
	new->next = NULL;

	return new;
}

void	enqueue_msg(t_msg **queue, t_msg *msg)
{
	if (queue == NULL || msg == NULL)
		return ;

	msg->next = *queue;
	*queue = msg;
}

void	display_msg_queue(t_msg **queue)
{
	const int fontsize = 10;
	const int spacing = 12;
	int		x_pos = 5;
	int		y_pos = CANVAS_HEIGHT - spacing;
	double	time = GetTime();

	t_msg *cur = *queue;
	if (cur == NULL)
		return ;
	while (cur->submitted_time + cur->display_time < time)
	{
		t_msg *tmp = cur->next;
		free(cur);
		cur = tmp;
		*queue = cur;
		if (cur == NULL)
			return ;
	}

	// DrawText(cur->buf, x_pos, y_pos, 20, GREEN);
	draw_text_outlined(cur->buf, x_pos, y_pos, fontsize, WHITE, 1, BLACK);
	y_pos -= spacing;

	while (cur->next != NULL)
	{
		if (cur->next->submitted_time + cur->next->display_time < time)
		{
			t_msg *tmp = cur->next;
			cur->next = tmp->next;
			free(tmp);
		}
		else
		{
			if (y_pos >= 50)
				draw_text_outlined(cur->next->buf, x_pos, y_pos, fontsize, WHITE, 1, BLACK);
			y_pos -= spacing;
			cur = cur->next;
		}
	}
}

void	clear_msg_queue(t_msg **queue)
{
	if (queue == NULL)
		return ;

	t_msg *tmp = *queue;
	while (tmp != NULL)
	{
		*queue = tmp->next;
		free(tmp);
		tmp = *queue;
	}
}

void resume_game(t_emulator *emu)
{
	emu->state = STATE_GAMEPLAY;
	set_input_hook(emu, handle_player_input);
}

void exit_emulator(t_emulator *emu)
{
	emu->state = STATE_EXIT;
}

void menu_go_settings(t_emulator *emu)
{
	emu->menustate.menutype = MENU_SETTINGS;
	emu->menustate.selected_idx = 0;
}

void menu_go_save(t_emulator *emu)
{
	emu->menustate.menutype = MENU_SAVE;
	emu->menustate.selected_idx = 0;
}

void menu_go_load(t_emulator *emu)
{
	emu->menustate.menutype = MENU_LOAD;
	emu->menustate.selected_idx = 0;
}

void menu_go_pause(t_emulator *emu)
{
	emu->menustate.menutype = MENU_PAUSE;
	emu->menustate.selected_idx = 0;
}

void menu_save_game(t_emulator *emu)
{
	uint8_t slot_num = emu->menustate.selected_idx;
	save_game(emu, slot_num);
}

void menu_load_game(t_emulator *emu)
{
	uint8_t slot_num = emu->menustate.selected_idx;
	if (load_save_game(emu, slot_num))
		resume_game(emu);
}

void format_save_slot(t_emulator *emu, int32_t entry_num, char *buf)
{
	struct stat statbuf;
	char save_path[256];

	construct_save_file_path(save_path, emu->nes.cart->title, entry_num, 256);
	if (stat(save_path, &statbuf) == 0)
	{
		time_t mtime = statbuf.st_mtime;

		struct tm tm_info;
		if (localtime_r(&mtime, &tm_info) == NULL) {
			printf("localtime_r failed");
			exit(EXIT_FAILURE);
		}

		char time_str[64];
		
		size_t written = strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);
		if (written == 0) {
			printf("Buffer too small for strftime\n");
			exit(EXIT_FAILURE);
		}
		snprintf(buf, 256, "Slot %02u    %s", entry_num, time_str);
	}
	else
		snprintf(buf, 256, "Slot %02u    <empty>", entry_num);
}

t_menuitem pause_menu[] = {
	{ .text = "Resume", .type = BUTTON, .button.exec = resume_game },
	{ .text = "Save", .type = BUTTON, .button.exec = menu_go_save },
	{ .text = "Load", .type = BUTTON, .button.exec = menu_go_load },
	{ .text = "Settings", .type = BUTTON, .button.exec = menu_go_settings },
	{ .text = "Exit", .type = BUTTON, .button.exec = exit_emulator },
};

t_menuitem save_menu[] = {
	{ .text = "Slot 0", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 1", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 2", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 3", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 4", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 5", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 6", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 7", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 8", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Slot 9", .type = BUTTON, .button.exec = menu_save_game, .button.format = format_save_slot },
	{ .text = "Back", .type = BUTTON, .button.exec = menu_go_pause },
};

t_menuitem load_menu[] = {
	{ .text = "Slot 00", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 01", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 02", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 03", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 04", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 05", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 06", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 07", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 08", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Slot 09", .type = BUTTON, .button.exec = menu_load_game, .button.format = format_save_slot },
	{ .text = "Back", .type = BUTTON, .button.exec = menu_go_pause },
};

const char *scaling_strs[] = {
	"Auto",
	"x1",
	"x2",
	"x3",
	"x4",
};

t_menuitem settings_menu[] = {
	{ .text = "Volume", .type = VAR_SLIDER, .slider.data_offset = offsetof(struct settings, volume),
		.slider.min_val = 0, .slider.max_val = 100, .slider.step = 5 },
	{ .text = "Target FPS", .type = VAR_SLIDER, .slider.data_offset = offsetof(struct settings, target_fps),
		.slider.min_val = 5, .slider.max_val = 500, .slider.step = 10 },
	{ .text = "Scaling", .type = VAR_SELECTOR, .selector.data_offset = offsetof(struct settings, scaling),
		.selector.num_options = SCALING_MAX, .selector.options = scaling_strs },
	{ .text = "Back", .type = BUTTON, .button.exec = menu_go_pause },
};

const int32_t PAUSE_MENU_COUNT = sizeof(pause_menu) / sizeof(t_menuitem);
const int32_t SETTINGS_MENU_COUNT = sizeof(settings_menu) / sizeof(t_menuitem);
const int32_t SAVE_MENU_COUNT = sizeof(save_menu) / sizeof(t_menuitem);
const int32_t LOAD_MENU_COUNT = sizeof(load_menu) / sizeof(t_menuitem);

void draw_menu(t_emulator *emu)
{
	t_menuitem	*menu;
	int32_t		item_count;
	void		*data_location;

	switch (emu->menustate.menutype) {
		case (MENU_PAUSE):
			menu = pause_menu;
			item_count = PAUSE_MENU_COUNT;
			data_location = NULL;
			break;
		case (MENU_SETTINGS):
			menu = settings_menu;
			item_count = SETTINGS_MENU_COUNT;
			data_location = &g_settings;
			break;
		case (MENU_SAVE):
			menu = save_menu;
			item_count = SAVE_MENU_COUNT;
			data_location = NULL;
			break;
		case (MENU_LOAD):
			menu = load_menu;
			item_count = LOAD_MENU_COUNT;
			data_location = NULL;
			break;
		default:
			return ;
	}

	int32_t	spacing = 5;
	int32_t	fontsize = 10;

	int32_t	start_y = (CANVAS_HEIGHT - ((spacing + fontsize) * item_count)) / 2;

	char	textbuf[256];
	
	// BeginDrawing();
	DrawRectangle(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, Fade(BLACK, 0.6f));
	for (int i = 0; i < item_count; i++)
	{
		int32_t		*data;
		switch (menu[i].type) {
			case (BUTTON):
				if (menu[i].button.format)
					menu[i].button.format(emu, i, textbuf);
				else
					snprintf(textbuf, 256, "%s", menu[i].text);
				break;
			case (VAR_SELECTOR):
				data = (int32_t *)((char *)data_location + menu[i].selector.data_offset);
				snprintf(textbuf, 256, "%s: < %s >", menu[i].text, menu[i].selector.options[*data]);
				break;
			case (VAR_SLIDER):
				data = (int32_t *)((char *)data_location + menu[i].slider.data_offset);
				snprintf(textbuf, 256, "%s: < %d >", menu[i].text, *data);
				break;
		}

		Color col = (i == emu->menustate.selected_idx) ? GREEN : WHITE;

		// int32_t x = (GetScreenWidth() - (strlen(textbuf) * fontsize)) / 2;
		int32_t x = CANVAS_WIDTH / 3;
		int32_t y = start_y + ((spacing + fontsize) * i);
		if (i == emu->menustate.selected_idx)
		{
			draw_text_outlined(">", x - (fontsize * 2), y,
				fontsize, col, 1, BLACK);
		}
		draw_text_outlined(textbuf, x, y, fontsize, col, 1, BLACK);
	}
}

void	handle_menu_input(t_emulator *emu)
{
	t_menuitem	*menu;
	int32_t		item_count;
	void		*data_location;

	switch (emu->menustate.menutype) {
		case (MENU_PAUSE):
			menu = pause_menu;
			item_count = PAUSE_MENU_COUNT;
			data_location = NULL;
			break;
		case (MENU_SETTINGS):
			menu = settings_menu;
			item_count = SETTINGS_MENU_COUNT;
			data_location = &g_settings;
			break;
		case (MENU_SAVE):
			menu = save_menu;
			item_count = SAVE_MENU_COUNT;
			data_location = NULL;
			break;
		case (MENU_LOAD):
			menu = load_menu;
			item_count = LOAD_MENU_COUNT;
			data_location = NULL;
			break;
		default:
			return ;
	}

	if (IsKeyPressed(KEY_ESCAPE))
	{
		switch (emu->menustate.menutype) {
			case (MENU_PAUSE):
				resume_game(emu);
				break;
			case (MENU_SETTINGS):
				menu_go_pause(emu);
				break;
			case (MENU_SAVE):
				menu_go_pause(emu);
				break;
			case (MENU_LOAD):
				menu_go_pause(emu);
				break;
		}
		return ;
	}

	if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
		emu->menustate.selected_idx = (emu->menustate.selected_idx + 1) % item_count;
	if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
		emu->menustate.selected_idx = (emu->menustate.selected_idx - 1 + item_count) % item_count;

	t_menuitem *menu_item = &menu[emu->menustate.selected_idx];
	int32_t		*data;

	switch (menu_item->type) {
		case (BUTTON):
			if (IsKeyPressed(KEY_K) || IsKeyPressed(KEY_U) || IsKeyPressed(KEY_I))
				menu_item->button.exec(emu);
			break;
		case (VAR_SELECTOR):
			data = (int32_t *)((char *)data_location + menu_item->selector.data_offset);
			if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
				*data = (*data + 1) % menu_item->selector.num_options;
			if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
				*data = (*data - 1 + menu_item->selector.num_options) % menu_item->selector.num_options;
			break;
		case (VAR_SLIDER):
			data = (int32_t *)((char *)data_location + menu_item->slider.data_offset);
			if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
			{
				*data = *data + menu_item->slider.step;
				if (*data > menu_item->slider.max_val)
					*data = menu_item->slider.max_val;
			}
			if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
			{
				*data = *data - menu_item->slider.step;
				if (*data < menu_item->slider.min_val)
					*data = menu_item->slider.min_val;
			}
			break;
		default:
			break;
	}
}
