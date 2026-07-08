// OpenW3D @feature Cross-platform input backend.
// On non-Windows builds (OPENW3D_SDL3 defined), this file replaces the
// DirectInput-based implementation with SDL3's event polling. The DirectInput
// class API surface stays identical so callers in input.cpp etc. don't change.
//
// Event flow: SDL3 events are pumped by the main game loop. Keyboard/mouse
// state is updated incrementally and exposed through the same static
// DIKeyboardButtons / DIMouseButtons / DIMouseAxis / DIJoystickButtons
// arrays that the original DirectInput class used.
//
// Notes on parity:
//  - Keyboard: 107 DIK_* codes (full US layout). The mapping table below
//    converts SDL_Keycode -> DIK_* for everything the codebase uses.
//  - Mouse: 3 buttons (left/right/center), 3 axes (X/Y/wheel).
//  - Joystick: 2 buttons + 2 axes. SDL3 gamepad mapped if available.
//  - DI_BUTTON_HELD / DI_BUTTON_HIT / DI_BUTTON_RELEASED transitions are
//    tracked per button per frame, just like DirectInput's buffered events.

#include "directinput.h"
#include <SDL3/SDL.h>
#include <cstring>
#include <cstdio>
#include <cmath>

// ============================================================================
// Static state (matches the private members of the original DirectInput class)
// ============================================================================

char    DirectInput::DIKeyboardButtons[DirectInput::NUM_KEYBOARD_BUTTONS];
char    DirectInput::DIMouseButtons[DirectInput::NUM_MOUSE_BUTTONS];
int     DirectInput::DIMouseAxis[DirectInput::NUM_MOUSE_AXIS];
char    DirectInput::DIJoystickButtons[DirectInput::NUM_MOUSE_BUTTONS];
float   DirectInput::ButtonLastHitTime[DirectInput::NUM_KEYBOARD_BUTTONS];

Vector3 DirectInput::CursorPos;
bool    DirectInput::EatMouseHeld = false;

void*   DirectInput::DirectInputLibrary = nullptr;
int     DirectInput::LastKeyPressed = 0;
bool    DirectInput::Captured = false;

// SDL3 handles (lazy-initialized in Init)
static SDL_Window*   s_sdl_window = nullptr;
static SDL_Gamepad*  s_sdl_gamepad = nullptr;
static bool          s_has_focus = true;
static bool          s_mouse_grabbed = false;
static bool          s_mouse_relative = false;
static Uint32        s_last_event_poll = 0;

// Last known state of keyboard (for HIT/RELEASED transitions)
static unsigned char s_keyboard_state[DirectInput::NUM_KEYBOARD_BUTTONS] = {0};

// ============================================================================
// SDL scancode -> DIK_* conversion
// ============================================================================

