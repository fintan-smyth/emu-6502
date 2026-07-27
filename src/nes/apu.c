#include "audio_stream.h"
#include "emu6502.h"
#include "nes.h"
#include "emulator.h"
#include <stdint.h>
#include <sys/types.h>

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

void	tick_square_timer(struct square_channel *sq)
{
	if (sq->timer_tick > 0)
		sq->timer_tick--;
	else
	{
		sq->timer_tick = sq->timer_reload;
		sq->duty_step = (sq->duty_step + 1) % 8;
	}
}

void	tick_triangle_timer(struct triangle_channel *tri)
{
	if (tri->timer_tick > 0)
		tri->timer_tick--;
	else
	{
		tri->timer_tick = tri->timer_reload;

		if (tri->length_counter > 0 && tri->linear_counter > 0 && tri->timer_reload > 2)
			tri->sequence_step = (tri->sequence_step + 1) % 32;
	}
}

void	tick_noise_timer(struct noise_channel *noise)
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

void	apu_tick_timers(t_apu *apu)
{
	static bool odd_cycle = false;

	if (odd_cycle)
	{
		tick_square_timer(&apu->square[0]);
		tick_square_timer(&apu->square[1]);
		tick_noise_timer(&apu->noise);
	}
	tick_triangle_timer(&apu->triangle);
	odd_cycle = !odd_cycle;
}

