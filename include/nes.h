#ifndef NES_H
# define NES_H

#include "emu6502.h"
#include <raylib.h>
#include <stdint.h>
#include <sys/types.h>

#define MIRROR_HORIZONTAL 0
#define MIRROR_VERTICAL 1
#define MIRROR_SINGLE_LOW 2
#define MIRROR_SINGLE_HIGH 3

#define WIN_WIDTH 256
#define WIN_HEIGHT 368
#define SCALING 4

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define AUDIO_BUFFER_SIZE 1024
#define CHANNELS 1

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

struct nes_1_header
{
	uint8_t magic[4];
	uint8_t prg_rom_size;
	uint8_t chr_rom_size;
	uint8_t flags6;
	uint8_t flags7;
	uint8_t prg_ram_size;
	uint8_t flags9;
	uint8_t flags10;
	uint8_t padding[5];
};

struct nes_2_header
{
	uint8_t magic[4];
	uint8_t prg_rom_size;
	uint8_t chr_rom_size;
	uint8_t flags6;
	uint8_t flags7;
	uint8_t mapper_extra;
	uint8_t rom_msb;
	uint8_t prg_ram_size;
	uint8_t chr_ram_size;
	uint8_t timing;
	uint8_t type_info;
	uint8_t misc_roms;
	uint8_t expansion_device;
};

typedef enum Mapper
{
	NROM = 0,
	MMC1,
	UxROM,
	CNROM,
	MMC3,
	MMC5,
} MapperID;

typedef struct s_cart
{
	char		title[256];
	uint8_t		*prg_rom;
	uint8_t		*chr_rom;
	uint8_t		*prg_ram;
	uint8_t		*chr_ram;
	size_t		prg_rom_banks;
	size_t		chr_rom_banks;
	size_t		prg_ram_banks;
	size_t		chr_ram_banks;
	MapperID	mapper_id;
	uint8_t		submapper;
	uint8_t		header_type;
	uint8_t		mirroring;
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
	SQ2_HI,
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
	IOREG_MAX,
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

enum
{
	MMC1_CTRL,
	MMC1_CHR0,
	MMC1_CHR1,
	MMC1_PRG,
};

union mapper
{
	struct
	{
		uint8_t shift;
		uint8_t	registers[4];
	}	mmc1;
	struct
	{
		uint8_t	cur_prg_bank;
		uint8_t	cur_chr_bank;
	}	uxrom;
	struct
	{

	}	cnrom;
	struct
	{

	}	mmc3;
	struct
	{

	}	mmc5;
};

struct square_channel
{
	uint8_t		duty_mode;
	uint8_t		duty_step;
	uint16_t	timer_reload;
	uint16_t	timer_tick;
	uint8_t		volume;
	uint8_t		length_counter;
	bool		length_halt;
};

typedef struct s_apu
{
	AudioStream	stream;
	uint32_t	cpu_cycles;
	uint8_t		frame_count;
	struct square_channel	square[2];
}	t_apu;

struct s_nes
{
	t_cpu	cpu;
	t_ppu	ppu;
	t_apu	apu;
	t_cart	*cart;
	bool	joy_strobe;
	uint8_t	joy_state[2];
	uint8_t	joy_shift[2];
	uint8_t	ram[0x800];
	// uint8_t	dump[0x400];
	union mapper	mapper;
	struct {
		uint8_t	pattern_palette;
		int32_t	fps;
	} settings;
};

void	init_nes(t_nes *nes);
void	apply_nes_mmap(t_nes *nes);

t_cart	*read_nes_cart(const char *path);
void	free_cart(t_cart *cart);
void	nes_load_cartridge(t_nes *nes, t_cart *cart);

const char	*get_ppureg_str(PPUReg reg);
void		map_ppu_pattern_tables(t_nes *nes, t_cart *cart);
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

void	save_game(t_nes *nes);
void	load_save_game(t_nes *nes);
void	handle_player_input(t_nes *nes);

// Mappers

void	uxrom_write_handler(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val);
void	init_mapper_mmap(t_nes *nes, t_cart *cart);
void	refresh_mapper_mmap(t_nes *nes, t_cart *cart);


void	handle_apu_writes(t_apu *apu, IOReg reg, uint8_t val);
void	apu_tick(t_apu *apu);
void	apu_tick_for(t_apu *apu, uint32_t n_ticks);

#endif
