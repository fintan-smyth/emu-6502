#ifndef NES_H
# define NES_H

#include "emu6502.h"
#include <raylib.h>
#include <stdint.h>
#include <sys/types.h>

#define MIRROR_HORIZONTAL 0
#define MIRROR_VERTICAL 1

#define WIN_WIDTH 256
#define WIN_HEIGHT 368
#define SCALING 4

#define JOY_A		0x01
#define JOY_B		0x02
#define JOY_SELECT	0x04
#define JOY_START	0x08
#define JOY_UP		0x10
#define JOY_DOWN	0x20
#define JOY_LEFT	0x40
#define JOY_RIGHT 	0x80

#define SPRITE_OVERFLOW	0x20
#define SPRITE_0_HIT	0x40
#define VBLANK_ACTIVE	0x80
#define VBLANK_ENABLE	0x80

#define PPUMASK_GREY	0x01
#define PPUMASK_BG_LEFT	0x02
#define PPUMASK_SP_LEFT	0x04
#define PPUMASK_BG		0x08
#define PPUMASK_SP		0x10
#define PPUMASK_RED		0x20
#define PPUMASK_GREEN	0x40
#define PPUMASK_BLUE 	0x80

struct nes_header {
	uint8_t name[4];
	uint8_t prg_rom_size;
	uint8_t chr_rom_size;
	uint8_t flags6;
	uint8_t flags7;
	uint8_t prg_ram_size;
	uint8_t flags9;
	uint8_t flags10;
	uint8_t padding[5];
};

typedef struct s_cart
{
	uint8_t	*prg_rom;
	uint8_t	*chr_mem;
	size_t	prg_banks;
	size_t	chr_banks;
	uint8_t	cur_prg_bank;
	uint8_t	cur_chr_bank;
	uint8_t	mapper_id;
	uint8_t	mirroring;
	bool	chr_is_ram;
} t_cart;

typedef enum e_ppureg
{
	PPUCTRL = 0,
	PPUMASK,
	PPUSTATUS,
	OAMADDR,
	OAMDATA,
	PPUSCROLL,
	PPUADDR,
	PPUDATA,
}	PPUReg;

typedef enum e_ioreg
{
	SQ1_VOL = 0,
	SQ1_SWEEP,
	SQ1_LO,
	SQ1_HI,
	SQ2_VOL,
	SQ2_SWEEP,
	SQ2_LO,
	SW2_HI,
	TRI_LINEAR,
	UNUSED_09,
	TRI_LO,
	TRI_HI,
	NOISE_VOL,
	UNUSED_0D,
	NOISE_LO,
	NOISE_HI,
	DMC_FREQ,
	DMC_RAW,
	DMC_START,
	DMC_LEN,
	OAMDMA,
	SND_CHN,
	JOY1,
	JOY2,
}	IOReg;

typedef struct s_sprite
{
	uint8_t x;
	uint8_t y;
	uint8_t	tile_id;
	uint8_t attr;
	uint8_t	pixels[8];
	bool	sprite_0;
}	t_sprite;

typedef struct s_nes t_nes;

typedef struct s_ppu
{
	t_nes		*nes;
	uint8_t		registers[8];
	uint16_t	cycle;
	uint16_t	scanline;
	uint8_t		mirroring;
	uint16_t	v;
	uint16_t	t;
	uint8_t		x;
	uint8_t		w;
	uint8_t		readbuf;
	bool		*nmi_pin;
	bool		nmi_state_prev;
	uint8_t		vram[0x800];
	uint8_t		oam[0x100];
	uint8_t		oam_addr;
	t_sprite	secondary_oam[0x08];
	uint8_t		secondary_count;
	uint8_t		palette[0x20];
	struct pt_entry	pagetable[0x10];
	uint32_t	*screenbuf;
	Texture2D	screen_tex;
} t_ppu;

struct s_nes
{
	t_cpu	cpu;
	t_ppu	ppu;
	t_cart	*cart;
	bool	joy_strobe;
	uint8_t	joy_state[2];
	uint8_t	joy_shift[2];
	uint8_t	ram[0x800];
	uint8_t	dump[0x400];
	struct {
		uint8_t	pattern_palette;
		int32_t	fps;
	} settings;
};

void	init_nes(t_nes *nes);
t_cart	*read_nes_cart(const char *path);
void	free_cart(t_cart *cart);
void	nes_load_cartridge(t_nes *nes, t_cart *cart);

const char	*get_ppureg_str(PPUReg reg);
void		map_ppu_nametables(t_ppu *ppu, int mirror_mode);
void		get_sprite_data(t_ppu *ppu, t_sprite *sprite, uint32_t oam_index);
uint8_t		ppu_read(t_ppu *ppu, uint16_t addr);
void		ppu_write(t_ppu *ppu, uint16_t addr, uint8_t val);
void		ppu_tick(t_ppu *ppu);
void		ppu_tick_for(t_ppu *ppu, uint32_t n_ticks);
void		ppu_catchup(t_nes *nes);

uint8_t		nes_step(t_nes *nes);
uint64_t	nes_run_for(t_nes *nes, uint64_t n_cycles);

void	init_raylib(t_nes *nes);
void	draw_palette(void);
void	fetch_tile_row(t_ppu *ppu, uint8_t *buf, uint8_t table, uint8_t tile_id, uint8_t row);
void	draw_pixel(t_ppu *ppu, int x, int y, uint32_t col);
void	test_tile_fetch(void);
void	draw_tile(t_ppu *ppu, uint8_t table, uint8_t tile_id, int x, int y);
void	draw_pattern_table(t_ppu *ppu, uint8_t table, int posX, int posY);
void	update_frame(t_nes *nes);

void	handle_player_input(t_nes *nes);

#endif
