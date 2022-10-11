#include "SDL.h"
#include "SDL_image.h"
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
    va_start(ap, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, ap);
    va_end(ap);
#endif
}

int main(int argc, char* argv[]) {
    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_Log("We compiled against SDL version %u.%u.%u ...\n",
            compiled.major, compiled.minor, compiled.patch);
    SDL_Log("But we are linking against SDL version %u.%u.%u.\n",
            linked.major, linked.minor, linked.patch);

    int r = SDL_Init(SDL_INIT_VIDEO);
    if (r < 0) {
        SDL_Log("SDL_Init failed with message=%s (r=%d)", SDL_GetError(), r);
        return 1;
    }

    r = IMG_Init(IMG_INIT_PNG);
    if (r != IMG_INIT_PNG) {
        SDL_Log("IMG_INit failed with message=%s (r=%d)", IMG_GetError(), r);
        return 1;
    }

    int width = 640;
    int height = 480;
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#if defined(__ANDROID__)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif

    SDL_Window *window;
    window = SDL_CreateWindow(
            "An SDL2 window",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
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
    renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
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
        SDL_Rect rect;
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
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_DISPLAYEVENT:
                    switch (event.display.event) {
                        case SDL_DISPLAYEVENT_ORIENTATION:
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
                    }
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            width = event.window.data1;
                            height = event.window.data2;
                            break;
                        case SDL_WINDOWEVENT_SHOWN:
                            foreground = 1;
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            foreground = 0;
                            break;
                    }
                    break;
#if !defined(__ANDROID__)
                case SDL_MOUSEBUTTONDOWN:
                    SDL_Log("mouse button down: which=%d, [%d, %d]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which >= 0 && event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 1;
                        locations[event.button.which].rect.x = event.button.x - RECT_W/2;
                        locations[event.button.which].rect.y = event.button.y - RECT_W/2;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    SDL_Log("mouse button up: which=%d, [%d, %d]", event.button.which, event.button.x, event.button.y);
                    if (event.button.which >= 0 && event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].valid = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    SDL_Log("mouse move: button=%d", event.motion.which);
                    if (event.button.which >= 0 && event.button.which < ARRAY_SIZE(locations)) {
                        locations[event.button.which].rect.x = event.motion.x - RECT_W/2;
                        locations[event.button.which].rect.y = event.motion.y - RECT_W/2;
                    }
                    break;
                case SDL_APP_WILLENTERBACKGROUND:
                    foreground = 0;
                    break;
                case SDL_APP_DIDENTERFOREGROUND:
                    foreground = 1;
                    break;
#endif
#if defined(ANDROID)
                case SDL_FINGERDOWN:
                    SDL_Log("finger down: fingerId=%lld, [%f, %f]", event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerId >= 0 && event.tfinger.fingerId < ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerId].valid = 1;
                        locations[event.tfinger.fingerId].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerId].rect.y = height * event.tfinger.y - RECT_W/2;
                    }
                    break;
                case SDL_FINGERUP:
                    SDL_Log("mouse button up: fingerId=%lld, [%f, %f]", event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                    if (event.tfinger.fingerId >= 0 && event.tfinger.fingerId < ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerId].valid = 0;
                    }
                    break;
                case SDL_FINGERMOTION:
                    SDL_Log("mouse move: button=%d", event.motion.which);
                    if (event.tfinger.fingerId >= 0 && event.tfinger.fingerId < ARRAY_SIZE(locations)) {
                        locations[event.tfinger.fingerId].rect.x = width * event.tfinger.x - RECT_W/2;
                        locations[event.tfinger.fingerId].rect.y = height * event.tfinger.y - RECT_W/2;
                    }
                    break;
                case SDL_APP_TERMINATING:
                    SDL_Log("Received SDL_APP_TERMINATING");
                    quit = 1;
                    break;
#endif
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_RETURN:
                            if (event.key.keysym.mod & KMOD_ALT) {
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

    IMG_Quit();
    SDL_Quit();
    return 0;
}
