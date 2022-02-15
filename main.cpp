#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <cmath>
#include <cstdio>
#include <SDL2/SDL.h>

constexpr auto FPS = 60;
constexpr auto FRAME_DELAY = 1000 / FPS;

constexpr auto MAX_AMPLITUDE = 32767.F;
constexpr auto FREQUENCY = 48000;
constexpr auto CHANNELS = 2;
constexpr auto FORMAT = AUDIO_F32;
constexpr auto SAMPLES = 1024;

SDL_AudioCVT cvt;

void cb(void* data, Uint8* stream, int len) {
    if (!stream) {
        return;
    }

    if (cvt.needed) {
        memcpy(cvt.buf, stream, len);
        SDL_ConvertAudio(&cvt);
    }

    auto streamf32 = cvt.needed ? (float*)cvt.buf : (float*)stream;
    auto len_final = cvt.needed ? cvt.len_cvt : len;
    auto max_sample = -INFINITY;

    for (int i = 0; i < len_final / 4; i += 4) {
        max_sample = MAX(max_sample, streamf32[i] + streamf32[i + 1] + streamf32[i + 2] + streamf32[i + 3]);
    }

    auto db = 20 * log10(max_sample / MAX_AMPLITUDE);
    printf("db is %f\n", db);
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    auto window = SDL_CreateWindow("HELLO WORLD", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = FREQUENCY;
    want.format = FORMAT;
    want.channels = CHANNELS;
    want.samples = SAMPLES;
    want.callback = cb;
    auto audio_device = SDL_OpenAudioDevice(nullptr, 1, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);

    SDL_BuildAudioCVT(&cvt, have.format, have.channels, have.freq, FORMAT, CHANNELS, FREQUENCY);
    cvt.len = have.samples * have.channels * (SDL_AUDIO_BITSIZE(have.format) / 8);
    cvt.buf = (Uint8*)SDL_malloc(cvt.len * cvt.len);

    SDL_PauseAudioDevice(audio_device, 0);

    bool close = false;
    while (!close) {
        auto start = SDL_GetTicks();

        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                close = true;
                break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        auto frame_time = SDL_GetTicks() - start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    SDL_CloseAudioDevice(audio_device);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
 
