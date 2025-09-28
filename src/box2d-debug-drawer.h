#ifndef BOX2D_DEBUG_DRAWER_H
#define BOX2D_DEBUG_DRAWER_H

#include "context.h"

#include <SDL3/SDL.h>

static void drawSolidPolygon(b2Transform transform, const b2Vec2 *vertices, int vertexCount, float radius, b2HexColor color, void *context)
{
    SdlCrossState *state = context;
    (void)radius;
    (void)context;

    // Extract RGB
    Uint8 r, g, b;
    SDL_GetRGB(color, state->format, NULL, &r, &g, &b);
    // Draw a collider rectangle with lines
    SDL_SetRenderDrawColor(state->renderer, r, g, b, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < vertexCount; ++i)
    {
        int next_index = (i + 1 == vertexCount) ? 0 : i + 1;
        b2Vec2 p0 = b2TransformPoint(transform, vertices[i]);
        b2Vec2 p1 = b2TransformPoint(transform, vertices[next_index]);
        float x0 = p0.x * state->ppm;
        float y0 = p0.y * state->ppm;
        float x1 = p1.x * state->ppm;
        float y1 = p1.y * state->ppm;
        SDL_RenderLine(state->renderer, state->width / 2 + x0, state->height / 2 - y0, state->width / 2 + x1, state->height / 2 - y1);
    }

    // --- Tip: Draw orientation line ---
    // Compute polygon center
    float cx = 0.f, cy = 0.f;
    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 p = b2TransformPoint(transform, vertices[i]);
        cx += p.x;
        cy += p.y;
    }
    cx = (cx / vertexCount) * state->ppm;
    cy = (cy / vertexCount) * state->ppm;

    // Midpoint of second edge (vertex[1] → vertex[2])
    b2Vec2 p1 = b2TransformPoint(transform, vertices[1]);
    b2Vec2 p2 = b2TransformPoint(transform, vertices[2]);
    float mx = (p1.x + p2.x) * 0.5f * state->ppm;
    float my = (p1.y + p2.y) * 0.5f * state->ppm;

    // Draw center → edge midpoint line (acts like "direction tip")
    SDL_RenderLine(state->renderer,
        state->width / 2 + cx, state->height / 2 - cy,
        state->width / 2 + mx, state->height / 2 - my);
}

static void drawSolidCircle(b2Transform transform, float radius, b2HexColor color, void *context)
{
    SdlCrossState *state = context;
    (void)radius;
    (void)context;

    float angle = 0.f;
    const int numberOfSegments = 20;
    const float angleStep = 360.f / numberOfSegments;

    // Extract RGB
    Uint8 r, g, b;
    SDL_GetRGB(color, state->format, NULL, &r, &g, &b);
    // Draw a collider rectangle with lines
    SDL_SetRenderDrawColor(state->renderer, r, g, b, SDL_ALPHA_OPAQUE);

    float x = radius * cos(angle * SDL_PI_F / 180.f);
    float y = radius * sin(angle * SDL_PI_F / 180.f);
    b2Vec2 p0 = b2TransformPoint(transform, (b2Vec2){ x, y });
    float x0 = p0.x * state->ppm;
    float y0 = p0.y * state->ppm;
    angle += angleStep;

    for (int i = 0; i < numberOfSegments; ++i)
    {
        float x = radius * cos(angle * SDL_PI_F / 180.f);
        float y = radius * sin(angle * SDL_PI_F / 180.f);
        b2Vec2 p1 = b2TransformPoint(transform, (b2Vec2){ x, y });
        float x1 = p1.x * state->ppm;
        float y1 = p1.y * state->ppm;
        SDL_RenderLine(state->renderer, state->width / 2 + x0, state->height / 2 - y0, state->width / 2 + x1, state->height / 2 - y1);
        x0 = x1;
        y0 = y1;
        angle += angleStep;
        if (angle >= 360.f)
        {
            angle = 0.f;
        }
    }

    // --- Tip: Draw orientation line ---
    // Circle center
    b2Vec2 c = b2TransformPoint(transform, (b2Vec2){ 0.f, 0.f });
    float cx = c.x * state->ppm;
    float cy = c.y * state->ppm;

    // Direction = transform applied to (radius, 0)
    b2Vec2 orient = b2TransformPoint(transform, (b2Vec2){ radius, 0.f });
    float ox = orient.x * state->ppm;
    float oy = orient.y * state->ppm;

    // Draw center → orientation marker
    SDL_RenderLine(state->renderer,
        state->width / 2 + cx, state->height / 2 - cy,
        state->width / 2 + ox, state->height / 2 - oy);
}

#endif /* BOX2D_DEBUG_DRAWER_H */