static int SDLKey_To_DIK(SDL_Keycode key)
{
	switch (key) {
	// Letters
	case SDLK_A: return DIK_A; case SDLK_B: return DIK_B; case SDLK_C: return DIK_C;
	case SDLK_D: return DIK_D; case SDLK_E: return DIK_E; case SDLK_F: return DIK_F;
	case SDLK_G: return DIK_G; case SDLK_H: return DIK_H; case SDLK_I: return DIK_I;
	case SDLK_J: return DIK_J; case SDLK_K: return DIK_K; case SDLK_L: return DIK_L;
	case SDLK_M: return DIK_M; case SDLK_N: return DIK_N; case SDLK_O: return DIK_O;
	case SDLK_P: return DIK_P; case SDLK_Q: return DIK_Q; case SDLK_R: return DIK_R;
	case SDLK_S: return DIK_S; case SDLK_T: return DIK_T; case SDLK_U: return DIK_U;
	case SDLK_V: return DIK_V; case SDLK_W: return DIK_W; case SDLK_X: return DIK_X;
	case SDLK_Y: return DIK_Y; case SDLK_Z: return DIK_Z;

	// Numbers
	case SDLK_0: return DIK_0; case SDLK_1: return DIK_1; case SDLK_2: return DIK_2;
	case SDLK_3: return DIK_3; case SDLK_4: return DIK_4; case SDLK_5: return DIK_5;
	case SDLK_6: return DIK_6; case SDLK_7: return DIK_7; case SDLK_8: return DIK_8;
	case SDLK_9: return DIK_9;

	// Common keys
	case SDLK_ESCAPE:      return DIK_ESCAPE;
	case SDLK_BACKSPACE:   return DIK_BACK;
	case SDLK_TAB:         return DIK_TAB;
	case SDLK_RETURN:      return DIK_RETURN;
	case SDLK_SPACE:       return DIK_SPACE;
	case SDLK_MINUS:       return DIK_MINUS;
	case SDLK_EQUALS:      return DIK_EQUALS;
	case SDLK_LEFTBRACKET: return DIK_LBRACKET;
	case SDLK_RIGHTBRACKET:return DIK_RBRACKET;
	case SDLK_BACKSLASH:   return DIK_BACKSLASH;
	case SDLK_SEMICOLON:   return DIK_SEMICOLON;
	case SDLK_APOSTROPHE:  return DIK_APOSTROPHE;
	case SDLK_GRAVE:       return DIK_GRAVE;
	case SDLK_COMMA:       return DIK_COMMA;
	case SDLK_PERIOD:      return DIK_PERIOD;
	case SDLK_SLASH:       return DIK_SLASH;
	case SDLK_CAPSLOCK:    return DIK_CAPITAL;

	// Modifiers
	case SDLK_LSHIFT:      return DIK_LSHIFT;
	case SDLK_RSHIFT:      return DIK_RSHIFT;
	case SDLK_LCTRL:       return DIK_LCONTROL;
	case SDLK_RCTRL:       return DIK_RCONTROL;
	case SDLK_LALT:        return DIK_LMENU;
	case SDLK_RALT:        return DIK_RMENU;
	case SDLK_LGUI:        return DIK_LWIN;
	case SDLK_RGUI:        return DIK_RWIN;
	case SDLK_APPLICATION: return DIK_APPS;

	// Function keys
	case SDLK_F1:  return DIK_F1;  case SDLK_F2:  return DIK_F2;
	case SDLK_F3:  return DIK_F3;  case SDLK_F4:  return DIK_F4;
	case SDLK_F5:  return DIK_F5;  case SDLK_F6:  return DIK_F6;
	case SDLK_F7:  return DIK_F7;  case SDLK_F8:  return DIK_F8;
	case SDLK_F9:  return DIK_F9;  case SDLK_F10: return DIK_F10;
	case SDLK_F11: return DIK_F11; case SDLK_F12: return DIK_F12;

	// Navigation
	case SDLK_INSERT:   return DIK_INSERT;
	case SDLK_DELETE:   return DIK_DELETE;
	case SDLK_HOME:     return DIK_HOME;
	case SDLK_END:      return DIK_END;
	case SDLK_PAGEUP:   return DIK_PRIOR;
	case SDLK_PAGEDOWN: return DIK_NEXT;
	case SDLK_UP:       return DIK_UP;
	case SDLK_DOWN:     return DIK_DOWN;
	case SDLK_LEFT:     return DIK_LEFT;
	case SDLK_RIGHT:    return DIK_RIGHT;

	// Numpad
	case SDLK_KP_0: return DIK_NUMPAD0; case SDLK_KP_1: return DIK_NUMPAD1;
	case SDLK_KP_2: return DIK_NUMPAD2; case SDLK_KP_3: return DIK_NUMPAD3;
	case SDLK_KP_4: return DIK_NUMPAD4; case SDLK_KP_5: return DIK_NUMPAD5;
	case SDLK_KP_6: return DIK_NUMPAD6; case SDLK_KP_7: return DIK_NUMPAD7;
	case SDLK_KP_8: return DIK_NUMPAD8; case SDLK_KP_9: return DIK_NUMPAD9;
	case SDLK_KP_ENTER:  return DIK_NUMPADENTER;
	case SDLK_KP_PLUS:   return DIK_ADD;
	case SDLK_KP_MINUS:  return DIK_SUBTRACT;
	case SDLK_KP_MULTIPLY: return DIK_MULTIPLY;
	case SDLK_KP_DIVIDE:   return DIK_DIVIDE;
	case SDLK_KP_PERIOD:   return DIK_DECIMAL;

	// Locks
	case SDLK_NUMLOCK:   return DIK_NUMLOCK;
	case SDLK_SCROLLLOCK:return DIK_SCROLL;
	case SDLK_PRINTSCREEN:return DIK_SYSRQ;
	}
	return -1;
}

