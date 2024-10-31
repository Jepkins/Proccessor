#include <iostream>
#include <stdint.h>
#include <assert.h>
#include "memcanvas.h"

using namespace std;

#define SCALE(val)   ((val) * (int)scl)
#define UNSCALE(val) ((val) / (int)scl)

int MemCanvas::Init(pix_t* pixs_ptr, int des_width, int des_height, size_t scale, const char* name)
{
    if (inited)
        return -1;
    if (des_width <= 0 || des_height <= 0 || scale <= 0)
        return -2;
    if (pixs_ptr == nullptr)
        return -3;

    width = des_width;
    height = des_height;
    pixs = (pix_t*) pixs_ptr;
    scl = scale;

    current = (pix_t*)calloc((size_t)(width * height), sizeof(pix_t));
    assert(current && "Allocation error");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;

    win = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCALE(width), SCALE(height), SDL_WINDOW_SHOWN);
    rndr = SDL_CreateRenderer(win, -1, 0);
    SDL_SetRenderDrawColor(rndr, 0, 0, 0, 0);
    SDL_RenderClear(rndr);
    if (win == NULL)
        return 1;

    srf = SDL_GetWindowSurface(win);

    inited = true;
    return 0;
}

int MemCanvas::Quit()
{
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(rndr);
    SDL_Quit();
    free(current);
    inited = false;
    return 0;
}

int MemCanvas::Update()
{
    if (!inited)
        return 1;
    int ind = 0;
    const int CMP_BLOCK = 32;
    for (; ind <  width * height - CMP_BLOCK; ind += CMP_BLOCK)
    {
        if (memcmp(&current[ind], &pixs[ind], (size_t)CMP_BLOCK * sizeof(pix_t)) != 0)
        {
            for (int i = 0; i < CMP_BLOCK; i++)
            {
                pix_t new_pix = pixs[ind+i];
                const SDL_Rect rect = {SCALE((ind+i) / height), SCALE((ind+i) % height), (int)scl, (int)scl};
                SDL_FillRect(srf, &rect, new_pix);
                current[ind+i] = new_pix;
            }
        }
    }
    for (int i = 0; i < width * height - ind; i++)
    {
        pix_t new_pix = pixs[ind+i];
        const SDL_Rect rect = {SCALE((ind+i) / height), SCALE((ind+i) % height), (int)scl, (int)scl};
        SDL_FillRect(srf, &rect, new_pix);
        current[ind+i] = new_pix;
    }
    SDL_UpdateWindowSurface(win);
    return 0;
}

#undef SCALE
#undef UNSCALE
