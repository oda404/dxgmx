/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_DRIVERS_INPUT_INPUT_USERKBD_H
#define _DXGMX_DRIVERS_INPUT_INPUT_USERKBD_H

#define KBD_KEY_PRESS 1
#define KBD_KEY_RELEASE 2
#define KBD_KEY_REPEAT 3

/* Based on my 75% keyboard, + ps2 scancode 2 there are keys missing probably */
#define KEY_ESC 0x1
#define KEY_F1 0x2
#define KEY_F2 0x3
#define KEY_F3 0x4
#define KEY_F4 0x5
#define KEY_F5 0x6
#define KEY_F6 0x7
#define KEY_F7 0x8
#define KEY_F8 0x9
#define KEY_F9 0xA
#define KEY_F10 0xB
#define KEY_F11 0xC
#define KEY_F12 0xD
#define KEY_SS 0xE
#define KEY_DEL 0xF
#define KEY_BACKTICK 0x10
#define KEY_1 0x11
#define KEY_2 0x12
#define KEY_3 0x13
#define KEY_4 0x14
#define KEY_5 0x15
#define KEY_6 0x16
#define KEY_7 0x17
#define KEY_8 0x18
#define KEY_9 0x19
#define KEY_0 0x1A
#define KEY_MINUS 0x1B
#define KEY_EQUAL 0x1C
#define KEY_BACKSPACE 0x1D
#define KEY_PAGEUP 0x1E
#define KEY_TAB 0x1F
#define KEY_Q 0x20
#define KEY_W 0x21
#define KEY_E 0x22
#define KEY_R 0x23
#define KEY_T 0x24
#define KEY_Y 0x25
#define KEY_U 0x26
#define KEY_I 0x27
#define KEY_O 0x28
#define KEY_P 0x29
#define KEY_OPEN_SQ_BRACKET 0x2A
#define KEY_CLOSE_SQ_BRACKET 0x2B
#define KEY_BACKSLASH 0x2C
#define KEY_PAGEDOWN 0x2D
#define KEY_CAPSLOCK 0x2E
#define KEY_A 0x2F
#define KEY_S 0x30
#define KEY_D 0x31
#define KEY_F 0x32
#define KEY_G 0x33
#define KEY_H 0x34
#define KEY_J 0x35
#define KEY_K 0x36
#define KEY_L 0x37
#define KEY_SEMI_COLON 0x38
#define KEY_SINGLE_QUOTE 0x39
#define KEY_RETURN 0x3A
#define KEY_HOME 0x3B
#define KEY_LEFT_SHIFT 0x3C
#define KEY_Z 0x3D
#define KEY_X 0x3E
#define KEY_C 0x3F
#define KEY_V 0x40
#define KEY_B 0x41
#define KEY_N 0x42
#define KEY_M 0x43
#define KEY_COMMA 0x44
#define KEY_DOT 0x45
#define KEY_SLASH 0x46
#define KEY_RIGHT_SHIFT 0x47
#define KEY_UP_ARROW 0x48
#define KEY_END 0x49
#define KEY_LEFT_CTRL 0x4A
#define KEY_LEFT_COMMAND 0x4B
#define KEY_LEFT_ALT 0x4C
#define KEY_SPACE 0x4D
#define KEY_RIGHT_ALT 0x4E
#define KEY_RIGHT_CTRL 0x4F
#define KEY_LEFT_ARROW 0x50
#define KEY_DOWN_ARROW 0x51
#define KEY_RIGHT_ARROW 0x52
#define KEY_RIGHT_COMMAND 0x53
#define KEY_SCROLL_LOCK 0x54
#define KEY_NUM_LOCK 0x55
#define KEY_NUM_ASTERISK 0x56
#define KEY_NUM_MINUS 0x57
#define KEY_NUM_7 0x58
#define KEY_NUM_8 0x59
#define KEY_NUM_9 0x5A
#define KEY_NUM_PLUS 0x5B
#define KEY_NUM_4 0x5C
#define KEY_NUM_5 0x5D
#define KEY_NUM_6 0x5E
#define KEY_NUM_1 0x5F
#define KEY_NUM_2 0x60
#define KEY_NUM_3 0x61
#define KEY_NUM_0 0x62
#define KEY_NUM_DOT 0x63
#define KEY_NUM_SLASH 0x64
#define KEY_NUM_RETURN 0x65
#define KEY_BREAK 0x66

#endif // !_DXGMX_DRIVERS_INPUT_INPUT_USERKBD_H
