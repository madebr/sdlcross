#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(WITH_IMAGE)
#include <SDL3_image/SDL_image.h>
#endif
#if defined(WITH_MIXER)
#include <SDL3_mixer/SDL_mixer.h>
#endif
#if defined(WITH_NET)
#include <SDL3_net/SDL_net.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#define ARRAY_SIZE(ARR) ((sizeof(ARR)) / (sizeof(*(ARR))))

static void show_important_message(int duration, const char *format, ...) {
#if defined(SDL_PLATFORM_ANDROID)
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    SDL_ShowAndroidToast(buffer, duration, -1, 0, 0);
#else
    va_list ap;
    (void)duration;
    va_start(ap, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, ap);
    va_end(ap);
#endif
}

int main(int argc, char* argv[]) {
    int linked_version;

    (void)argc;
    (void)argv;

    linked_version = SDL_GetVersion();
    SDL_Log("We compiled against SDL version %u.%u.%u ...\n",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("But we are linking against SDL version %u.%u.%u.\n",
            SDL_VERSIONNUM_MAJOR(linked_version), SDL_VERSIONNUM_MINOR(linked_version), SDL_VERSIONNUM_MICRO(linked_version));

    SDL_SetHint("SDL_MIXER_DISABLE_DRFLAC", "1");
    SDL_SetHint("SDL_MIXER_DISABLE_DRMP3", "1");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed (%s)", SDL_GetError());
        return 1;
    }

#if defined(WITH_IMAGE)
    {
        int v = IMG_Version();
        SDL_Log("SDL3_image version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }
#endif

#if defined(WITH_MIXER)
    {
        int v = MIX_Version();
        SDL_Log("SDL3_mixer version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }

    if (!MIX_Init()) {
        SDL_Log("MIX_Init failed (%s)", SDL_GetError());
        return 1;
    }

    MIX_Mixer *mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (mixer == NULL) {
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec mixerspec;
    MIX_GetMixerFormat(mixer, &mixerspec);
    SDL_Log("Mixer is format %s, %d channels, %d frequency", SDL_GetAudioFormatName(mixerspec.format), mixerspec.channels, mixerspec.freq);

    SDL_Log("Available MIXER decoders:");
    const int num_decoders = MIX_GetNumAudioDecoders();
    if (num_decoders < 0) {
        SDL_Log(" - [error (%s)]", SDL_GetError());
    } else if (num_decoders == 0) {
        SDL_Log(" - [none]");
    } else {
        for (int i = 0; i < num_decoders; i++) {
            SDL_Log(" - %s", MIX_GetAudioDecoder(i));
        }
    }

    const char *const audiofname = "audio/picked-coin-echo-2.wav";
    MIX_Audio *audio = MIX_LoadAudio(mixer, audiofname, false);
    if (audio == NULL) {
        SDL_Log("Failed to load '%s' (%s)", audiofname, SDL_GetError());
    }
    if (audio) {
        SDL_AudioSpec audiospec;
        MIX_GetAudioFormat(audio, &audiospec);
        SDL_Log("%s: %s, %d channel%s, %d freq", audiofname, SDL_GetAudioFormatName(audiospec.format),
            audiospec.channels, (audiospec.channels == 1) ? "" : "s", audiospec.freq);
    }
#endif

#if defined(WITH_NET)
    {
        int v = NET_Version();
        SDL_Log("SDL3_net version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    }
#endif

    int width = 640;
    int height = 480;
    int flags = SDL_WINDOW_RESIZABLE;
#if defined(SDL_PLATFORM_ANDROID)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

    char title[32];
    SDL_Window *window;

    SDL_snprintf(title, sizeof(title), "An SDL %d.%d.%d window",
                 SDL_VERSIONNUM_MAJOR(linked_version), SDL_VERSIONNUM_MINOR(linked_version), SDL_VERSIONNUM_MICRO(linked_version));
    window = SDL_CreateWindow(
            title,
            width,
            height,
            flags
    );

    if (window == NULL) {
        show_important_message(5, "Could not create window %s", SDL_GetError());
        return 1;
    }
    SDL_Log("Window created!");

    SDL_Renderer* renderer = NULL;
    renderer =  SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        show_important_message(5, "Could not create renderer: %s", SDL_GetError());
        return 1;
    }
    SDL_Log("Renderer created!");

    const SDL_Color COLORS[10] = {
            {255, 0, 0, 0},
            {0, 255, 0, 0},
            {0, 0, 255, 0},
            {128, 0, 0, 0},
            {0, 128, 0, 0},
            {0, 0, 128, 0},
            {128, 128, 0, 0},
            {0, 128, 128, 0},
            {128, 0, 128, 0},
            {192, 192, 192, 0},
    };
    struct {
        int valid;
        SDL_FRect rect;
    } locations[10];

#ifdef SDL_PLATFORM_ANDROID
#define RECT_W 250
#else
#define RECT_W 50
#endif
    for (size_t i = 0; i < ARRAY_SIZE(locations); i++) {
        locations[i].valid = 0;
        locations[i].rect.w = RECT_W;
        locations[i].rect.h = RECT_W;
    }

    show_important_message(1, "Entering the loop");

    int fullscreen = 0;
    int foreground = 1;
    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;
                case SDL_EVENT_DISPLAY_ORIENTATION:
                    switch (event.display.data1) {
                        case SDL_ORIENTATION_LANDSCAPE:
                            show_important_message(1, "landscape");
                            break;
                        case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                            show_important_message(1, "landscape (flipped)");
                            break;
                        case SDL_ORIENTATION_PORTRAIT:
                            show_important_message(1, "portrait");
                            break;
                        case SDL_ORIENTATION_PORTRAIT_FLIPPED:
                            show_important_message(1, "portrait (flipped)");
                            break;
                    }
                    break;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    width = event.window.data1;
                    height = event.window.data2;
                    break;
                case SDL_EVENT_WINDOW_SHOWN:
                    foreground = 1;
                    break;
                case SDL_EVENT_WINDOW_HIDDEN:
                    foreground = 0;
                    break;
#if !defined(SDL_PLATFORM_ANDROID)
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button down: which=%d, [%g, %g]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 1;
                        locations[event.button.which].rect.x = event.button.x - RECT_W/2;
                        locations[event.button.which].rect.y = event.button.y - RECT_W/2;
                    }
#if defined(WITH_MIXER)
                    if (audio != NULL && !MIX_PlayAudio(mixer, audio)) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to play audio (%s)", SDL_GetError());
                    }
#endif
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button up: which=%d, [%g, %g]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 0;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d", event.motion.which);
                    if (event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].rect.x = event.motion.x - RECT_W/2;
                        locations[event.button.which].rect.y = event.motion.y - RECT_W/2;
                    }
                    break;
                case SDL_EVENT_WILL_ENTER_BACKGROUND:
                    foreground = 0;
                    break;
                case SDL_EVENT_DID_ENTER_FOREGROUND:
                    foreground = 1;
                    break;
#endif
#if defined(SDL_PLATFORM_ANDROID)
                case SDL_EVENT_FINGER_DOWN:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "finger down: fingerID=%d, [%f, %f]", (int)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].valid = 1;
                        locations[event.tfinger.fingerID].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerID].rect.y = height * event.tfinger.y - RECT_W/2;
                    }

#if defined(WITH_MIXER)
                    // Play the sound effect
                    MIX_PlayAudio(mixer, audio);
#endif

                    break;
                case SDL_EVENT_FINGER_UP:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button up: fingerID=%d, [%f, %f]", (int)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].valid = 0;
                    }
                    break;
                case SDL_EVENT_FINGER_MOTION:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d", event.motion.which);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerID].rect.y = height * event.tfinger.y - RECT_W/2;
                    }
                    break;
                case SDL_EVENT_TERMINATING:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Received SDL_EVENT_TERMINATING");
                    quit = 1;
                    break;
#endif
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_RETURN:
                            if (event.key.mod & SDL_KMOD_ALT) {
                                fullscreen = !fullscreen;
                                SDL_SetWindowFullscreen(window, fullscreen);
                            }
                            break;
                    }
                    break;
            }
        }
        if (foreground) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            for (size_t i = 0; i < ARRAY_SIZE(locations); i++) {
                if (locations[i].valid) {
                    SDL_SetRenderDrawColor(renderer, COLORS[i].r, COLORS[i].g, COLORS[i].b, COLORS[i].a);
                    SDL_RenderFillRect(renderer, &locations[i].rect);
                }
            }
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(10);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);


#if defined(WITH_MIXER)
    MIX_DestroyAudio(audio);
    MIX_DestroyMixer(mixer);
    MIX_Quit();
#endif
    SDL_Quit();
    return 0;
}
