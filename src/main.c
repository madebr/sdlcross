#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(WITH_IMAGE)
#include <SDL3/SDL_image.h>
#endif
#if defined(WITH_MIXER)
#include <SDL3/SDL_mixer.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#define ARRAY_SIZE(ARR) ((sizeof(ARR)) / (sizeof(*(ARR))))

static void show_important_message(int duration, const char *format, ...) {
#if defined(__ANDROID__)
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    SDL_AndroidShowToast(buffer, duration, -1, 0, 0);
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

    int r = SDL_Init(SDL_INIT_VIDEO);
    if (r < 0) {
        SDL_Log("SDL_Init failed with message=%s (r=%d)", SDL_GetError(), r);
        return 1;
    }

#if defined(WITH_IMAGE)
    r = IMG_Init(IMG_INIT_PNG);
    if (r != IMG_INIT_PNG) {
        SDL_Log("IMG_INit failed with message=%s (r=%d)", IMG_GetError(), r);
        return 1;
    }
#endif

#if defined(WITH_MIXER)
    r = Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS);
    if (r != (MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS)) {
        SDL_Log("Mix_Init failed with message=%s (r=%d)", Mix_GetError(), r);
        return 1;
    }
#endif

    int width = 640;
    int height = 480;
    int flags = SDL_WINDOW_RESIZABLE;
#if defined(__ANDROID__)
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

#ifdef __ANDROID__
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
#if !defined(__ANDROID__)
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    SDL_Log("mouse button down: which=%d, [%g, %g]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 1;
                        locations[event.button.which].rect.x = event.button.x - RECT_W/2;
                        locations[event.button.which].rect.y = event.button.y - RECT_W/2;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    SDL_Log("mouse button up: which=%d, [%g, %g]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 0;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    SDL_Log("mouse move: button=%d", event.motion.which);
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
#if defined(ANDROID)
                case SDL_EVENT_FINGER_DOWN:
                    SDL_Log("finger down: fingerID=%d, [%f, %f]", (int)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].valid = 1;
                        locations[event.tfinger.fingerID].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerID].rect.y = height * event.tfinger.y - RECT_W/2;
                    }
                    break;
                case SDL_EVENT_FINGER_UP:
                    SDL_Log("mouse button up: fingerID=%d, [%f, %f]", (int)event.tfinger.fingerID, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].valid = 0;
                    }
                    break;
                case SDL_EVENT_FINGER_MOTION:
                    SDL_Log("mouse move: button=%d", event.motion.which);
                    if (event.tfinger.fingerID >= 0 && event.tfinger.fingerID < (int)ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerID].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerID].rect.y = height * event.tfinger.y - RECT_W/2;
                    }
                    break;
                case SDL_EVENT_TERMINATING:
                    SDL_Log("Received SDL_EVENT_TERMINATING");
                    quit = 1;
                    break;
#endif
                case SDL_EVENT_KEY_UP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_RETURN:
                            if (event.key.keysym.mod & SDL_KMOD_ALT) {
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
    Mix_Quit();
#endif
#if defined(WITH_IMAGE)
    IMG_Quit();
#endif
    SDL_Quit();
    return 0;
}
