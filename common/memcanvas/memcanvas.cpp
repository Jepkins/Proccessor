#include <iostream>
#include <stdint.h>
#include "memcanvas.h"

using namespace std;

#define GET_R(p) (uint8_t)((p & R_MASK)      )
#define GET_G(p) (uint8_t)((p & G_MASK) >>  8)
#define GET_B(p) (uint8_t)((p & B_MASK) >> 16)
#define GET_O(p) (uint8_t)((p & O_MASK) >> 24)

#define GET_CLR(p) {GET_R(p), GET_G(p), GET_B(p), GET_O(p)}

#define SCALE(val) (decltype(val))((double)val * scl)
#define UNSCALE(val) (decltype(val))((double)val / scl)

int MemCanvas::Init(pix_t* pixs_ptr, int des_width, int des_height, double scale, const char* name)
{
    if (inited)
        return -1;
    if (des_width <= 0 && des_height <= 0)
        return -2;
    if (scale < 0.1)
        return -2;
    if (pixs_ptr == nullptr)
        return -3;

    width = des_width;
    height = des_height;
    pixs = (pix_t*) pixs_ptr;
    scl = scale;

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

    inited = false;
    return 0;
}

int MemCanvas::Update()
{
    for (int i = 0; i <  SCALE(width); i++)
    for (int j = 0; j < SCALE(height); j++)
    {
        pix_t pix = pixs[UNSCALE(i) * height + UNSCALE(j)];
        clr_t clr = GET_CLR(pix);
        SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, clr.o);
        SDL_RenderDrawPoint(rndr, i, j);
        // const SDL_Rect rect = {SCALE(i), SCALE(j), (int)round(scl), (int)round(scl)};
        // SDL_FillRect(srf, &rect, pix);
    }
    SDL_RenderPresent(rndr);
    // SDL_UpdateWindowSurface(win);
    return 0;
}

#undef GET_R
#undef GET_G
#undef GET_B
#undef GET_O

#undef GET_CLR

#undef SCALE
#undef UNSCALE
