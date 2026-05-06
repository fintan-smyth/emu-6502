#include "nes.h"
#include <fcntl.h>
#include <raylib.h>
#include <stdint.h>
#include <unistd.h>

void	load_save_game(t_nes *nes)
{
	t_cart *cart = nes->cart;
	uint32_t *screenbufptr = nes->ppu.screenbuf;
	Texture2D screen_tex = nes->ppu.screen_tex;
	char	buf[512];
	snprintf(buf, 512, ".savedata_%s", nes->cart->title);
	printf("%s\n", buf);
	int fd = open(buf, O_RDONLY);
	read(fd, nes, sizeof(t_nes));
	if (cart->has_chr_ram)
		read(fd, cart->chr_ram, 0x2000);
	close(fd);
	nes->ppu.screenbuf = screenbufptr;
	nes->ppu.screen_tex = screen_tex;
	init_nes(nes);
	nes_load_cartridge(nes, cart);
}

void	save_game(t_nes *nes)
{
	t_cart *cart = nes->cart;

	char	buf[512];
	snprintf(buf, 512, ".savedata_%s", nes->cart->title);
	printf("%s\n", buf);
	int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
	write(fd, nes, sizeof(t_nes));
	if (nes->cart->has_chr_ram)
		write(fd, nes->cart->chr_ram, 0x2000);
	close(fd);
}
