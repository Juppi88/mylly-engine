#pragma once
#ifndef __KEYS_H
#define __KEYS_H

#ifdef _WIN32

#include <Windows.h>

#define MKEY_LMB			1
#define MKEY_MMB			2
#define MKEY_RMB			3

#define MKEY_CANCEL			VK_CANCEL
#define MKEY_BACKSPACE		VK_BACK
#define MKEY_TAB			VK_TAB
#define MKEY_CLEAR			VK_CLEAR
#define MKEY_RETURN			VK_RETURN

#define MKEY_SHIFT			VK_SHIFT
#define MKEY_CONTROL		VK_CONTROL
#define MKEY_ALT			VK_MENU
#define MKEY_PAUSE			VK_PAUSE
#define MKEY_CAPS			VK_CAPITAL

#define MKEY_ESCAPE			VK_ESCAPE

#define MKEY_SPACE			VK_SPACE
#define MKEY_PAGEDOWN		VK_PRIOR
#define MKEY_PAGEUP			VK_NEXT
#define MKEY_END			VK_END
#define MKEY_HOME			VK_HOME
#define MKEY_LEFT			VK_LEFT
#define MKEY_UP				VK_UP
#define MKEY_RIGHT			VK_RIGHT
#define MKEY_DOWN			VK_DOWN
#define MKEY_PRINTSCR		VK_SNAPSHOT
#define MKEY_INSERT			VK_INSERT
#define MKEY_DELETE			VK_DELETE

// ASCII 0-9 and A-Z are the same as their ASCII codes.

#define MKEY_NUMPAD0		VK_NUMPAD0
#define MKEY_NUMPAD1		VK_NUMPAD1
#define MKEY_NUMPAD2		VK_NUMPAD2
#define MKEY_NUMPAD3		VK_NUMPAD3
#define MKEY_NUMPAD4		VK_NUMPAD4
#define MKEY_NUMPAD5		VK_NUMPAD5
#define MKEY_NUMPAD6		VK_NUMPAD6
#define MKEY_NUMPAD7		VK_NUMPAD7
#define MKEY_NUMPAD8		VK_NUMPAD8
#define MKEY_NUMPAD9		VK_NUMPAD9
#define MKEY_MULTIPLY		VK_MULTIPLY
#define MKEY_ADD			VK_ADD
#define MKEY_SEPARATOR		VK_SEPARATOR
#define MKEY_SUBTRACT		VK_SUBTRACT
#define MKEY_DECIMAL		VK_DECIMAL
#define MKEY_DIVIDE			VK_DIVIDE
#define MKEY_F1				VK_F1
#define MKEY_F2				VK_F2
#define MKEY_F3				VK_F3
#define MKEY_F4				VK_F4
#define MKEY_F5				VK_F5
#define MKEY_F6				VK_F6
#define MKEY_F7				VK_F7
#define MKEY_F8				VK_F8
#define MKEY_F9				VK_F9
#define MKEY_F10			VK_F10
#define MKEY_F11			VK_F11
#define MKEY_F12			VK_F12
#define MKEY_NUMLOCK		VK_NUMLOCK
#define MKEY_SCROLL			VK_SCROLL

#define MKEY_LSHIFT			VK_LSHIFT
#define MKEY_RSHIFT			VK_RSHIFT
#define MKEY_LCONTROL		VK_LCONTROL
#define MKEY_RCONTROL		VK_RCONTROL
#define MKEY_LALT			VK_LMENU
#define MKEY_RALT			VK_RMENU

#else

#define XK_MISCELLANY
#include <X11/keysym.h>

#define MKEY_LMB			1
#define MKEY_MMB			2
#define MKEY_RMB			3

#define MKEY_CANCEL			XK_Cancel
#define MKEY_BACKSPACE		XK_BackSpace
#define MKEY_TAB			XK_Tab
#define MKEY_CLEAR			XK_Clear
#define MKEY_RETURN			XK_Return

// Notice that the following symbols only detect the left modifier key. Currently there is no way to
// detect all of them without using multiple keybinds (check key symbols below).
#define MKEY_SHIFT			XK_Shift_L
#define MKEY_CONTROL		XK_Control_L
#define MKEY_ALT			XK_Alt_L
#define MKEY_PAUSE			XK_Pause
#define MKEY_CAPS			XK_Caps_Lock

#define MKEY_LSHIFT			XK_Shift_L
#define MKEY_RSHIFT			XK_Shift_R
#define MKEY_LCONTROL		XK_Control_L
#define MKEY_RCONTROL		XK_Control_R
#define MKEY_LALT			XK_Alt_L
#define MKEY_RALT			XK_Alt_R

#define MKEY_ESCAPE			XK_Escape

#define MKEY_SPACE			XK_space
#define MKEY_PAGEDOWN		XK_Page_Down
#define MKEY_PAGEUP			XK_Page_Up
#define MKEY_END			XK_End
#define MKEY_HOME			XK_Home
#define MKEY_LEFT			XK_Left
#define MKEY_UP				XK_Up
#define MKEY_RIGHT			XK_Right
#define MKEY_DOWN			XK_Down
#define MKEY_PRINTSCR		XK_Print
#define MKEY_INSERT			XK_Insert
#define MKEY_DELETE			XK_Delete

// ASCII 0-9 and A-Z are the same as their ASCII codes.

#define MKEY_NUMPAD0		XK_KP_0
#define MKEY_NUMPAD1		XK_KP_1
#define MKEY_NUMPAD2		XK_KP_2
#define MKEY_NUMPAD3		XK_KP_3
#define MKEY_NUMPAD4		XK_KP_4
#define MKEY_NUMPAD5		XK_KP_5
#define MKEY_NUMPAD6		XK_KP_6
#define MKEY_NUMPAD7		XK_KP_7
#define MKEY_NUMPAD8		XK_KP_8
#define MKEY_NUMPAD9		XK_KP_9
#define MKEY_MULTIPLY		XK_KP_Multiply
#define MKEY_ADD			XK_KP_Add
#define MKEY_SEPARATOR		XK_KP_Separator
#define MKEY_SUBTRACT		XK_KP_Subtract
#define MKEY_DECIMAL		XK_KP_Decimal
#define MKEY_DIVIDE			XK_KP_Divide
#define MKEY_F1				XK_F1
#define MKEY_F2				XK_F2
#define MKEY_F3				XK_F3
#define MKEY_F4				XK_F4
#define MKEY_F5				XK_F5
#define MKEY_F6				XK_F6
#define MKEY_F7				XK_F7
#define MKEY_F8				XK_F8
#define MKEY_F9				XK_F9
#define MKEY_F10			XK_F10
#define MKEY_F11			XK_F11
#define MKEY_F12			XK_F12
#define MKEY_SCROLL			XK_Scroll_Lock

#define MKEY_NUMLOCK		XK_VoidSymbol

#endif /* _WIN32 */

#endif /* __KEYS */