// Map SDL3 scancode directly to a positional keyboard state index (1:1 for
// the lower byte of the SDL scancode on US layouts, which matches DIK_*).
static int SDLScancode_To_DIK(SDL_Scancode scancode)
{
	// SDL_Scancode values match the HID/USB usage codes which for the main
	// keyboard block happen to align with DIK_* in the 0x04-0x38 range.
	// DIK_ESCAPE=0x01 vs SDL_SCANCODE_ESCAPE=0x29 — they don't align, so we
	// use SDLKey_To_DIK for the actual mapping via key events. This function
	// is kept for symmetry but unused in the current implementation.
	(void)scancode;
	return -1;
}

// ============================================================================
// Init / Shutdown
// ============================================================================

void DirectInput::Init(void)
{
	// Allocate the DirectInputLibrary placeholder so the rest of the engine
	// sees a non-null handle (it never dereferences it on this path).
	DirectInputLibrary = reinterpret_cast<void*>(0x1);

	// Clear state
	std::memset(DIKeyboardButtons, 0, sizeof(DIKeyboardButtons));
	std::memset(DIMouseButtons,    0, sizeof(DIMouseButtons));
	std::memset(DIJoystickButtons, 0, sizeof(DIJoystickButtons));
	std::memset(ButtonLastHitTime, 0, sizeof(ButtonLastHitTime));
	std::memset(DIMouseAxis,       0, sizeof(DIMouseAxis));
	CursorPos = Vector3(0.0f, 0.0f, 0.0f);
	LastKeyPressed = 0;
	EatMouseHeld = false;
	Captured = true;

	// Try to open a gamepad if one is connected
	if (SDL_NumGamepads() > 0) {
		s_sdl_gamepad = SDL_OpenGamepad(0);
	}

	// Try to find the main window for mouse capture (set by the game via
	// DirectInput::SetCaptureWindow or just left as null = desktop coords).
	// Mouse will be grabbed relative-to-window on Read() if the window is set.
}

void DirectInput::Shutdown(void)
{
	if (s_sdl_gamepad) {
		SDL_CloseGamepad(s_sdl_gamepad);
		s_sdl_gamepad = nullptr;
	}
	DirectInputLibrary = nullptr;
}

void DirectInput::Acquire(void)
{
	Captured = true;
	if (s_sdl_window && !s_mouse_grabbed) {
		SDL_SetWindowMouseGrab(s_sdl_window, true);
		SDL_HideCursor();
		s_mouse_grabbed = true;
	}
}

void DirectInput::Unacquire(void)
{
	Captured = false;
	if (s_sdl_window && s_mouse_grabbed) {
		SDL_SetWindowMouseGrab(s_sdl_window, false);
		SDL_ShowCursor();
		s_mouse_grabbed = false;
	}
}

void DirectInput::Flush(void)
{
	// Clear HIT/RELEASED bits, keep HELD
	for (int i = 0; i < NUM_KEYBOARD_BUTTONS; ++i) {
		DIKeyboardButtons[i] &= DirectInput::DI_BUTTON_HELD;
	}
	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
		DIMouseButtons[i] &= DirectInput::DI_BUTTON_HELD;
	}
	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
		DIJoystickButtons[i] &= DirectInput::DI_BUTTON_HELD;
	}
	// Drain any pending SDL events
	SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
}

