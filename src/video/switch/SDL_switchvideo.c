#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_SWITCH

/* SDL internals */
#include "../SDL_sysvideo.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_keyboard_c.h"

#include<libtransistor/nx.h>
#include<libtransistor/display/display.h>
#include<libtransistor/gfx/gfx.h>

static int SWITCH_VideoInit(_THIS);
static int SWITCH_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode);
static void SWITCH_VideoQuit(_THIS);

static int SWITCH_Available(void) {
	return 1;
}

static void SWITCH_DeleteDevice(SDL_VideoDevice *device) {
	SDL_free(device);
}

static SDL_VideoDevice *SWITCH_CreateDevice(int devindex) {
	SDL_VideoDevice *device;

	device = (SDL_VideoDevice*) SDL_calloc(1, sizeof(SDL_VideoDevice));
	if(!device) {
		SDL_OutOfMemory();
		if(device) {
			SDL_free(device);
		}
		return NULL;
	}

	device->VideoInit = SWITCH_VideoInit;
	device->VideoQuit = SWITCH_VideoQuit;
	device->SetDisplayMode = SWITCH_SetDisplayMode;
	device->PumpEvents = SWITCH_PumpEvents;
	device->CreateWindowFramebuffer = SWITCH_CreateWindowFramebuffer;
	device->UpdateWindowFramebuffer = SWITCH_UpdateWindowFramebuffer;
	device->DestroyWindowFramebuffer = SWITCH_DestroyWindowFramebuffer;

	device->free = SWITCH_DeleteDevice;

	return device;
}

VideoBootStrap SWITCH_bootstrap = {
	"Switch", "Video driver for Nintendo Switch (libtransistor)",
	SWITCH_Avaialable, SWITCH_CreateDevice
};

int SWITCH_VideoInit(_THIS) {
	SDL_DisplayMode mode;

	result_t r;
	if((r = display_init()) != RESULT_OK) {
		return -1;
	}
	
	mode.format = SDL_PIXELFORMAT_RGBA8888;
	mode.w = 1280;
	mode.h = 720;
	mode.refresh_rate = 60;
	mode.driverdata = NULL;
	if(SDL_AddBasicVideoDisplay(&mode) < 0) {
		return -1;
	}

	SDL_AddDisplayMode(&_this->displays[0], &mode);
	return 0;
}

void SWITCH_VideoQuit(_THIS) {
	display_finalize();
}

int SWITCH_SetDisplayMode(_THIS) {
	return 0;
}

int SWITCH_PumpEvents(_THIS) {
	// TODO
}

#define SWITCH_DATA "_SDL_SwitchData"

typedef struct {
	SDL_Surface *sdl_surface;
	surface_t libtransistor_surface;
} SWITCH_WindowData;

int SWITCH_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch) {
	SDL_Surface *surface;
	const Uint32 surface_format = SDL_PIXELFORMAT_RGBA8888;
	int w, h;
	int bpp;
	Uint32 Rmask, Gmask, Bmask, Amask;
	
	surface = (SDL_Surface*) SDL_GetWindowData(window, SWITCH_SURFACE);
	SDL_FreeSurface(surface);

	SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
	SDL_GetWindowSize(window, &w, &h);
	surface = SDL_CreateRGBASurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);
	if(!surface) {
		return -1;
	}

	SWITCH_WindowData *data = SDL_calloc(1, sizeof(SWITCH_WindowData));
	data->sdl_surface = surface;
	display_open_layer(&data->libtransistor_surface);
	
	SDL_SetWindowData(window, SWITCH_DATA, data);
	*format = surface_format;
	*pixels = surface->pixels;
	*pitch = surface->pitch;
}

int SWITCH_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects) {
	SWITCH_WindowData *data;

	data = (SWITCH_WindowData*) SDL_GetWindowData(window, SWITCH_DATA);
	if(!data) {
		return SDL_SetError("Couldn't find switch data for window");
	}

	SDL_Surface *sdl_surface = data->sdl_surface;
	surface_t *lt_surface = &data->libtransistor_surface;
	uint32_t *out_buffer = NULL;
	
	result_t r;
	if((r = surface_dequeue_buffer(lt_surface, &out_buffer)) != RESULT_OK) {
		return -1;
	}
	
	gfx_slow_swizzling_blit(out_buffer, sdl_surface->pixels, sdl_surface->w, sdl_surface->h, 0, 0);

	if((r = surface_queue_buffer(lt_surface)) != RESULT_OK) {
		return -1;
	}

	return 0;
}

int SWITCH_DestroyWindowFramebuffer(_THIS) {
	SWITCH_WindowData *data;

	data = (SWITCH_WindowData*) SDL_GetWindowData(window, SWITCH_DATA);
	SDL_FreeSurface(data->sdl_surface);
	// TODO: destroy libtransistor surface
	SDL_free(data);
}

#endif /* SDL_VIDEO_DRIVER_SWITCH */
