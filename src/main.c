#include "context.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
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

#if defined(WITH_BOX2D)
#include "box2d-debug-drawer.h"
#include <box2d/box2d.h>
#endif

#include <stdarg.h>


static const SDL_Color COLORS[10] = {
    {255, 0,   0,   0},
    {0,   255, 0,   0},
    {0,   0,   255, 0},
    {128, 0,   0,   0},
    {0,   128, 0,   0},
    {0,   0,   128, 0},
    {128, 128, 0,   0},
    {0,   128, 128, 0},
    {128, 0,   128, 0},
    {192, 192, 192, 0},
};

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

SDL_AppResult SDLCALL SDL_AppInit(void **appstate, int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    SdlCrossState *state = SDL_calloc(1, sizeof(SdlCrossState));
    if (state == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate state");
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    const int linked_sdl_version = SDL_GetVersion();
    SDL_Log("We compiled against SDL version %u.%u.%u ...\n",
            SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("But we are linking against SDL version %u.%u.%u.\n",
            SDL_VERSIONNUM_MAJOR(linked_sdl_version), SDL_VERSIONNUM_MINOR(linked_sdl_version),
            SDL_VERSIONNUM_MICRO(linked_sdl_version));

#ifdef WITH_MIXER
    SDL_SetHint("SDL_MIXER_DISABLE_DRFLAC", "1");
    SDL_SetHint("SDL_MIXER_DISABLE_DRMP3", "1");
#endif

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed (%s)", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#if defined(WITH_IMAGE)
    {
        int v = IMG_Version();
        SDL_Log("SDL3_image version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v),
                SDL_VERSIONNUM_MICRO(v));
    }
#endif

#if defined(WITH_MIXER)
    {
        int v = MIX_Version();
        SDL_Log("SDL3_mixer version %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v),
                SDL_VERSIONNUM_MICRO(v));
    }

    if (!MIX_Init()) {
        SDL_Log("MIX_Init failed (%s)", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (state->mixer == NULL) {
        SDL_Log("Couldn't create mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec mixerspec;
    MIX_GetMixerFormat(state->mixer, &mixerspec);
    SDL_Log("Mixer is format %s, %d channels, %d frequency", SDL_GetAudioFormatName(mixerspec.format),
            mixerspec.channels, mixerspec.freq);

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
    state->audio = MIX_LoadAudio(state->mixer, audiofname, false);
    if (state->audio == NULL) {
        SDL_Log("Failed to load '%s' (%s)", audiofname, SDL_GetError());
    }
    if (state->audio) {
        SDL_AudioSpec audiospec;
        MIX_GetAudioFormat(state->audio, &audiospec);
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

    state->width = 640;
    state->height = 480;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
#if defined(SDL_PLATFORM_ANDROID)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

#ifdef SDL_PLATFORM_ANDROID
#define RECT_W 250
#else
#define RECT_W 50
#endif
    for (size_t i = 0; i < SDL_arraysize(state->locations); i++) {
        state->locations[i].valid = false;
        state->locations[i].rect.w = RECT_W;
        state->locations[i].rect.h = RECT_W;
    }
    state->fullscreen = false;
    state->foreground = true;

#ifdef WITH_BOX2D
    state->ppm = 30.0f;
    state->debugDrawer = b2DefaultDebugDraw();
    state->debugDrawer.drawShapes = true;
    state->debugDrawer.DrawSolidPolygonFcn = drawSolidPolygon;
    state->debugDrawer.DrawSolidCircleFcn = drawSolidCircle;
    state->debugDrawer.context = state;

    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = (b2Vec2){0.f, -10.f};
        state->worldId = b2CreateWorld(&worldDef);
    }

    {
        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.position = (b2Vec2){0.0f, -10.0f};
        b2BodyId groundBodyId = b2CreateBody(state->worldId, &groundBodyDef);
        b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);
        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        b2ShapeId groundShapeId = b2CreatePolygonShape(groundBodyId, &groundShapeDef, &groundBox);
        (void) groundShapeId;
    }

    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = (b2Vec2){0.0f, 4.0f};
        b2BodyId bodyId = b2CreateBody(state->worldId, &bodyDef);
        b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.material.friction = 0.3f;
        b2ShapeId boxShapeId = b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
        (void)boxShapeId;
    }
#endif
    char title[32];
    SDL_snprintf(title, sizeof(title), "An SDL %d.%d.%d window",
        SDL_VERSIONNUM_MAJOR(linked_sdl_version), SDL_VERSIONNUM_MINOR(linked_sdl_version),
        SDL_VERSIONNUM_MICRO(linked_sdl_version));
    state->window = SDL_CreateWindow(title, state->width, state->height, flags);

    if (state->window == NULL) {
        show_important_message(5, "Could not create window %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Log("Window created!");

    state->renderer = SDL_CreateRenderer(state->window, NULL);
    if (state->renderer == NULL) {
        show_important_message(5, "Could not create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    const SDL_PixelFormat *formats = SDL_GetPointerProperty(SDL_GetRendererProperties(state->renderer), SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, NULL);
    state->format = SDL_GetPixelFormatDetails(formats[0]);
    SDL_Log("Renderer created!");

    show_important_message(1, "Entering the loop");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDLCALL SDL_AppEvent(void *appstate, SDL_Event *event) {
    SdlCrossState *state = appstate;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_DISPLAY_ORIENTATION:
        switch (event->display.data1) {
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
        state->width = event->window.data1;
        state->height = event->window.data2;
        break;
    case SDL_EVENT_WINDOW_SHOWN:
        state->foreground = 1;
        break;
    case SDL_EVENT_WINDOW_HIDDEN:
        state->foreground = 0;
        break;
#if !defined(SDL_PLATFORM_ANDROID)
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button down: which=%d, [%g, %g]", event->button.which,
                     event->button.x, event->button.y);
        if (event->button.which < SDL_arraysize(state->locations)) {
            state->locations[event->button.which].valid = true;
            state->locations[event->button.which].rect.x = event->button.x - RECT_W / 2;
            state->locations[event->button.which].rect.y = event->button.y - RECT_W / 2;
        }
#if defined(WITH_MIXER)
        if (state->audio != NULL && !MIX_PlayAudio(state->mixer, state->audio)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to play audio (%s)", SDL_GetError());
        }
#endif
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button up: which=%d, [%g, %g]", event->button.which,
                     event->button.x, event->button.y);
        if (event->button.which < SDL_arraysize(state->locations)) {
            state->locations[event->button.which].valid = false;
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d", event->motion.which);
        if (event->button.which < SDL_arraysize(state->locations)) {
            state->locations[event->button.which].rect.x = event->motion.x - RECT_W / 2;
            state->locations[event->button.which].rect.y = event->motion.y - RECT_W / 2;
        }
        break;
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
        state->foreground = 0;
        break;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
        state->foreground = 1;
        break;
#endif
#if defined(SDL_PLATFORM_ANDROID)
    case SDL_EVENT_FINGER_DOWN:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "finger down: fingerID=%d, [%f, %f]", (int)event->tfinger.fingerID, event->tfinger.x, event->tfinger.y);
        if (event->tfinger.fingerID >= 0 && event->tfinger.fingerID < (int)SDL_arraysize(state->locations)) {
            state->locations[event->tfinger.fingerID].valid = true;
            state->locations[event->tfinger.fingerID].rect.x = state->width * event->tfinger.x - RECT_W/2;
            state->locations[event->tfinger.fingerID].rect.y = state->height * event->tfinger.y - RECT_W/2;
        }

#if defined(WITH_MIXER)
        // Play the sound effect
        MIX_PlayAudio(state->mixer, state->audio);
#endif

        break;
    case SDL_EVENT_FINGER_UP:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse button up: fingerID=%d, [%f, %f]", (int)event->tfinger.fingerID, event->tfinger.x, event->tfinger.y);
        if (event->tfinger.fingerID >= 0 && event->tfinger.fingerID < (int)SDL_arraysize(state->locations)) {
            state->locations[event->tfinger.fingerID].valid = false;
        }
        break;
    case SDL_EVENT_FINGER_MOTION:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "mouse move: button=%d", event->motion.which);
        if (event->tfinger.fingerID >= 0 && event->tfinger.fingerID < (int)SDL_arraysize(state->locations)) {
            state->locations[event->tfinger.fingerID].rect.x = state->width * event->tfinger.x - RECT_W/2;
            state->locations[event->tfinger.fingerID].rect.y = state->height * event->tfinger.y - RECT_W/2;
        }
        break;
    case SDL_EVENT_TERMINATING:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Received SDL_EVENT_TERMINATING");
        return SDL_APP_SUCCESS;
#endif
    case SDL_EVENT_KEY_UP:
        switch (event->key.key) {
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS;
        case SDLK_RETURN:
            if (event->key.mod & SDL_KMOD_ALT) {
                state->fullscreen = !state->fullscreen;
                SDL_SetWindowFullscreen(state->window, state->fullscreen);
            }
            break;
        }
        break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDLCALL SDL_AppIterate(void *appstate)
{
    SdlCrossState *const state = appstate;

    if (state->foreground) {
#ifdef WITH_BOX2D
        Uint64 now = SDL_GetTicks();
        while (now > state->nextTick) {
            static const float timeStep = 1.0f / 60.0f;
            static int subStepCount = 4;
            b2World_Step(state->worldId, timeStep, subStepCount);
            state->nextTick += (Uint64)(1000.0f * timeStep);
        }
#endif
        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
        SDL_RenderClear(state->renderer);
#ifdef WITH_BOX2D
        b2World_Draw(state->worldId, &state->debugDrawer);
#endif
        for (size_t i = 0; i < SDL_arraysize(state->locations); i++) {
            if (state->locations[i].valid) {
                SDL_SetRenderDrawColor(state->renderer, COLORS[i].r, COLORS[i].g, COLORS[i].b, COLORS[i].a);
                SDL_RenderFillRect(state->renderer, &state->locations[i].rect);
            }
        }
        SDL_RenderPresent(state->renderer);
    }
    return SDL_APP_CONTINUE;
}

void SDLCALL SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SdlCrossState *const state = appstate;
    (void) result;

    if (state == NULL) {
        return;
    }

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);

#if defined(WITH_BOX2D)
    b2DestroyWorld(state->worldId);
#endif

#if defined(WITH_MIXER)
    MIX_DestroyAudio(state->audio);
    MIX_DestroyMixer(state->mixer);
    MIX_Quit();
#endif

    SDL_free(state);

    SDL_Quit();
}
