#include <iostream>
#include <stdint.h>
#include "memcanvas.h"

using namespace std;

#define GET_R(p) (uint8_t)((p & R_MASK)      )
#define GET_G(p) (uint8_t)((p & G_MASK) >> 8 )
#define GET_B(p) (uint8_t)((p & B_MASK) >> 16)
#define GET_O(p) (uint8_t)((p & O_MASK) >> 24)

#define GET_CLR(p) {GET_R(p), GET_G(p), GET_B(p), GET_O(p)}

int MemCanvas::Init(pix_t* pixs_ptr, int des_width, int des_height, const char* name)
{
    if (inited)
        return -1;
    if (des_width <= 0 && des_height <= 0)
        return -2;
    if (pixs_ptr == nullptr)
        return -3;

    width = des_width;
    height = des_height;
    pixs = (pix_t*) pixs_ptr;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "Can't init: " << SDL_GetError() << endl;
        system("pause");
        return 1;
    }

    win = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    rndr = SDL_CreateRenderer(win, -1, 0);
    SDL_SetRenderDrawColor(rndr, 0, 0, 0, 0);
    SDL_RenderClear(rndr);
    if (win == NULL) {
        cout << "Can't create window: " << SDL_GetError() << endl;
        system("pause");
        return 1;
    }

    scr = SDL_GetWindowSurface(win);

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
    for (int i = 0; i <  width; i++)
    for (int j = 0; j < height; j++)
    {
        pix_t pix = pixs[i * height + j];
        clr_t clr = GET_CLR(pix);
        SDL_SetRenderDrawColor(rndr, clr.r, clr.g, clr.b, clr.o);
        SDL_RenderDrawPoint(rndr, i, j);
    }
    SDL_RenderPresent(rndr);
    return 0;
}
