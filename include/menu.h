#ifndef MENU_H
# define MENU_H

#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

#define MSG_QUEUE_FONTSIZE 30

typedef struct s_msg
{
	char			buf[256];
	double			submitted_time;
	double			display_time;
	struct s_msg	*next;
}	t_msg;

typedef enum
{
	BUTTON,
	VAR_SELECTOR,
	VAR_SLIDER,
}	MenuItemType;

typedef enum
{
	PAUSE,
	SETTINGS,
}	MenuType;

typedef struct
{
	const char		*text;
	MenuItemType	type;
	union {
		struct {
			void (*exec)(void *);
		} button;
		struct {
			size_t	data_offset;
			int32_t	min_val;
			int32_t	max_val;
			int32_t	step;
		} slider;
		struct {
			size_t		data_offset;
			const char	**options;
			int32_t		num_options;
		} selector;
	};
}	t_menuitem;

t_msg	*new_msg(const char *str, double display_time);
void	enqueue_msg(t_msg **queue, t_msg *msg);
void	display_msg_queue(t_msg **queue, double scaling);
void	clear_msg_queue(t_msg **queue);

typedef struct s_emulator t_emulator;

void	draw_menu(t_emulator *emu);
void	handle_menu_input(t_emulator *emu);

#endif // MENU_H
