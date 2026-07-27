#include "emulator.h"
#include "audio_stream.h"
#include "nes.h"
#include <raylib.h>

struct settings g_settings = {};

void init_emulator(t_emulator *emu)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(CANVAS_WIDTH * DEFAULT_SCALING, CANVAS_HEIGHT * DEFAULT_SCALING, "emu6502");
	SetWindowMinSize(CANVAS_WIDTH, CANVAS_HEIGHT);
	InitAudioDevice();
	SetAudioStreamBufferSizeDefault(1024);
	// SetTargetFPS(nes->settings.fps);
	Image blankImage = GenImageColor(CANVAS_WIDTH, CANVAS_HEIGHT, BLANK);
    emu->screen_tex = LoadTextureFromImage(blankImage);
	emu->stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
	g_settings.target_fps = 60;
	g_settings.volume = 100;
	SetAudioStreamCallback(emu->stream, apu_audio_callback);
	PlayAudioStream(emu->stream);
    UnloadImage(blankImage);
	SetExitKey(KEY_NULL);
}

void	run_emulator_frame(t_emulator *emu)
{
	while (!emu->nes.frame_ready)
	{
		nes_step_alt(&emu->nes);
	}
	update_frame(emu);
	emu->nes.frame_ready = false;
}