// ============================================================================
// Event pump — call once per frame from the main game loop. SDL3 events are
// transient, so we convert them into the held/hit/released bitmask that the
// rest of the engine expects.
// ============================================================================

void DirectInput::Read(void)
{
	if (!Captured && s_sdl_window) {
		// Don't pump when window doesn't have focus; SDL_PollEvent still
		// drains but we ignore input updates.
	}

	// 1) Update mouse axis accumulation (use SDL_GetRelativeMouseMotion for
	//    deltas — same semantic as DirectInput's DIMOFS_X/Y/Z events).
	float mx = 0.0f, my = 0.0f;
	SDL_GetRelativeMouseState(&mx, &my);
	DIMouseAxis[MOUSE_X_AXIS] = static_cast<int>(mx);
	DIMouseAxis[MOUSE_Y_AXIS] = static_cast<int>(my);
	// Mouse wheel is delivered as an event; we accumulate and drain in the
	// event loop below.

	// 2) Snapshot the current HELD bits before processing events. The event
	//    loop will set HIT/RELEASED by comparing to this snapshot.
	unsigned char prev_kb[NUM_KEYBOARD_BUTTONS];
	std::memcpy(prev_kb, s_keyboard_state, sizeof(prev_kb));

	// Mouse button snapshots
	bool prev_mb[NUM_MOUSE_BUTTONS] = {
		(s_keyboard_state[0] & DI_BUTTON_HELD) != 0,  // unused slot
		false, false
	};
	// We track mouse buttons separately from the keyboard array; rebuild
	// from sdl state below.
	(void)prev_mb;

	// 3) Process SDL events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP: {
			int dik = SDLKey_To_DIK(event.key.key);
			if (dik < 0 || dik >= NUM_KEYBOARD_BUTTONS) break;
			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (!(s_keyboard_state[dik] & DI_BUTTON_HELD)) {
					// First press this frame -> HIT
					s_keyboard_state[dik] = (char)(DI_BUTTON_HELD | DI_BUTTON_HIT);
				} else {
					// Already held (key repeat) -> just HELD
					s_keyboard_state[dik] = (char)DI_BUTTON_HELD;
				}
				LastKeyPressed = dik;
			} else {
				s_keyboard_state[dik] = (char)DI_BUTTON_RELEASED;
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			int btn = -1;
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:   btn = BUTTON_MOUSE_LEFT;   break;
			case SDL_BUTTON_RIGHT:  btn = BUTTON_MOUSE_RIGHT;  break;
			case SDL_BUTTON_MIDDLE: btn = BUTTON_MOUSE_CENTER; break;
			}
			if (btn < 0) break;
			int idx = btn - BUTTON_MOUSE_FIRST;
			if (idx < 0 || idx >= NUM_MOUSE_BUTTONS) break;
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				DIMouseButtons[idx] = (char)(DI_BUTTON_HELD | DI_BUTTON_HIT);
			} else {
				DIMouseButtons[idx] = (char)DI_BUTTON_RELEASED;
			}
			break;
		}

		case SDL_EVENT_MOUSE_WHEEL: {
			// SDL3 wheel.y is positive = away from user
			DIMouseAxis[MOUSE_Z_AXIS] += event.wheel.y;
			break;
		}

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			// Map gamepad A -> BUTTON_JOYSTICK_A, B -> BUTTON_JOYSTICK_B
			int btn = -1;
			if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) btn = 0; // A
			else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) btn = 1; // B
			if (btn < 0 || btn >= NUM_MOUSE_BUTTONS) break;
			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				DIJoystickButtons[btn] = (char)(DI_BUTTON_HELD | DI_BUTTON_HIT);
			} else {
				DIJoystickButtons[btn] = (char)DI_BUTTON_RELEASED;
			}
			break;
		}

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			s_has_focus = true;
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			s_has_focus = false;
			// Release all keys/buttons on focus loss so they don't stick
			for (int i = 0; i < NUM_KEYBOARD_BUTTONS; ++i) {
				if (s_keyboard_state[i] & DI_BUTTON_HELD) {
					s_keyboard_state[i] = (char)DI_BUTTON_RELEASED;
				}
			}
			for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
				if (DIMouseButtons[i] & DI_BUTTON_HELD) {
					DIMouseButtons[i] = (char)DI_BUTTON_RELEASED;
				}
			}
			break;

		case SDL_EVENT_QUIT:
			// App quit — propagate to engine by setting the application
			// keyboard "escape" key as a HIT
			s_keyboard_state[DIK_ESCAPE] = (char)(DI_BUTTON_HELD | DI_BUTTON_HIT);
			break;

		default:
			break;
		}
	}

	// 4) Now compute HIT/RELEASED transitions for keyboard by comparing the
	//    event-driven s_keyboard_state to the prev_kb snapshot taken before
	//    the event loop. For any key currently HELD that wasn't in prev_kb,
	//    mark HIT. For any key in prev_kb that isn't HELD now, mark RELEASED.
	//    (The event loop already set HIT/RELEASED on key events; this is
	//    belt-and-suspenders to catch the no-event "all keys up" case.)
	for (int i = 0; i < NUM_KEYBOARD_BUTTONS; ++i) {
		bool prev_held = (prev_kb[i] & DI_BUTTON_HELD) != 0;
		bool cur_held  = (s_keyboard_state[i] & DI_BUTTON_HELD) != 0;
		if (cur_held && !prev_held) {
			s_keyboard_state[i] = (char)(DI_BUTTON_HELD | DI_BUTTON_HIT);
		} else if (!cur_held && prev_held) {
			s_keyboard_state[i] = (char)DI_BUTTON_RELEASED;
		}
	}
	std::memcpy(DIKeyboardButtons, s_keyboard_state, sizeof(DIKeyboardButtons));

	// 5) Joystick axes
	if (s_sdl_gamepad) {
		// SDL3 gamepad axes are -32768..32767; DirectInput scaled to
		// roughly -1000..1000. The engine divides by 1000.0f, so we scale.
		float lx = SDL_GetGamepadAxis(s_sdl_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		float ly = SDL_GetGamepadAxis(s_sdl_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
		// Apply deadzone (5%)
		const float deadzone = 0.05f;
		if (std::fabs(lx) < deadzone) lx = 0.0f;
		if (std::fabs(ly) < deadzone) ly = 0.0f;
		// Store in a side channel — DirectInput::Get_Joystick_Axis_State
		// reads the actual DirectInput-style axis. We stash via the
		// Get_Joystick_Axis_State method below.
		// (Implementation is overridden at the bottom of this file.)
		// For now, just ensure the buttons are populated; axes go through
		// the helper.
	}

	// 6) Eat-mouse-held: if requested, convert HIT -> HELD so the next frame
	//    doesn't see the press (used to suppress double-click on focus loss)
	if (EatMouseHeld) {
		for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
			DIMouseButtons[i] &= DI_BUTTON_HELD;
		}
	}
}

// ============================================================================
// Joystick axis (overrides the inline in directinput.h since the SDL3 path
// reads from SDL_GetGamepadAxis, not DirectInput's buffered events)
// ============================================================================

int DirectInput::Get_Joystick_Axis_State(JoystickAxis axis)
{
	if (!s_sdl_gamepad) return 0;
	float v = 0.0f;
	switch (axis) {
	case JOYSTICK_X_AXIS:
		v = SDL_GetGamepadAxis(s_sdl_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		break;
	case JOYSTICK_Y_AXIS:
		v = SDL_GetGamepadAxis(s_sdl_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
		break;
	}
	// SDL3 axis range: -32768..32767. DirectInput range: -1000..1000.
	return static_cast<int>(v / 32.768f);
}

void DirectInput::Eat_Mouse_Held_States(void)
{
	EatMouseHeld = true;
}
