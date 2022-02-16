#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <cmath>
#include <cstdio>
#include <SDL2/SDL.h>
#include <algorithm>

constexpr auto FPS = 60;
constexpr auto FRAME_DELAY = 1000 / FPS;
constexpr auto PROCESS_INTERVAL_MS = 500;
constexpr auto MAX_AMPLITUDE = 32767.F;

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
    auto len_final = cvt.needed ? cvt.len * cvt.len_ratio : len / 4;
    auto max_sample = *std::max_element(streamf32, streamf32 + (int)len_final);
    auto db = 20 * log10(max_sample / MAX_AMPLITUDE);

    printf("db is %f\n", db);
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    auto window = SDL_CreateWindow("HELLO WORLD", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_AudioSpec spec;
    SDL_GetAudioDeviceSpec(0, 1, &spec);
    spec.format = AUDIO_F32;
    spec.callback = cb;
    spec.samples = spec.freq / (1000 / PROCESS_INTERVAL_MS);
    auto audio_device = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &spec, &spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    
    SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_F32, spec.channels, spec.freq);
    cvt.len = spec.samples * spec.channels * (SDL_AUDIO_BITSIZE(spec.format) / 8);
    cvt.buf = (Uint8*)SDL_malloc(cvt.len * cvt.len_mult);

    printf("Conversion needed: %d\n", (int)cvt.needed);
    printf("Specs are the same: %d\n", (int)(spec.format == AUDIO_F32));

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
 
