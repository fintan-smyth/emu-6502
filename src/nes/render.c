#include "audio_stream.h"
#include "menu.h"
#include "nes.h"
#include "emulator.h"
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <unistd.h>

extern const Color palette_alt[];

void	draw_pixel(t_ppu *ppu, int x, int y, uint32_t col)
{
	// printf("drawing (%d, %d) col: 0x%08X\n", x, y, col);
	ppu->screenbuf[y * CANVAS_WIDTH + x] = col;
}

// void	draw_palette(void)
// {
// 	BeginDrawing();
// 	ClearBackground(BLACK);
// 	for (int i = 0; i < 64; i++)
// 	{
// 		int x = (i % 16) * 16;
// 		int y = (i / 16) * 16;
// 		DrawRectangle(x, y, 16, 16, GetColor(palette[i]));
// 	}
// 	EndDrawing();
// }

void	draw_text_outlined(const char *str, int x, int y, int fontsize, Color col, int out_size, Color out_col)
{
	for (int dx = x - out_size; dx <= x + out_size; dx++)
	{
		for (int dy = y - out_size; dy <= y + out_size; dy++)
			DrawText(str, dx, dy, fontsize, out_col);
	}
	DrawText(str, x, y, fontsize, col);
}

static inline float	scale_frame(Rectangle *rec)
{
	int new_width = GetScreenWidth();
	int new_height = GetScreenHeight();

	float width_scale = (float)new_width / CANVAS_WIDTH;
	float height_scale = (float)new_height / CANVAS_HEIGHT;
	float scaling;

	if (width_scale < height_scale)
	{
		scaling = width_scale;
		rec->x = 0;
		rec->y = (new_height - (width_scale * CANVAS_HEIGHT)) / 2.0;
	}
	else
	{
		scaling = height_scale;
		rec->x = (new_width - (height_scale * CANVAS_WIDTH)) / 2.0;
		rec->y = 0;
	}
	rec->width = CANVAS_WIDTH * scaling;
	rec->height = CANVAS_HEIGHT * scaling;

	return scaling;
}

static inline float get_auto_scale_factor(void)
{
	int new_width = GetScreenWidth();
	int new_height = GetScreenHeight();

	float width_scale = (float)new_width / CANVAS_WIDTH;
	float height_scale = (float)new_height / CANVAS_HEIGHT;

	return width_scale < height_scale ? width_scale : height_scale;
}

static inline void centre_window(Rectangle *rec, float scaling)
{
	int screen_width = GetScreenWidth();
	int screen_height = GetScreenHeight();

	// rec->x = (screen_width - (scaling * CANVAS_WIDTH)) / 2.0;
	// rec->y = (screen_height - (scaling * CANVAS_HEIGHT)) / 2.0;
	// rec->width = CANVAS_WIDTH * scaling;
	// rec->height = CANVAS_HEIGHT * scaling;
	rec->x = (int)((screen_width - (scaling * CANVAS_WIDTH)) / 2.0);
	rec->y = (int)((screen_height - (scaling * CANVAS_HEIGHT)) / 2.0);
	rec->width = (int)(CANVAS_WIDTH * scaling);
	rec->height = (int)(CANVAS_HEIGHT * scaling);
}

#define FPS_WINDOW_SIZE 32

double	calculate_fps(void)
{
	static double	rb[256] = {};
	static uint8_t	tail = 0;
	static uint8_t	head = FPS_WINDOW_SIZE;

	rb[head] = GetTime();

	double fps = (double)FPS_WINDOW_SIZE / (rb[head] - rb[tail]);

	head++;
	tail++;
	return fps;
}

void	draw_ui_elements(t_emulator *emu)
{
	BeginTextureMode(emu->ui_tex);
	ClearBackground(BLANK);
	display_msg_queue(&emu->msg_queue);
	if (emu->state == STATE_MENU)
		draw_menu(emu);
	EndTextureMode();
}

void draw_frame_scaled(t_emulator *emu)
{
	t_nes *nes = &emu->nes;
	const Rectangle src = {
		.x = 0,
		.y = 0,
		.width = CANVAS_WIDTH,
		.height = CANVAS_HEIGHT,
	};
	const Rectangle flipped_src = {
		.x = 0,
		.y = 0,
		.width = CANVAS_WIDTH,
		.height = -CANVAS_HEIGHT,
	};
	static Rectangle dest = {
		.x = 0,
		.y = 0,
		.width = CANVAS_WIDTH,
		.height = CANVAS_HEIGHT,
	};

	static double framesync = 0.0;
	double fps = calculate_fps();

	framesync += 1.0;
	if (framesync < nes->apu.fps_scale)
	{
		// printf("Frame skipped! %.0f\n", framesync);
		return ;
	}
	framesync -= nes->apu.fps_scale;

	static int32_t old_scaling_mode = SCALING_AUTO;
	if (IsWindowResized() || g_settings.scaling != old_scaling_mode)
	{
		float scaling = (g_settings.scaling == SCALING_AUTO)
			? get_auto_scale_factor()
			: (float)g_settings.scaling;
		centre_window(&dest, scaling);
		old_scaling_mode = g_settings.scaling;
	}

	// nes->apu.drc_scale = calculate_drc_scale();
	nes->apu.drc_scale = calculate_drc_scale_alt();

	emu->input_hook(emu);
	UpdateTexture(emu->nes_tex, emu->nes.ppu.screenbuf);

	BeginDrawing();
	ClearBackground(DARKGREEN);
	DrawTexturePro(emu->nes_tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
	DrawTexturePro(emu->ui_tex.texture, flipped_src, dest, (Vector2){0, 0}, 0.0f, WHITE);
	DrawText(TextFormat("%.1f", fps), 10, 10, 20, GREEN);
	DrawText(TextFormat("Buf lvl %4u", ring_buffer_available(&g_audio_buffer)), 10, 30, 20, GREEN);
	DrawText(TextFormat("Drc %f", nes->apu.drc_scale), 10, 50, 20,
		  nes->apu.drc_scale >= 0.99 && nes->apu.drc_scale <= 1.01 ? GREEN : RED);
	DrawText(TextFormat("dest.x %.3f", dest.x), 10, 70, 20, GREEN);
	DrawText(TextFormat("dest.y %.3f", dest.y), 10, 90, 20, GREEN);
	DrawText(TextFormat("dest.w %.3f", dest.width), 10, 110, 20, GREEN);
	DrawText(TextFormat("dest.h %.3f", dest.height), 10, 130, 20, GREEN);
	EndDrawing();
	nes->frames++;
}
