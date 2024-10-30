#include <iostream>
#include <stdint.h>
#include <assert.h>
#include "memcanvas.h"

using namespace std;

#define GET_R(p) (uint8_t)((p & R_MASK)      )
#define GET_G(p) (uint8_t)((p & G_MASK) >>  8)
#define GET_B(p) (uint8_t)((p & B_MASK) >> 16)
#define GET_O(p) (uint8_t)((p & O_MASK) >> 24)

#define GET_CLR(p) {GET_R(p), GET_G(p), GET_B(p), GET_O(p)}

#define SCALE(val) (decltype(val))((double)val * scl)
#define UNSCALE(val) (decltype(val))((double)val / scl)

int MemCanvas::Init(pix_t* pixs_ptr, int des_width, int des_height, const char* name)
{
    if (inited)
        return -1;
    if (des_width <= 0 || des_height <= 0)
        return -2;
    if (pixs_ptr == nullptr)
        return -3;

    width = des_width;
    height = des_height;
    pixs = (pix_t*) pixs_ptr;

    current = (pix_t*)calloc((size_t)(width * height), sizeof(pix_t));
    assert(current && "Allocation error");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;

    win = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
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

#define CMP_BLOCK 16
int MemCanvas::Update()
{
    if (!inited)
        return 1;
    int ind = 0;
    for (; ind <  width * height - CMP_BLOCK; ind += CMP_BLOCK)
    {
        if (memcmp(&current[ind], &pixs[ind], CMP_BLOCK * sizeof(pix_t)) != 0)
        {
            for (int i = 0; i < CMP_BLOCK; i++)
            {
                pix_t new_pix = pixs[ind+i];
                clr_t clr = GET_CLR(new_pix);
                SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, clr.o);
                SDL_RenderDrawPoint(rndr, (ind+i) / height, (ind+i) % height);
                current[ind+i] = new_pix;
            }
        }
    }
    for (int i = 0; i < width * height - ind; i++)
    {
        pix_t new_pix = pixs[ind+i];
        clr_t clr = GET_CLR(new_pix);
        SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, clr.o);
        SDL_RenderDrawPoint(rndr, (ind+i) / height, (ind+i) % height);
        current[ind+i] = new_pix;
    }
    SDL_RenderPresent(rndr);
    return 0;
}

#undef GET_R
#undef GET_G
#undef GET_B
#undef GET_O

#undef GET_CLR

#undef SCALE
#undef UNSCALE
