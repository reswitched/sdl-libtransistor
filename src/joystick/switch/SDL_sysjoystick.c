/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "../../SDL_internal.h"

#if SDL_JOYSTICK_SWITCH

#include <libtransistor/nx.h>

#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#include "SDL_assert.h"
#include "SDL_events.h"

#define JOYSTICK_MAX (0x7FFF)
#define JOYSTICK_MIN (-0x8000)

static hid_controller_button_mask_t button_map[] = {
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_LSTICK,
    BUTTON_RSTICK,
    BUTTON_L,
    BUTTON_R,
    BUTTON_ZL,
    BUTTON_ZR,
    BUTTON_PLUS,
    BUTTON_MINUS,
    BUTTON_LEFT,
    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_LSTICK_LEFT,
    BUTTON_LSTICK_UP,
    BUTTON_LSTICK_DOWN,
    BUTTON_LSTICK_RIGHT,
    BUTTON_RSTICK_LEFT,
    BUTTON_RSTICK_UP,
    BUTTON_RSTICK_DOWN,
    BUTTON_RSTICK_RIGHT,
    BUTTON_SL,
    BUTTON_SR
};

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * This function should return the number of available joysticks, or -1
 * on an unrecoverable fatal error.
 */
int SDL_SYS_JoystickInit(void) {
    result_t result = hid_init();
    if(result != RESULT_OK) {
        return -1;
    }

    return 1;
}

/* Function to return the number of joystick devices plugged in right now */
int SDL_SYS_NumJoysticks(void) {
    return 1; // TODO: Detect joycon number. Look at 'controller_state' bit0 or bit1
}

/* Function to cause any queued joystick insertions to be processed */
void SDL_SYS_JoystickDetect(void) {
    // TODO
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickNameForDeviceIndex(int device_index) {
    return "Nintendo Switch JoyCon Controller";
}

/* Function to get the current instance id of the joystick located at device_index */
SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index) {
    return device_index;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index) {
    joystick->nbuttons = sizeof(button_map) / sizeof(hid_controller_button_mask_t);
    joystick->naxes = 4;
    joystick->nhats = 0;
    return 0;
}

/* Function to query if the joystick is currently attached
 * It returns SDL_TRUE if attached, SDL_FALSE otherwise.
 */
SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick * joystick) {
    return SDL_TRUE;
}

/* Function to fetch the controller specified by the index.
 * It returns a hid_controller_t if index is valid, NULL otherwise
 */
static hid_controller_t *GetHidController(int idx) {
    SDL_assert(idx >= 0 && idx < 9);
    return &hid_get_shared_memory()->controllers[idx];
}

static int32_t clamp(int32_t val, int32_t min, int32_t max) {
    return val < min ? min : (val > max ? max : val);
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick * joystick) {
    static int16_t x1_old = 0;
    static int16_t y1_old = 0;
    static int16_t x2_old = 0;
    static int16_t y2_old = 0;

    hid_controller_t *joycon = GetHidController(0);
    hid_controller_t* main = GetHidController(8);

    hid_controller_state_entry_t ent_joycon = 
        joycon->main.entries[joycon->main.latest_idx];
    hid_controller_state_entry_t ent_main = 
        main->main.entries[main->main.latest_idx];

    uint32_t x1_joycon = ent_joycon.left_stick_x;
    uint32_t y1_joycon = ent_joycon.left_stick_y;
    uint32_t x2_joycon = ent_joycon.right_stick_x;
    uint32_t y2_joycon = ent_joycon.right_stick_y;

    uint32_t x1_main = ent_main.left_stick_x;
    uint32_t y1_main = ent_main.left_stick_y;
    uint32_t x2_main = ent_main.right_stick_x;
    uint32_t y2_main = ent_main.right_stick_y;

    uint32_t ux1 = x1_joycon | x1_main;
    uint32_t uy1 = y1_joycon | y1_main;
    uint32_t ux2 = x2_joycon | x2_main;
    uint32_t uy2 = y2_joycon | y2_main;

    int16_t x1 = (int16_t) clamp(ux1, JOYSTICK_MIN, JOYSTICK_MAX);
    int16_t y1 = (int16_t) clamp(uy1, JOYSTICK_MIN, JOYSTICK_MAX);
    int16_t x2 = (int16_t) clamp(ux2, JOYSTICK_MIN, JOYSTICK_MAX);
    int16_t y2 = (int16_t) clamp(uy2, JOYSTICK_MIN, JOYSTICK_MAX);


    if(x1 != x1_old) {
        SDL_PrivateJoystickAxis(joystick, 0, x1);
        x1_old = x1;
    }
    if(y1 != y1_old) {
        SDL_PrivateJoystickAxis(joystick, 1, y1);
        y1_old = y1;
    }
    if(x2 != x2_old) {
        SDL_PrivateJoystickAxis(joystick, 2, x2);
        x2_old = x2;
    }
    if(y2 != y2_old) {
        SDL_PrivateJoystickAxis(joystick, 3, y2);
        y2_old = y2;
    }

    int i;
    for(i = 0; i < joystick->nbuttons; i++) {
        SDL_PrivateJoystickButton(
            joystick, i,
            (hid_controller_buttons_down(joycon, button_map[i]) ||  
             hid_controller_buttons_down(main, button_map[i])) ?
            SDL_PRESSED : SDL_RELEASED
        );
    }
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick * joystick) {
    hid_finalize();
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void) {
}

/* Function to convert the hid_controller mac address into a JoystickGUID */ 
static SDL_JoystickGUID GetDeviceMacAddressAsGUID(int device_index) {
    SDL_JoystickGUID guid;
    hid_controller_t *controller = GetHidController(device_index);

    SDL_zero(guid);
    SDL_memcpy(&guid, controller->mac, sizeof(hid_mac));

    return guid;
}

/* Function to return the stable GUID for a plugged in device */
SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID(int device_index) {
    return GetDeviceMacAddressAsGUID(device_index);
}

/* Function to return the stable GUID for a opened joystick */
SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick) {
    return GetDeviceMacAddressAsGUID(joystick->instance_id);
}

#endif
