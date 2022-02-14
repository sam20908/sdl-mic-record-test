#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <cmath>
#include <cstdio>
#include <SDL2/SDL.h>

constexpr auto FPS = 60;
constexpr auto FRAME_DELAY = 1000 / FPS;
constexpr auto MAX_AMPLITUDE = 32767.F * 2 + 1;

void cb(void* data, Uint8* stream, int len) {
    if (!stream) {
        return;
    }

    auto stream16 = (Uint16*) stream;
    auto len16 = len / (sizeof(Uint16) / sizeof(Uint8));

    auto max_sample = stream16[0];
    for (int i = 0; i < len16; ++i) {
        max_sample = MAX(max_sample, stream16[i]);
    }

    auto db = 20 * log10(max_sample / MAX_AMPLITUDE);
    printf("db is %f at %u\n", db, SDL_GetTicks());
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    auto window = SDL_CreateWindow("HELLO WORLD", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_AudioSpec want, have;
    SDL_zero(want);
    SDL_zero(have);
    want.freq = 44100;
    want.format = AUDIO_U16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = cb;
    auto audio_device = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
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
