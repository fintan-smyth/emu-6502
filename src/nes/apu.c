#include "nes.h"
#include <stdint.h>

static const uint8_t duty_table[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1}
};

static const uint8_t triangle_sequence[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const uint8_t length_table[32] = {
    10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

static const uint16_t noise_timer_table[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

static float mix_sq_table[31] = {};
static float mix_tnd_table[203] = {};

void	init_audio_mixer()
{
	mix_sq_table[0] = 0.0f;
	for (int i = 1; i < 31; i++)
	{
		mix_sq_table[i] = 95.52f / ((8128.0f / (float)i) + 100);
	}

	mix_tnd_table[0] = 0.0f;
	for (int i = 1; i < 203; i++)
	{
		mix_tnd_table[i] = 159.79f / (1.0f / ((float)i / 8227.0f) + 100.0f);
	}
}

// void	handle_apu_writes(t_apu *apu, IOReg reg, uint8_t val)
// {
// 	switch (reg) {
//
// 		default:
// 			// Unreachable
// 			return;
// 	}
// }

void cpu_io_page_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu		*cpu = arg;
	t_nes		*nes = (t_nes *)cpu->parent_device;
	t_ppu		*ppu = &nes->ppu;
	t_apu		*apu = &nes->apu;
	uint16_t	dma_src = 0;

	addr &= 0xFFF;
	if (addr >= IOREG_MAX)
		return ;

	// IOReg reg = addr & 0xFF;
	switch (addr) {
		case (SQ1_VOL): // 0x4000
			apu->square[0].duty_mode = (val >> 6) & 0x03;
			apu->square[0].volume = val & 0x0F;
			apu->square[0].length_halt = (val & 0x20) != 0;
			apu->square[0].envelope_enabled = (val & 0x10) == 0;
			return;
		case (SQ1_SWEEP): // 0x4001
			return;
		case (SQ1_LO): // 0x4002
			apu->square[0].timer_reload = (apu->square[0].timer_reload & 0xFF00) | val;
			return;
		case (SQ1_HI): // 0x4003
			apu->square[0].timer_reload = (apu->square[0].timer_reload & 0x00FF) | ((val & 0x07) << 8);
			apu->square[0].duty_step = 0;
			apu->square[0].envelope_start = true;
			if (apu->status & CHANNEL_SQ1)
				apu->square[0].length_counter = length_table[val >> 3];
			return;
		case (SQ2_VOL): // 0x4004
			apu->square[1].duty_mode = (val >> 6) & 0x03;
			apu->square[1].volume = val & 0x0F;
			apu->square[1].length_halt = (val & 0x20) != 0;
			apu->square[1].envelope_enabled = (val & 0x10) == 0;
			return;
		case (SQ2_SWEEP): // 0x4005
			return;
		case (SQ2_LO): // 0x4006
			apu->square[1].timer_reload = (apu->square[1].timer_reload & 0xFF00) | val;
			return;
		case (SQ2_HI): // 0x4007
			apu->square[1].timer_reload = (apu->square[1].timer_reload & 0x00FF) | ((val & 0x07) << 8);
			apu->square[1].duty_step = 0;
			apu->square[1].envelope_start = true;
			if (apu->status & CHANNEL_SQ2)
				apu->square[1].length_counter = length_table[val >> 3];
			return;
		case (TRI_LINEAR): // 0x4008
			apu->triangle.control_flag = (val & 0x80) != 0;
			apu->triangle.linear_reload = val & 0x7F;
			return;
		case (UNUSED_09): // 0x4009
			return;
		case (TRI_LO): // 0x400a
			apu->triangle.timer_reload = (apu->triangle.timer_reload & 0xFF00) | val;
			return;
		case (TRI_HI): // 0x400b
			apu->triangle.timer_reload = (apu->triangle.timer_reload & 0x00FF) | ((val & 0x07) << 8);
			if (apu->status & CHANNEL_TRI)
				apu->triangle.length_counter = length_table[val >> 3];
			apu->triangle.linear_reload_flag = true;
			return;
		case (NOISE_VOL): // 0x400c
			apu->noise.length_halt = (val & 0x20) != 0;
			apu->noise.envelope_enabled = (val & 0x10) == 0;
			apu->noise.volume = val & 0x0F;
			return;
		case (UNUSED_0D): // 0x400d
			return;
		case (NOISE_LO): // 0x400e
			apu->noise.mode_flag = (val & 0x80) != 0;
			apu->noise.timer_reload = noise_timer_table[val & 0x0F];
			return;
		case (NOISE_HI): // 0x400f
			if (apu->status & CHANNEL_NOISE)
				apu->noise.length_counter = length_table[val >> 3];
			apu->noise.envelope_start = true;
			return;
		case (DMC_FREQ): // 0x4010
			return;
		case (DMC_RAW): // 0x4011
			return;
		case (DMC_START): // 0x4012
			return;
		case (DMC_LEN): // 0x4013
			return;		case (OAMDMA): // 0x4014
			// cpu->cycle_events |= CYCLE_DMA;
			dma_src = val << 8;
			// cpu->catchup_cycles += 1;
			// ppu_catchup(nes);
			ppu_tick_for(ppu, 3);
			apu_tick_for(apu, 1);
			if (ppu->oam_addr != 0)
				printf("\e[31;1mDMA initiated\e[m OAMADDR: 0x%02X scanline: %d cycle: %d\n", ppu->oam_addr, ppu->scanline, ppu->cycle);
			for (int i = 0; i < 256; i++)
			{
				// cpu->catchup_cycles += 2;
				ppu->oam[ppu->oam_addr++] = read_byte(cpu, dma_src + (uint8_t)i);
				ppu_tick_for(ppu, 6);
				apu_tick_for(apu, 2);
				// ppu_catchup(nes);
			}
			return;
		case (SND_CHN): // 0x4015
			apu->status = (apu->status & 0xE0) | (val & 0x1F);
			if ((val & CHANNEL_SQ1) == 0)
				apu->square[0].length_counter = 0;
			if ((val & CHANNEL_SQ2) == 0)
				apu->square[1].length_counter = 0;
			if ((val & CHANNEL_TRI) == 0)
				apu->triangle.length_counter = 0;
			if ((val & CHANNEL_NOISE) == 0)
				apu->noise.length_counter = 0;
			return;
		case (JOY1): // 0x4016
			nes->joy_strobe = (val & 0x01);
			if (nes->joy_strobe)
			{
				nes->joy_shift[0] = nes->joy_state[0];
				nes->joy_shift[1] = nes->joy_state[1];
			}
			return;
		case (JOY2): // 0x4017
			return;
		default:
			// Unreachable
			return;
	}
	(void)entry;
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

void	tick_triangle(struct triangle_channel *tri)
{
	if (tri->timer_tick > 0)
		tri->timer_tick--;
	else
	{
		tri->timer_tick = tri->timer_reload;

		if (tri->length_counter >0 && tri->linear_counter > 0 && tri->timer_reload > 2)
			tri->sequence_step = (tri->sequence_step + 1) % 32;
	}
}

void	tick_noise(struct noise_channel *noise)
{
	if (noise->timer_tick > 0)
		noise->timer_tick--;
	else
	{
		noise->timer_tick = noise->timer_reload;

		uint16_t feedback = 0;
		if (noise->mode_flag)
			feedback = (noise->shift & 0x01) ^ ((noise->shift >> 6) & 0x01);
		else
			feedback = (noise->shift & 0x01) ^ ((noise->shift >> 1) & 0x01);

		noise->shift >>= 1;
		noise->shift |= (feedback << 14);
	}
}

void	apu_tick(t_apu *apu)
{
	static bool odd_cycle = false;

	if (odd_cycle)
	{
		tick_square(&apu->square[0]);
		tick_square(&apu->square[1]);
		tick_noise(&apu->noise);
	}
	tick_triangle(&apu->triangle);
	odd_cycle = !odd_cycle;
}

void	square_tick_envelope(struct square_channel *square)
{
	if (square->envelope_start)
	{
		square->envelope_start = false;
		square->decay_level = 15;
		square->divider = square->volume;
	}
	else
	{
		if (square->divider > 0)
			square->divider--;
		else
		{
			square->divider = square->volume;
			if (square->decay_level > 0)
				square->decay_level--;
			else if (square->envelope_loop)
				square->decay_level = 15;
		}
	}
}

void	noise_tick_envelope(struct noise_channel *noise)
{
	if (noise->envelope_start)
	{
		noise->envelope_start = false;
		noise->decay_level = 15;
		noise->divider = noise->volume;
	}
	else
	{
		if (noise->divider > 0)
			noise->divider--;
		else
		{
			noise->divider = noise->volume;
			if (noise->decay_level > 0)
				noise->decay_level--;
			else if (noise->envelope_loop)
				noise->decay_level = 15;
		}
	}
}

void	triangle_tick_linear(struct triangle_channel *tri)
{
	if (tri->linear_reload_flag)
		tri->linear_counter = tri->linear_reload;
	else if (tri->linear_counter > 0)
		tri->linear_counter--;

	if (!tri->control_flag)
		tri->linear_reload_flag = false;
}

uint8_t square_get_output_volume(struct square_channel *square)
{
	uint8_t		final_volume = square->envelope_enabled ? square->decay_level : square->volume;

	if (square->length_counter > 0 && square->timer_tick >= 8
		&& duty_table[square->duty_mode][square->duty_step] == 1)
	{
		return final_volume;
	}

	return 0;
}

uint8_t	noise_get_output_volume(struct noise_channel *noise)
{
	uint8_t		final_volume = noise->envelope_enabled ? noise->decay_level : noise->volume;

	if (noise->length_counter > 0 && (noise->shift & 0x01) == 0)
	{
		return final_volume;
	}

	return 0;
}

void apu_tick_for(t_apu *apu, uint32_t cpu_cycles)
{
	static double	audio_timer = 0.0;
	static int16_t	sample_buffer[1024] = {};
	static uint16_t	buffer_idx = 0;
	static float	prev_raw = 0.0f;
	static float	prev_filtered = 0.0f;
	const float		HPF_ALPHA = 0.99f;

	apu->cpu_cycles += cpu_cycles;
	audio_timer += cpu_cycles;

	while (cpu_cycles--)
		apu_tick(apu);

	if (apu->cpu_cycles >= 7457)
	{
		apu->cpu_cycles -= 7457;
		apu->frame_count = (apu->frame_count + 1) % 4;

		square_tick_envelope(&apu->square[0]);
		square_tick_envelope(&apu->square[1]);
		noise_tick_envelope(&apu->noise);
		triangle_tick_linear(&apu->triangle);

		// printf("Frame count ticked!\n");
		if (apu->frame_count % 2 == 1)
		{
			if (!apu->square[0].length_halt && (apu->square[0].length_counter > 0))
				apu->square[0].length_counter--;
			if (!apu->square[1].length_halt && (apu->square[1].length_counter > 0))
				apu->square[1].length_counter--;
			if (!apu->noise.length_halt && (apu->noise.length_counter > 0))
				apu->noise.length_counter--;
			if (!apu->triangle.control_flag && apu->triangle.length_counter > 0)
				apu->triangle.length_counter--;
		}
	}

	while (audio_timer >= (40.58 * apu->fps_scale))
	{
		audio_timer -= (40.58 * apu->fps_scale);

		uint8_t sq_out = square_get_output_volume(&apu->square[0]) + square_get_output_volume(&apu->square[1]);
		uint8_t noise_out = noise_get_output_volume(&apu->noise);
		uint8_t tri_out = triangle_sequence[apu->triangle.sequence_step];
		uint8_t tnd_out = (2 * noise_out) + (3 * tri_out);

		float audio_mix = mix_sq_table[sq_out] + mix_tnd_table[tnd_out];
		float filtered = HPF_ALPHA * (prev_filtered + audio_mix - prev_raw);

		prev_raw = audio_mix;
		prev_filtered = filtered;

		int16_t output = (int16_t)(filtered * 20000.0f);
		// int16_t output = (sq_out + noise_out + tri_out) * 300;
		sample_buffer[buffer_idx++] = output;

		if (buffer_idx == 1024)
		{
			while (!IsAudioStreamProcessed(apu->stream))
				;
			UpdateAudioStream(apu->stream, sample_buffer, 1024);
			buffer_idx = 0;
		}
	}
}
