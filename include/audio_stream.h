#ifndef AUDIO_STREAM_H
# define AUDIO_STREAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define RING_BUFFER_SIZE 4096

typedef struct
{
	int16_t				buffer[RING_BUFFER_SIZE];
	_Atomic uint32_t	head;
	_Atomic uint32_t	tail;
}	t_audio_ring_buffer;

extern t_audio_ring_buffer g_audio_buffer;


bool		ring_buffer_push(t_audio_ring_buffer *rb, int16_t sample);
bool		ring_buffer_pop(t_audio_ring_buffer *rb, int16_t *sample);
uint32_t	ring_buffer_available(t_audio_ring_buffer *rb);
void		apu_audio_callback(void *buffer_data, unsigned int frames);
double		calculate_drc_scale(void);
double		calculate_drc_scale_alt(void);

#endif // AUDIO_STREAM_H
