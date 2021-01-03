/* (c) David Alan Gilbert 1995-1999 - see Readme file for copying info
   Hacked about with for new display driver interface by Jeffrey Lee, 2011 */
/* Display and keyboard interface for the Arc emulator */

/*#define DEBUG_VIDCREGS */
/*#define DEBUG_KBD */
/*#define DEBUG_MOUSEMOVEMENT */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>


#include "armdefs.h"
#include "arch/armarc.h"
#include "arch/keyboard.h"
#include "arch/archio.h"
#include "arch/hdc63463.h"
#ifdef SOUND_SUPPORT
#include "arch/sound.h"
#endif
#include "arch/displaydev.h"

#include "ControlPane.h"

#include "SDL.h"
#include "platform.h"

extern const DisplayDev sdl_DisplayDev;

struct plat_display PD;

const int initialWindowWidth = 640;
const int initialWindowHeight = 480;

int DisplayDev_Init(ARMul_State *state)
{
	SDL_CreateWindowAndRenderer(initialWindowWidth, initialWindowHeight, SDL_WINDOW_RESIZABLE, &PD.window, &PD.renderer);
	// SDL_SetWindowTitle(&PD.window, "ArcEm");
	ChangeRenderResolution(initialWindowWidth, initialWindowHeight);
	SDL_GL_SetSwapInterval(1);
	return DisplayDev_Set(state, &sdl_DisplayDev);
}

int Kbd_PollHostKbd(ARMul_State *state)
{
}

int ChangeRenderResolution(int width, int height) {
	
	if (PD.texture) {
		SDL_DestroyTexture(PD.texture);
	}
	
	if (!PD.renderer) {
		printf("ChangeRenderResolution:Renderer is invalid, can't set new resolution");
	}
	
	PD.texture = SDL_CreateTexture(PD.renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
	
	if(!PD.texture) {
		printf("ChangeRenderResolution: %s", SDL_GetError());
	}
	
	int textureWidth, textureHeight;
	SDL_QueryTexture(PD.texture, NULL, NULL, &textureWidth, &textureHeight);
	
	printf("Texture: %d %d\n", textureWidth, textureHeight);
	
	printf("ChangeRenderResolution: changed to: %d, %d\n", width, height);
	
	PD.width = width;
	PD.height = height;
}