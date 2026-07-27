#include <stdatomic.h>
#include <stdint.h>
#include "audio_stream.h"

t_audio_ring_buffer	g_audio_buffer = {};

bool	ring_buffer_push(t_audio_ring_buffer *rb, int16_t sample)
{
	uint32_t	current_head = atomic_load_explicit(&rb->head, memory_order_relaxed);
	uint32_t	current_tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

	uint32_t	next_head = (current_head + 1) % RING_BUFFER_SIZE;

	if (next_head == current_tail)
		return false;

	rb->buffer[current_head] = sample;
	atomic_store_explicit(&rb->head, next_head, memory_order_release);
	return true;
}

bool	ring_buffer_pop(t_audio_ring_buffer *rb, int16_t *sample)
{
	uint32_t	current_tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
	uint32_t	current_head = atomic_load_explicit(&rb->head, memory_order_acquire);

	if (current_tail == current_head)
		return false;

	*sample = rb->buffer[current_tail];
	uint32_t	next_tail = (current_tail + 1) % RING_BUFFER_SIZE;

	atomic_store_explicit(&rb->tail, next_tail, memory_order_release);
	return true;
}

uint32_t ring_buffer_available(t_audio_ring_buffer *rb)
{
	uint32_t	current_head = atomic_load_explicit(&rb->head, memory_order_acquire);
	uint32_t	current_tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

	return (current_head - current_tail) & (RING_BUFFER_SIZE - 1);
}

void apu_audio_callback(void *buffer_data, unsigned int frames)
{
	int16_t	*samples = (int16_t *)buffer_data;

	for (unsigned int i = 0; i < frames; i++)
	{
		int16_t sample;

		if (ring_buffer_pop(&g_audio_buffer, &sample))
			samples[i] = sample;
		else
			samples[i] = 0;
	}
}

double	calculate_drc_scale(void) // P
{
	const int32_t	TARGET_LVL = RING_BUFFER_SIZE / 2;
	const int32_t	DEADZONE = 500;
	const double	Kp = 0.000005;
	const double	EMA_ALPHA = 0.05;
	const double	MAX_PITCH_SHIFT = 0.005;

	static double	smoothed_lvl = (double)TARGET_LVL;

	int32_t	raw_lvl = (int32_t)ring_buffer_available(&g_audio_buffer);
	smoothed_lvl = smoothed_lvl + EMA_ALPHA * ((double)raw_lvl - smoothed_lvl);

	double	error = smoothed_lvl - TARGET_LVL;
	if (error > -DEADZONE && error < DEADZONE)
		return 1.0;

	error = (error >= DEADZONE) ? error - DEADZONE : error + DEADZONE;

	double	scale = 1.0 + (error * Kp);

	if (scale > 1.0 + MAX_PITCH_SHIFT)
		scale = 1.0 + MAX_PITCH_SHIFT;
	else if (scale < 1.0 - MAX_PITCH_SHIFT)
		scale = 1.0 - MAX_PITCH_SHIFT;

	return scale;
}

double	calculate_drc_scale_alt(void) // PI
{
	const int32_t	TARGET_LVL = RING_BUFFER_SIZE / 2;
	const int32_t	DEADZONE = 500;
	const double	Kp = 0.000005;
	const double	Ki = 0.00000001;
	const double	EMA_ALPHA = 0.05;
	const double	MAX_PITCH_SHIFT = 0.005;

	static double	smoothed_lvl = (double)TARGET_LVL;
	static double	integral_baseline = 1.0;

	int32_t	raw_lvl = (int32_t)ring_buffer_available(&g_audio_buffer);
	smoothed_lvl = smoothed_lvl + EMA_ALPHA * ((double)raw_lvl - smoothed_lvl);

	double	error = smoothed_lvl - TARGET_LVL;

	if (error >= DEADZONE)
		error = error - DEADZONE;
	else if (error <= -DEADZONE)
		error = error + DEADZONE;

	integral_baseline += (error * Ki);

	if (integral_baseline > 1.0 + MAX_PITCH_SHIFT)
		integral_baseline = 1.0 + MAX_PITCH_SHIFT;
	else if (integral_baseline < 1.0 - MAX_PITCH_SHIFT)
		integral_baseline = 1.0 - MAX_PITCH_SHIFT;

	double	scale = integral_baseline + (error * Kp);

	if (scale > 1.0 + MAX_PITCH_SHIFT)
		scale = 1.0 + MAX_PITCH_SHIFT;
	else if (scale < 1.0 - MAX_PITCH_SHIFT)
		scale = 1.0 - MAX_PITCH_SHIFT;

	return scale;
}
