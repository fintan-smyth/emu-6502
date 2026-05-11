#include "nes.h"
#include <stdint.h>

static const uint8_t duty_table[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1} 
};

static const uint8_t length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

void	handle_apu_writes(t_apu *apu, IOReg reg, uint8_t val)
{
	switch (reg) {
		case (SQ1_VOL): // 0x4000
			apu->square[0].duty_mode = (val >> 6) & 0x03;
			apu->square[0].volume = val & 0x0F;
			apu->square[0].length_halt = (val & 0x20) != 0;
			return;
		case (SQ1_SWEEP): // 0x4001
			return;
		case (SQ1_LO): // 0x4002
			apu->square[0].timer_reload = (apu->square[0].timer_reload & 0xFF00) | val;
			return;
		case (SQ1_HI): // 0x4003
			apu->square[0].timer_reload = (apu->square[0].timer_reload & 0x00FF) | ((val & 0x07) << 8);
			apu->square[0].duty_step = 0;
			apu->square[0].length_counter = length_table[val >> 3];
			return;
		case (SQ2_VOL): // 0x4004
			apu->square[1].duty_mode = (val >> 6) & 0x03;
			apu->square[1].volume = val & 0x0F;
			apu->square[1].length_halt = (val & 0x20) != 0;
			return;
		case (SQ2_SWEEP): // 0x4005
			return;
		case (SQ2_LO): // 0x4006
			apu->square[1].timer_reload = (apu->square[1].timer_reload & 0xFF00) | val;
			return;
		case (SQ2_HI): // 0x4007
			apu->square[1].timer_reload = (apu->square[1].timer_reload & 0x00FF) | ((val & 0x07) << 8);
			apu->square[1].duty_step = 0;
			apu->square[1].length_counter = length_table[val >> 3];
			return;
		case (TRI_LINEAR): // 0x4008
			return;
		case (UNUSED_09): // 0x4009
			return;
		case (TRI_LO): // 0x400a
			return;
		case (TRI_HI): // 0x400b
			return;
		case (NOISE_VOL): // 0x400c
			return;
		case (UNUSED_0D): // 0x400d
			return;
		case (NOISE_LO): // 0x400e
			return;
		case (NOISE_HI): // 0x400f
			return;
		case (DMC_FREQ): // 0x4010
			return;
		case (DMC_RAW): // 0x4011
			return;
		case (DMC_START): // 0x4012
			return;
		case (DMC_LEN): // 0x4013
			return;
		default:
			// Unreachable
			return;
	}
}

void	tick_square(struct square_channel *sq)
{
	if (sq->timer_tick > 0)
		sq->timer_tick--;
	else
	{
		sq->timer_tick = sq->timer_reload;
		sq->duty_step = (sq->duty_step + 1) % 8;
	}
}

void	apu_tick(t_apu *apu)
{
	tick_square(&apu->square[0]);
	tick_square(&apu->square[1]);

}

void apu_tick_for(t_apu *apu, uint32_t cpu_cycles)
{
	static double	audio_timer = 0.0;
	static uint16_t	sample_buffer[1024] = {};
	static uint16_t	buffer_idx = 0;
	static uint8_t	cycle_carry = 0;

	apu->cpu_cycles += cpu_cycles;
	audio_timer += cpu_cycles;

	cpu_cycles += cycle_carry;
	cycle_carry = cpu_cycles & 0x01;
	uint32_t apu_cycles = cpu_cycles / 2;
	while (apu_cycles--)
		apu_tick(apu);

	if (apu->cpu_cycles >= 7457)
	{
		apu->cpu_cycles -= 7457;
		apu->frame_count = (apu->frame_count + 1) % 4;

		// printf("Frame count ticked!\n");
		if (apu->frame_count % 2 == 1)
		{
			if (!apu->square[0].length_halt && apu->square[0].length_counter > 0)
				apu->square[0].length_counter--;
			if (!apu->square[1].length_halt && apu->square[1].length_counter > 0)
				apu->square[1].length_counter--;
		}
	}

	while (audio_timer >= 40.58)
	{
		audio_timer -= 40.58;

		uint16_t output = 0;
		if (duty_table[apu->square[0].duty_mode][apu->square[0].duty_step] == 1 && apu->square[0].length_counter)
			output = apu->square[0].volume * 300;
		if (duty_table[apu->square[1].duty_mode][apu->square[1].duty_step] == 1 && apu->square[1].length_counter)
			output += apu->square[1].volume * 300;

		sample_buffer[buffer_idx] = output;
		buffer_idx++;

		if (buffer_idx == 1024)
		{
			while (!IsAudioStreamProcessed(apu->stream))
				;
			UpdateAudioStream(apu->stream, sample_buffer, 1024);
			buffer_idx = 0;
		}
	}
}
