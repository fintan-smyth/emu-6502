#include "audio_stream.h"
#include "menu.h"
#include "nes.h"
#include "emulator.h"
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

static inline void	scale_frame(float *scaling, int *x_offset, int *y_offset)
{
	int new_width = GetScreenWidth();
	int new_height = GetScreenHeight();

	float width_scale = (float)new_width / CANVAS_WIDTH;
	float height_scale = (float)new_height / CANVAS_HEIGHT;

	if (width_scale < height_scale)
	{
		*scaling = width_scale;
		*x_offset = 0;
		*y_offset = (new_height - (width_scale * CANVAS_HEIGHT)) / 2.0;
	}
	else
	{
		*scaling = height_scale;
		*x_offset = (new_width - (height_scale * CANVAS_WIDTH)) / 2.0;
		*y_offset = 0;
	}
}

#define FPS_WINDOW_SIZE 16

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

void update_frame(t_emulator *emu)
{
	t_nes *nes = &emu->nes;
	static float scaling = DEFAULT_SCALING;
	static int x_offset = 0;
	static int y_offset = 0;
	static double framesync = 0.0;

	double fps = calculate_fps();

	framesync += 1.0;
	if (framesync < nes->apu.fps_scale)
	{
		// printf("Frame skipped! %.0f\n", framesync);
		return ;
	}
	framesync -= nes->apu.fps_scale;

	handle_player_input(emu);
	UpdateTexture(emu->screen_tex, nes->ppu.screenbuf);
	if (IsWindowResized())
		scale_frame(&scaling, &x_offset, &y_offset);

	// while (ring_buffer_available(nes->apu.rb) > (RING_BUFFER_SIZE / 2))
	// 	usleep(1000);
	
	// nes->apu.drc_scale = calculate_drc_scale();
	nes->apu.drc_scale = calculate_drc_scale_alt();

	BeginDrawing();
	ClearBackground(BLACK);
	DrawTextureEx(emu->screen_tex, (Vector2){x_offset, y_offset}, 0, scaling, WHITE);
	// DrawFPS(10, 10);
	DrawText(TextFormat("%.0f", fps), 10, 10, 20, GREEN);
	DrawText(TextFormat("Buf lvl %4u", ring_buffer_available(&g_audio_buffer)), 10, 30, 20, GREEN);
	DrawText(TextFormat("Drc %f", nes->apu.drc_scale), 10, 50, 20,
		  nes->apu.drc_scale >= 0.99 && nes->apu.drc_scale <= 1.01 ? GREEN : RED);
	display_msg_queue(&emu->msg_queue, scaling);
	EndDrawing();
	nes->frames++;
}
