/*
  sdl/sdlDisplayDriver.c

  (c) 2016 Rob Anderberg

  Standard display device implementation for truecolour X11 displays

*/
#include "../armdefs.h"
#include "armarc.h"
#include "arch/keyboard.h"
#include "archio.h"
#include "../armemu.h"
#include "arch/displaydev.h"

#include "platform.h"

//  SDD_HostColour
//  - Data type used to store a colour value. E.g.
//    "typedef unsigned short SDD_HostColour".
typedef uint32_t SDD_HostColour;

// SDD_Name(x)
//  - Macro used to convert symbol name 'x' into an instance-specific version
//    of the symbol. e.g.
//    "#define SDD_Name(x) MySDD_##x"

#define SDD_Name(x) sdl_##x
// #define SDD_Stats TRUE
// #define VIDEO_STATS TRUE

// SDD_RowsAtOnce
//  - The number of source rows to process per update. This can be a non-const
//    variable if you want, so it can be tweaked on the fly to tune performance
static const int SDD_RowsAtOnce = 1;

// SDD_Row
//  - A data type that acts as an iterator/accessor for a screen row. It can be
//    anything from a simple *SDD_HostColour (for hosts with direct framebuffer
//    access, e.g. RISC OS), or something more complex like a struct which keeps
//    track of the current drawing coordinates (e.g. the truecolour X11 driver)
//  - SDD_Row instances must be able to cope with being passed by value as
//    function parameters.
typedef SDD_HostColour *SDD_Row;

uint32_t pixelFromRGB(int red, int green, int blue)
{
   return red << 16 | green << 8 | blue;
}

void putpixel(uint32_t *pixels, int x, int y, int width, int height, Uint32 targetPixel)
{
   pixels[y * width + x] = targetPixel;
}

int count = 0;

/* ------------------------------------------------------------------ */

/* X uses 16-bit RGB amounts.  If we've an Arcem one that's 4-bit, we
 * need to scale it up so 0x0 remains 0x0000 and 0xf becomes 0xffff,
 * with ther other fourteen values being evenly spread inbetween. */

// unsigned int vidc_col_to_x_col(unsigned int col)
// {
//    unsigned int r = (col & 0xf) * 0x1111;
//    unsigned int g = ((col & 0xf0) >> 4) * 0x1111;
//    unsigned int b = ((col & 0xf00) >> 8) * 0x1111;
//    return get_pixelval(r, g, b);
// }

