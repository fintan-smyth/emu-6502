#include "emulator.h"
#include "menu.h"
#include "nes.h"
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
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

void	display_msg_queue(t_msg **queue, double scaling)
{
	int		x_pos = 10;
	int		y_pos = (CANVAS_HEIGHT * scaling) - MSG_QUEUE_FONTSIZE;
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
	draw_text_outlined(cur->buf, x_pos, y_pos, MSG_QUEUE_FONTSIZE, WHITE, 2, BLACK);
	y_pos -= MSG_QUEUE_FONTSIZE;

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
			if (y_pos >= 70)
				// DrawText(cur->next->buf, x_pos, y_pos, 20, GREEN);
				draw_text_outlined(cur->buf, x_pos, y_pos, MSG_QUEUE_FONTSIZE, WHITE, 2, BLACK);
			y_pos -= MSG_QUEUE_FONTSIZE;
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

void resume_game(void *arg)
{
	t_emulator *emu = arg;
	emu->state = GAMEPLAY;
}

void exit_emulator(void *arg)
{
	t_emulator *emu = arg;
	emu->state = EXIT;
}

void menu_go_settings(void *arg)
{
	t_emulator *emu = arg;
	emu->menustate.menutype = SETTINGS;
	emu->menustate.selected_idx = 0;
}

void exit_settings(void *arg)
{
	t_emulator *emu = arg;
	emu->menustate.menutype = PAUSE;
	emu->menustate.selected_idx = 0;
}

t_menuitem pause_menu[] = {
	{ .text = "Resume", .type = BUTTON, .button.exec = resume_game },
	{ .text = "Settings", .type = BUTTON, .button.exec = menu_go_settings },
	{ .text = "Exit", .type = BUTTON, .button.exec = exit_emulator },
};

t_menuitem settings_menu[] = {
	{ .text = "Volume", .type = VAR_SLIDER, .slider.data_offset = offsetof(struct settings, volume),
		.slider.min_val = 0, .slider.max_val = 100, .slider.step = 1 },
	{ .text = "Target FPS", .type = VAR_SLIDER, .slider.data_offset = offsetof(struct settings, target_fps),
		.slider.min_val = 5, .slider.max_val = 500, .slider.step = 5 },
	{ .text = "Back", .type = BUTTON, .button.exec = exit_settings },
};

const int32_t PAUSE_MENU_COUNT = sizeof(pause_menu) / sizeof(t_menuitem);
const int32_t SETTINGS_MENU_COUNT = sizeof(settings_menu) / sizeof(t_menuitem);

void draw_menu(t_emulator *emu)
{
	t_menuitem	*menu;
	int32_t		item_count;
	void		*data_location;

	switch (emu->menustate.menutype) {
		case (PAUSE):
			menu = pause_menu;
			item_count = PAUSE_MENU_COUNT;
			data_location = NULL;
			break;
		case (SETTINGS):
			menu = settings_menu;
			item_count = SETTINGS_MENU_COUNT;
			data_location = &g_settings;
			break;
		default:
			return ;
	}

	int32_t	spacing = 20;
	int32_t	fontsize = 40;

	int32_t	start_y = (GetScreenHeight() - ((spacing + fontsize) * item_count)) / 2;

	char	textbuf[256];
	
	BeginDrawing();
	ClearBackground(BLACK);
	for (int i = 0; i < item_count; i++)
	{
		int32_t		*data;
		switch (menu[i].type) {
			case (BUTTON):
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
		int32_t x = GetScreenWidth() / 3;
		int32_t y = start_y + ((spacing + fontsize) * i);
		if (i == emu->menustate.selected_idx)
		{
			draw_text_outlined(">", x - (fontsize * 2), y,
				fontsize, col, 3, BLACK);
		}
		draw_text_outlined(textbuf, x, y, fontsize, col, 3, BLACK);
	}
	double fps = calculate_fps();
	DrawText(TextFormat("%.0f", fps), 10, 10, 20, GREEN);
	EndDrawing();
}

void	handle_menu_input(t_emulator *emu)
{
	t_menuitem	*menu;
	int32_t		item_count;
	void		*data_location;

	switch (emu->menustate.menutype) {
		case (PAUSE):
			menu = pause_menu;
			item_count = PAUSE_MENU_COUNT;
			data_location = NULL;
			break;
		case (SETTINGS):
			menu = settings_menu;
			item_count = SETTINGS_MENU_COUNT;
			data_location = &g_settings;
			break;
		default:
			return ;
	}

	if (IsKeyPressed(KEY_ESCAPE))
	{
		switch (emu->menustate.menutype) {
			case (PAUSE):
				emu->state = GAMEPLAY;
				break;
			case (SETTINGS):
				exit_settings(emu);
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
			if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
				*data = (*data + 1) % menu_item->selector.num_options;
			if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
				*data = (*data - 1 + menu_item->selector.num_options) % menu_item->selector.num_options;
			break;
		case (VAR_SLIDER):
			data = (int32_t *)((char *)data_location + menu_item->slider.data_offset);
			if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
			{
				*data = *data + menu_item->slider.step;
				if (*data > menu_item->slider.max_val)
					*data = menu_item->slider.max_val;
			}
			if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
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
