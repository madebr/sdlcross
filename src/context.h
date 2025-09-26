#ifndef CONTEXT_H
#define CONTEXT_H

#include <SDL3/SDL.h>
#ifdef WITH_MIXER
#include <SDL3_mixer/SDL_mixer.h>
#endif
#ifdef WITH_BOX2D
#include <box2d/box2d.h>
#endif


typedef struct {
    int width;
    int height;
    struct {
        int valid;
        SDL_FRect rect;
    } locations[10];
    SDL_Window *window;
    SDL_Renderer *renderer;
    const SDL_PixelFormatDetails *format;
#ifdef WITH_MIXER
    MIX_Mixer *mixer;
    MIX_Audio *audio;
#endif
#ifdef WITH_BOX2D
    Uint64 nextTick;
    float ppm;
    b2WorldId worldId;
    b2DebugDraw debugDrawer;
#endif
    bool foreground;
    bool fullscreen;
} SdlCrossState;

#endif /* CONTEXT_H */
