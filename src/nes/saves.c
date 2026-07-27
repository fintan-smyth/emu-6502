#include "emulator.h"
#include "menu.h"
#include "nes.h"
#include <fcntl.h>
#include <raylib.h>
#include <stdint.h>
#include <unistd.h>

void	save_game(t_emulator *emu)
{
	t_nes *nes = &emu->nes;
	t_cart *cart = nes->cart;

	char	buf[512];
	snprintf(buf, 512, ".savedata_%s", nes->cart->title);
	enqueue_msg(&emu->msg_queue, new_msg(TextFormat("Saved state to file \'%s\'", buf), 3.0));
	int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
	write(fd, nes, sizeof(t_nes));
	if (cart->prg_ram_banks > 0)
		write(fd, cart->prg_ram, 0x2000 * cart->prg_ram_banks);
	if (cart->chr_ram_banks > 0)
		write(fd, cart->chr_ram, 0x1000 * cart->chr_ram_banks);
	close(fd);
}

void	load_save_game(t_emulator *emu)
{
	t_nes *nes = &emu->nes;

	char	buf[512];
	snprintf(buf, 512, ".savedata_%s", nes->cart->title);
	printf("%s\n", buf);
	int fd = open(buf, O_RDONLY);
	if (fd < 0)
		return ;

	t_cart *cart = nes->cart;
	read(fd, nes, sizeof(t_nes));
	if (cart->prg_ram_banks > 0)
		read(fd, cart->prg_ram, 0x2000 * cart->prg_ram_banks);
	if (cart->chr_ram_banks > 0)
		read(fd, cart->chr_ram, 0x1000 * cart->chr_ram_banks);
	close(fd);
	init_nes(nes);
	// nes_load_cartridge(nes, cart);
	nes->cart = cart;
	apply_nes_mmap(nes);
	refresh_mapper_mmap(nes, cart);
	enqueue_msg(&emu->msg_queue, new_msg(TextFormat("Loaded state from file \'%s\'", buf), 3.0));
}