uint16_t	sweep_get_period(struct square_channel *sq)
{
	uint16_t change = sq->timer_reload >> sq->sweep.shift;

	if (sq->sweep.negate)
		return sq->timer_reload - change - sq->sweep.up_fix;

	return sq->timer_reload + change;
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

void	square_tick_sweep(struct square_channel *sq)
{
	uint16_t target_period = sweep_get_period(sq);

	if (sq->sweep.divider == 0 && sq->sweep.enabled && sq->sweep.shift > 0)
	{
		if (sq->timer_reload >= 8 && target_period <= 0x7FF)
			sq->timer_reload = target_period;
	}

	if (sq->sweep.divider == 0 || sq->sweep.reload)
	{
		sq->sweep.reload = false;
		sq->sweep.divider = sq->sweep.divider_period;
	}
	else
		sq->sweep.divider--;
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

void	clock_quarter_frame(t_apu *apu)
{
	square_tick_envelope(&apu->square[0]);
	square_tick_envelope(&apu->square[1]);
	noise_tick_envelope(&apu->noise);
	triangle_tick_linear(&apu->triangle);
}

void	clock_half_frame(t_apu *apu)
{
	square_tick_sweep(&apu->square[0]);
	square_tick_sweep(&apu->square[1]);

	if (!apu->square[0].length_halt && (apu->square[0].length_counter > 0))
		apu->square[0].length_counter--;
	if (!apu->square[1].length_halt && (apu->square[1].length_counter > 0))
		apu->square[1].length_counter--;
	if (!apu->noise.length_halt && (apu->noise.length_counter > 0))
		apu->noise.length_counter--;
	if (!apu->triangle.control_flag && (apu->triangle.length_counter > 0))
		apu->triangle.length_counter--;
}

void	tick_frame_counter(t_apu *apu)
{
	if (apu->frame_count.mode == 0)
	{
		switch (apu->frame_count.step) {
			case (0):
				clock_quarter_frame(apu);
				break;
			case (1):
				clock_quarter_frame(apu);
				clock_half_frame(apu);
				break;
			case (2):
				clock_quarter_frame(apu);
				break;
			case (3):
				clock_quarter_frame(apu);
				clock_half_frame(apu);
				if (!apu->frame_count.irq_inhibit)
					apu->frame_count.irq_pending = true;
				break;
		}
		apu->frame_count.step = (apu->frame_count.step + 1) % 4;
	}
	else
	{
		switch (apu->frame_count.step) {
			case (0):
				clock_quarter_frame(apu);
				break;
			case (1):
				clock_quarter_frame(apu);
				clock_half_frame(apu);
				break;
			case (2):
				clock_quarter_frame(apu);
				break;
			case (3):
				// Idle
				break;
			case (4):
				clock_quarter_frame(apu);
				clock_half_frame(apu);
				break;
		}
		apu->frame_count.step = (apu->frame_count.step + 1) % 5;
	}
}

uint8_t square_get_output_volume(struct square_channel *square)
{
	uint8_t		final_volume = square->envelope_enabled ? square->decay_level : square->volume;
	uint16_t	target_period = sweep_get_period(square);
	bool		muted = (square->timer_reload < 8) || (target_period > 0x7FF);
	// bool		muted = false;

	if (!muted && square->length_counter > 0 && square->timer_tick >= 8
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

float	mix_channels(uint8_t sq_out, uint8_t tri_out, uint8_t noise_out, uint8_t dmc_out)
{
	float sq_mix = 0.0f;
	if (sq_out > 0)
		sq_mix = 95.88f / ( (8128.0f / (float)sq_out) + 100.0f);

	float tnd_mix = 0.0f;
	float tnd_divisor = (tri_out / 8227.0f) + (noise_out / 12241.0f) + (dmc_out / 22638.0f);
	if (tnd_divisor > 0)
		tnd_mix = 159.79f / ( (1.0f / tnd_divisor) + 100.0f);

	return sq_mix + tnd_mix;
	(void)dmc_out;
}

float filter_audio_mix(float audio_mix)
{
	const float		HPF_ALPHA = 0.99f;
	const float		LPF_ALPHA = 0.50f;
	// const float		LPF_ALPHA = 1.00f;
	static float	prev_raw = 0.0f;
	static float	prev_hpf = 0.0f;
	static float	prev_lpf = 0.0f;

	float hpf_out = HPF_ALPHA * (prev_hpf + audio_mix - prev_raw);
	prev_raw = audio_mix;
	prev_hpf = hpf_out;

	float lpf_out = prev_lpf + LPF_ALPHA * (hpf_out - prev_lpf);
	prev_lpf = lpf_out;
	
	return lpf_out;
}

float	generate_audio_sample(t_apu *apu)
{

	uint8_t sq_out = square_get_output_volume(&apu->square[0]) + square_get_output_volume(&apu->square[1]);
	uint8_t noise_out = noise_get_output_volume(&apu->noise);
	uint8_t tri_out = triangle_sequence[apu->triangle.sequence_step];

	// float audio_mix = (sq_out + tri_out + noise_out) / 80.0;
	float audio_mix = mix_channels(sq_out, tri_out, noise_out, 0);
	// float filtered = filter_audio_mix(audio_mix);
	//
	// return (int16_t)(filtered * 24000.0f);
	return audio_mix;
}

void apu_tick(t_apu *apu)
{
	static double	audio_timer = 0.0;

	apu->sample_sum += generate_audio_sample(apu);
	apu->sample_count++;
	audio_timer += 1.0;
	apu->frame_count.cpu_cycles++;

	apu_tick_timers(apu);

	if (apu->frame_count.cpu_cycles >= 7457)
	{
		apu->frame_count.cpu_cycles -= 7457;
		tick_frame_counter(apu);
	}

	double cycles_per_sample = 40.58 * apu->drc_scale * apu->fps_scale;

	while (audio_timer >= cycles_per_sample)
	{
		audio_timer -= cycles_per_sample;

		// uint16_t output = generate_audio_sample(apu);
		float		avg_mix = apu->sample_sum / (float)apu->sample_count;
		float		filtered = filter_audio_mix(avg_mix);
		int16_t		final_sample = (int16_t)(filtered * 32000.0f * ((float)g_settings.volume / 100.0));

		ring_buffer_push(&g_audio_buffer, final_sample);

		apu->sample_sum = 0;
		apu->sample_count = 0;
	}
}

void cpu_io_page_write(struct pt_entry *entry, void *arg, uint16_t addr, uint8_t val)
{
	t_cpu		*cpu = arg;
	t_nes		*nes = (t_nes *)cpu->parent_device;
	// t_ppu		*ppu = &nes->ppu;
	t_apu		*apu = &nes->apu;
	struct square_channel *sq1 = &apu->square[0];
	struct square_channel *sq2 = &apu->square[1];
	struct triangle_channel *tri = &apu->triangle;
	struct noise_channel *noise = &apu->noise;


	addr &= 0xFFF;
	if (addr >= IOREG_MAX)
		return ;

	// IOReg reg = addr & 0xFF;
	// catchup_with_cpu(nes);
	switch (addr) {
		case (SQ1_VOL): // 0x4000
			sq1->duty_mode = (val >> 6) & 0x03;
			sq1->volume = val & 0x0F;
			sq1->length_halt = (val & 0x20) != 0;
			sq1->envelope_enabled = (val & 0x10) == 0;
			return;
		case (SQ1_SWEEP): // 0x4001
			sq1->sweep.enabled = (val & 0x80) != 0;
			sq1->sweep.divider_period = (val >> 4) & 0x07;
			sq1->sweep.negate = (val & 0x08) != 0;
			sq1->sweep.shift = (val & 0x07);
			sq1->sweep.reload = true;
			return;
		case (SQ1_LO): // 0x4002
			sq1->timer_reload = (sq1->timer_reload & 0xFF00) | val;
			return;
		case (SQ1_HI): // 0x4003
			sq1->timer_reload = (sq1->timer_reload & 0x00FF) | ((val & 0x07) << 8);
			sq1->duty_step = 0;
			sq1->envelope_start = true;
			if (apu->status & CHANNEL_SQ1)
				sq1->length_counter = length_table[val >> 3];
			return;
		case (SQ2_VOL): // 0x4004
			sq2->duty_mode = (val >> 6) & 0x03;
			sq2->volume = val & 0x0F;
			sq2->length_halt = (val & 0x20) != 0;
			sq2->envelope_enabled = (val & 0x10) == 0;
			return;
		case (SQ2_SWEEP): // 0x4005
			sq2->sweep.enabled = (val & 0x80) != 0;
			sq2->sweep.divider_period = (val >> 4) & 0x07;
			sq2->sweep.negate = (val & 0x08) != 0;
			sq2->sweep.shift = (val & 0x07);
			sq2->sweep.reload = true;
			return;
		case (SQ2_LO): // 0x4006
			sq2->timer_reload = (sq2->timer_reload & 0xFF00) | val;
			return;
		case (SQ2_HI): // 0x4007
			sq2->timer_reload = (sq2->timer_reload & 0x00FF) | ((val & 0x07) << 8);
			sq2->duty_step = 0;
			sq2->envelope_start = true;
			if (apu->status & CHANNEL_SQ2)
				sq2->length_counter = length_table[val >> 3];
			return;
		case (TRI_LINEAR): // 0x4008
			tri->control_flag = (val & 0x80) != 0;
			tri->linear_reload = val & 0x7F;
			return;
		case (UNUSED_09): // 0x4009
			return;
		case (TRI_LO): // 0x400A
			tri->timer_reload = (tri->timer_reload & 0xFF00) | val;
			return;
		case (TRI_HI): // 0x400B
			tri->timer_reload = (tri->timer_reload & 0x00FF) | ((val & 0x07) << 8);
			if (apu->status & CHANNEL_TRI)
				tri->length_counter = length_table[val >> 3];
			tri->linear_reload_flag = true;
			return;
		case (NOISE_VOL): // 0x400C
			noise->length_halt = (val & 0x20) != 0;
			noise->envelope_enabled = (val & 0x10) == 0;
			noise->volume = val & 0x0F;
			return;
		case (UNUSED_0D): // 0x400D
			return;
		case (NOISE_LO): // 0x400E
			noise->mode_flag = (val & 0x80) != 0;
			noise->timer_reload = noise_timer_table[val & 0x0F];
			return;
		case (NOISE_HI): // 0x400F
			if (apu->status & CHANNEL_NOISE)
				noise->length_counter = length_table[val >> 3];
			noise->envelope_start = true;
			return;
		case (DMC_FREQ): // 0x4010
			return;
		case (DMC_RAW): // 0x4011
			return;
		case (DMC_START): // 0x4012
			return;
		case (DMC_LEN): // 0x4013
			return;
		case (OAMDMA): // 0x4014
			// printf("\e[31;1mDMA initiated\e[m OAMADDR: 0x%02X scanline: %d cycle: %d\n", ppu->oam_addr, ppu->scanline, ppu->cycle);
			cpu->dma.active = true;
			cpu->dma.page = val;
			cpu->dma.step = 0;
			cpu->dma.offset = 0;
			return;
		case (SND_CHN): // 0x4015
			apu->status = (apu->status & 0xE0) | (val & 0x1F);
			if ((val & CHANNEL_SQ1) == 0)
				sq1->length_counter = 0;
			if ((val & CHANNEL_SQ2) == 0)
				sq2->length_counter = 0;
			if ((val & CHANNEL_TRI) == 0)
				tri->length_counter = 0;
			if ((val & CHANNEL_NOISE) == 0)
				noise->length_counter = 0;
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
			apu->frame_count.step = 0;
			apu->frame_count.cpu_cycles = 0;
			apu->frame_count.mode = (val >> 7) & 0x01;
			apu->frame_count.irq_inhibit = (val & BIT_6) != 0;

			if (apu->frame_count.irq_inhibit)
				apu->frame_count.irq_pending = false;

			if (apu->frame_count.mode == 1)
			{
				clock_quarter_frame(apu);
				clock_half_frame(apu);
			}
			return;
		default:
			// Unreachable
			return;
	}
	(void)entry;
}