// void SDD_Name(Host_PollDisplay)(ARMul_State *state)
//  - A function that the driver will call at the start of each frame.
void SDD_Name(Host_PollDisplay)(ARMul_State *state)
{
   //printf("Hello!");

   // printf("Host_PollDisplay\n");

   SDL_Event e;

   while (SDL_PollEvent(&e) != 0)
   {
      //User requests quit
      if (e.type == SDL_QUIT)
      {
         SDL_DestroyRenderer(PD.renderer);
         SDL_DestroyWindow(PD.window);
         SDL_DestroyTexture(PD.texture);
         SDL_Quit();
         exit(0);
      }
   }

   if (PD.window && PD.renderer && PD.texture && PD.pixels)
   {
      // int textureWidth, textureHeight;
      // SDL_QueryTexture(PD.texture, NULL, NULL, &textureHeight, &textureWidth);

      SDL_SetRenderDrawColor(PD.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(PD.renderer);

      // uint32_t *dst;
      // uint32_t color;
      // for (int row = 0; row < PD.height; ++row)
      // {
      //    dst = (uint32_t *)((uint8_t *)PD.pixels + row * PD.pitch);
      //    for (int col = 0; col < PD.width; ++col)
      //    {
      //       color = col % 10 ? pixelFromRGB(0, 0, 0) : pixelFromRGB(col % 128 + 127, 0, col % 128 + 127);
      //       *dst++ = color;
      //    }
      // }
      SDL_UnlockTexture(PD.texture);
      SDL_RenderCopy(PD.renderer, PD.texture, NULL, NULL);
      SDL_RenderPresent(PD.renderer);
   }

   if (SDL_LockTexture(PD.texture, NULL, &PD.pixels, &PD.pitch) < 0)
   {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
      // quit(5);
   }
}

// SDD_HostColour SDD_Name(Host_GetColour)(ARMul_State *state,uint_fast16_t col)
//  - A function that the driver will call in order to convert a 13-bit VIDC
//    physical colour into a host colour.
SDD_HostColour SDD_Name(Host_GetColour)(ARMul_State *state, uint_fast16_t col)
{
   // I think the 13-bit VIDC values are like this:
   // ? bbbb gggg rrrr
   // and we need to convert to 8-bit RGB values, so we mask off each component, then
   // shift them left by 4 positions.
   int r = (col & 0x00f) << 4;
   int g = (col & 0x0f0);
   int b = (col & 0xf00) >> 4;
   printf("Host_GetColour: %d %d %d, ", r, g, b);

   return r << 16 | g << 8 | b;
}

// SDD_Row SDD_Name(Host_BeginRow)(ARMul_State *state,int row,int offset)
//  - Function to return a SDD_Row instance suitable for accessing the indicated
//    row, starting from the given X offset
SDD_Row SDD_Name(Host_BeginRow)(ARMul_State *state, int row, int offset)
{
   // printf("Host_BeginRow, ");
   return (uint32_t *)((uint8_t *)PD.pixels + row * PD.pitch);
}

// void SDD_Name(Host_EndRow)(ARMul_State *state,SDD_Row *row)
//  - Function to end the use of a SDD_Row
//  - Where a SDD_Row has been copied via pass-by-value, currently only the
//    original instance will have Host_EndRow called on it.
void SDD_Name(Host_EndRow)(ARMul_State *state, SDD_Row *row)
{
   // printf("Host_EndRow, ");
   // Nothing here
}

// void SDD_Name(Host_BeginUpdate)(ARMul_State *state,SDD_Row *row,
//                                 unsigned int count)
//  - Function called when the driver is about to write to 'count' pixels of the
//    row. Implementations could use it for tracking dirty regions in the
//    display window.
void SDD_Name(Host_BeginUpdate)(ARMul_State *state, SDD_Row *row, unsigned int count)
{
   // printf("Host_BeginUpdate, ");
}
// void SDD_Name(Host_EndUpdate)(ARMul_State *state,SDD_Row *row)
//  - Function called once the driver has finished the write operation
void SDD_Name(Host_EndUpdate)(ARMul_State *state, SDD_Row *row)
{
   // printf("Host_EndUpdate, ");
}
// void SDD_Name(Host_SkipPixels)(ARMul_State *state,SDD_Row *row,
//                                unsigned int count)
//  - Function to skip forwards 'count' pixels in the row
void SDD_Name(Host_SkipPixels)(ARMul_State *state, SDD_Row *row, unsigned int count)
{
   // printf("Host_SkipPixels, ");
   (*row) += count;
}

// void SDD_Name(Host_WritePixel)(ARMul_State *state,SDD_Row *row,
//                                SDD_HostColour col)
//  - Function to write a single pixel and advance to the next location. Only
//    called between BeginUpdate & EndUpdate.
void SDD_Name(Host_WritePixel)(ARMul_State *state, SDD_Row *row, SDD_HostColour pix)
{
   // printf("WP, ");
   *(*row)++ = pix;
}

// void SDD_Name(Host_WritePixels)(ARMul_State *state,SDD_Row *row,
//                                 SDD_HostColour col,unsigned int count)
//  - Function to fill N adjacent pixels with the same colour. 'count' may be
//    zero. Only called between BeginUpdate & EndUpdate.
void SDD_Name(Host_WritePixels)(ARMul_State *state, SDD_Row *row, SDD_HostColour pix, unsigned int count)
{
   // printf("WPS, ");
   while (count--)
   {
      *(*row)++ = pix;
   }
}

// SDD_Stats
//  - Define this to enable the stats code.
//#define SDD_Stats

// SDD_DisplayDev
//  - The name to use for the const DisplayDev struct that will be generated
#define SDD_DisplayDev sdl_DisplayDev

static void SDD_Name(Host_ChangeMode)(ARMul_State *state, int width, int height, int hz);

#include "../arch/stddisplaydev.c"

// void SDD_Name(Host_ChangeMode)(ARMul_State *state,int width,int height,
//                                int hz)
//  - A function that the driver will call whenever the display timings have
//    changed enough to warrant a mode change.
//  - The implementation must change to the most appropriate display mode
//    available and fill in the Width, Height, XScale and YScale members of the
//    HostDisplay struct to reflect the new display parameters.
//  - Currently, failure to find a suitable mode isn't supported - however it
//    shouldn't be too hard to keep the screen blanked (not ideal) by keeping
//    DC.ModeChanged set to 1
void SDD_Name(Host_ChangeMode)(ARMul_State *state, int width, int height, int hz)
{
   HD.Width = width;
   HD.Height = height;
   HD.XScale = 1;
   HD.YScale = 1;
   /* Try and detect rectangular pixel modes */
   if ((width >= height * 2))
   {
      HD.YScale = 2;
      HD.Height *= 2;
   }
   else if ((height >= width))
   {
      HD.XScale = 2;
      HD.Width *= 2;
   }

   ChangeRenderResolution(HD.Width, HD.Height);
   // resizeWindow(HD.Width, HD.Height);
   // /* Screen is expected to be cleared */
   // memset(dibbmp, 0, sizeof(SDD_HostColour) * MonitorWidth * MonitorHeight);

   printf("Host_ChangeMode, ");
}