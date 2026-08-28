#include "emulator.h"
#include "menu.h"
#include "nes.h"
#include <errno.h>
#include <fcntl.h>
#include <raylib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

size_t	construct_save_folder_path(char *buf, const char *rom_title, size_t bufsize)
{
	size_t len = 0;
	if ((len = strlcpy(buf, XSTR(DATA_DIR)"/saves/", bufsize)) >= bufsize)
		return len;
	len = strlcat(buf, rom_title, bufsize);
	return len;
}

size_t	construct_save_file_path(char *buf, const char *rom_title, uint8_t slot_num, size_t bufsize)
{
	char save_path[256];
	if (slot_num > MAX_SAVE_SLOTS)
		return SIZE_MAX;

	if (slot_num == QUICKSAVE_SLOT_NUM)
		snprintf(save_path, 256, "/savedata_%s", "quicksave");
	else
		snprintf(save_path, 256, "/savedata_slot%2u", slot_num);

	size_t len = 0;
	if ((len = construct_save_folder_path(buf, rom_title, bufsize)) >= bufsize)
		return len;
	
	return strlcat(buf, save_path, bufsize);
}


void	ensure_dir_exists(const char *path)
{
	if (mkdir(path, 0755) == 0)
		return ;

	if (errno == EEXIST)
	{
		struct stat statbuf;
		if (stat(path, &statbuf) == 0)
		{
			if (S_ISDIR(statbuf.st_mode))
				return ;
			else
				printf("Error: directory path exists but is not a directory: %s\n", path);
		}
		else
			printf("Error: stat failed on directory path: %s\n", path);
	}
	else
		printf("Error: mkdir failed on directory path: %s\n", path);
	exit(EXIT_FAILURE);
}

void	ensure_save_dir_exists(const char *rom_title)
{
	char buf[256];

	ensure_dir_exists(XSTR(DATA_DIR)"/saves");
	construct_save_folder_path(buf, rom_title, 256);
	ensure_dir_exists(buf);
}

void	save_game(t_emulator *emu, uint8_t slot_num)
{
	t_nes *nes = &emu->nes;
	t_cart *cart = nes->cart;

	char	buf[512];
	construct_save_file_path(buf, cart->title, slot_num, 512);

	int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
	write(fd, nes, sizeof(t_nes));
	if (cart->prg_ram_banks > 0)
		write(fd, cart->prg_ram, 0x2000 * cart->prg_ram_banks);
	if (cart->chr_ram_banks > 0)
		write(fd, cart->chr_ram, 0x1000 * cart->chr_ram_banks);
	close(fd);

	if (slot_num > MAX_SAVE_SLOTS)
		return ;
	if (slot_num == QUICKSAVE_SLOT_NUM)
		enqueue_msg(&emu->msg_queue, new_msg("Saved state to quicksave", 3.0));
	else
		enqueue_msg(&emu->msg_queue, new_msg(TextFormat("Saved state to Slot %02u", slot_num), 3.0));
}

bool	load_save_game(t_emulator *emu, uint8_t slot_num)
{
	t_nes *nes = &emu->nes;
	t_cart *cart = nes->cart;

	char	buf[512];
	if (construct_save_file_path(buf, cart->title, slot_num, 512) > 512)
		return false;
	int fd = open(buf, O_RDONLY);
	if (fd < 0)
		return false;

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
	if (slot_num > MAX_SAVE_SLOTS)
		return false;
	if (slot_num == QUICKSAVE_SLOT_NUM)
		enqueue_msg(&emu->msg_queue, new_msg("Loaded state from quicksave", 3.0));
	else
		enqueue_msg(&emu->msg_queue, new_msg(TextFormat("Loaded state from Slot %02u", slot_num), 3.0));
	return true;
}
