#include "nes.h"
#include <raylib.h>
#include <stdint.h>

extern const Color palette_alt[];

void init_raylib(t_nes *nes)
{
	InitWindow(WIN_WIDTH * SCALING, WIN_HEIGHT * SCALING, "emu6502");
	SetTargetFPS(nes->settings.fps);
	Image blankImage = GenImageColor(WIN_WIDTH, WIN_HEIGHT, BLANK);
    nes->ppu.screen_tex = LoadTextureFromImage(blankImage);
	nes->ppu.screenbuf = calloc(WIN_HEIGHT * WIN_WIDTH, sizeof(uint32_t));
    UnloadImage(blankImage);
}

void	draw_pixel(t_ppu *ppu, int x, int y, uint32_t col)
{
	// printf("drawing (%d, %d) col: 0x%08X\n", x, y, col);
	ppu->screenbuf[y * WIN_WIDTH + x] = col;
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

void	draw_tile(t_ppu *ppu, uint8_t table, uint8_t tile_id, int x, int y)
{
	uint8_t 		tile[64];
	const uint8_t	palette_id = ppu->nes->settings.pattern_palette;

	for (int i = 0; i < 8; i++)
		fetch_tile_row(ppu, &tile[i * 8], table, tile_id, i);

	for (int i = 0; i < 64; i++)
	{
		uint8_t col_index = ppu_read(ppu, 0x3F00 | (palette_id << 2) | tile[i]);
		Color col = palette_alt[col_index & 0x3F];
		draw_pixel(ppu, x + (i % 8), y + (i / 8), *(uint32_t *)&col);
		// printf("pattern table col: 0x%08X pix: %d\n", *(uint32_t *)&col, tile[i]);
	}
}

void	draw_pattern_table(t_ppu *ppu, uint8_t table, int posX, int posY)
{
	for (int i = 0; i < 256; i++)
	{
		int x = (i % 16) * 8;
		int y = (i / 16) * 8;
		draw_tile(ppu, table, i, x + posX, y + posY);
	}
}

void update_frame(t_nes *nes)
{
	t_ppu *ppu = &nes->ppu;

	handle_player_input(nes);
	// printf("updating frame...\n");
	draw_pattern_table(ppu, 0, 0, 240);
	draw_pattern_table(ppu, 1, 128, 240);
	UpdateTexture(ppu->screen_tex, ppu->screenbuf);
	BeginDrawing();
	ClearBackground(BLACK);
	// BeginBlendMode(BLEND_ALPHA);
	DrawTextureEx(ppu->screen_tex, (Vector2){0, 0}, 0, SCALING, WHITE);
	DrawFPS(10, 10);
	EndBlendMode();
	EndDrawing();
}
