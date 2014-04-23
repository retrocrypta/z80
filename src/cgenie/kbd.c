/* ed:set tabstop=8 noexpandtab: */
/***************************************************************************************
 *
 * kbd.c	Colour Genie EG2000 keyboard emulation
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 ***************************************************************************************/
#include "cgenie/kbd.h"

/** @brief keyboard matrix */
static uint8_t keymap[8];

/** @brief dump keycodes of pressed/released keys */
#define	DUMP_KEYCODE	0

#define	PRINTABLE(c)	((c) < 32 ? '.' : (c))

/***************************************************************************
 *   w/o SHIFT                             with SHIFT
 *   +-------------------------------+     +-------------------------------+
 *   | 0   1   2   3   4   5   6   7 |     | 0   1   2   3   4   5   6   7 |
 *+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
 *|0 | @ | A | B | C | D | E | F | G |  |0 | ` | a | b | c | d | e | f | g |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|1 | H | I | J | K | L | M | N | O |  |1 | h | i | j | k | l | m | n | o |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|2 | P | Q | R | S | T | U | V | W |  |2 | p | q | r | s | t | u | v | w |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|3 | X | Y | Z | [ | \ | ] | ^ | _ |  |3 | x | y | z | { | | | } | ~ |   |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  |4 |   | ! | " | # | $ | % | & | ' |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|5 | 8 | 9 | : | ; | , | - | . | / |  |5 | ( | ) | * | + | < | = | > | ? |
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|  |6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|
 *|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
 *|7 |SHF|GFX|PUP|PDN|INS|DEL|CTL|END|  |7 |SHF|GFX|PUP|PDN|INS|DEL|CTL|END|
 *+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
 ***************************************************************************/

typedef enum {
K_NONE=-1,
K_AT,		K_A,		K_B,		K_C,		K_D,		K_E,		K_F,		K_G,
K_H,		K_I,		K_J,		K_K,		K_L,		K_M,		K_N,		K_O,
K_P,		K_Q,		K_R,		K_S,		K_T,		K_U,		K_V,		K_W,
K_X,		K_Y,		K_Z,		K_LEFTBRACKET,	K_BACKSLASH,	K_RIGHTBRACKET,	K_CARET,	K_UNDERSCORE,
K_0,		K_1,		K_2,		K_3,		K_4,		K_5,		K_6,		K_7,
K_8,		K_9,		K_COLON,	K_SEMICOLON,	K_COMMA,	K_MINUS,	K_PERIOD,	K_SLASH,
K_ENTER,	K_CLEAR,	K_BREAK,	K_UP,		K_DOWN,		K_LEFT,		K_RIGHT,	K_SPACE,
K_SHIFT,	K_GRAPHICS,	K_PAGEUP,	K_PAGEDOWN,	K_INSERT,	K_DELETE,	K_CTRL,		K_END,
K_SHIFTED,
K_NMI=128,	K_RESET
}	cgenie_keycode_t;

typedef struct {
	uint32_t sym;
	cgenie_keycode_t key;
}	cgenie_keymap_t;

cgenie_keymap_t map[] = {
	{SDLK_BACKSPACE,	K_LEFT},
	{SDLK_TAB,		K_RIGHT},
	{SDLK_CLEAR,		K_CLEAR},
	{SDLK_RETURN,		K_ENTER},
	{SDLK_PAUSE,		K_NONE},
	{SDLK_ESCAPE,		K_BREAK},
	{SDLK_SPACE,		K_SPACE},
	{SDLK_EXCLAIM,		K_1 | K_SHIFTED},
	{SDLK_QUOTEDBL,		K_2 | K_SHIFTED},
	{SDLK_HASH,		K_3 | K_SHIFTED},
	{SDLK_DOLLAR,		K_4 | K_SHIFTED},
	{SDLK_AMPERSAND,	K_5 | K_SHIFTED},
	{SDLK_QUOTE,		K_7 | K_SHIFTED},
	{SDLK_LEFTPAREN,	K_8 | K_SHIFTED},
	{SDLK_RIGHTPAREN,	K_9 | K_SHIFTED},
	{SDLK_ASTERISK,		K_COLON | K_SHIFTED},
	{SDLK_PLUS,		K_SEMICOLON | K_SHIFTED},
	{SDLK_COMMA,		K_COMMA},
	{SDLK_MINUS,		K_MINUS},
	{SDLK_PERIOD,		K_PERIOD},
	{SDLK_SLASH,		K_SLASH},
	{SDLK_0,		K_0},
	{SDLK_1,		K_1},
	{SDLK_2,		K_2},
	{SDLK_3,		K_3},
	{SDLK_4,		K_4},
	{SDLK_5,		K_5},
	{SDLK_6,		K_6},
	{SDLK_7,		K_7},
	{SDLK_8,		K_8},
	{SDLK_9,		K_9},
	{SDLK_COLON,		K_COLON},
	{SDLK_SEMICOLON,	K_SEMICOLON},
	{SDLK_LESS,		K_COMMA | K_SHIFTED},
	{SDLK_EQUALS,		K_MINUS | K_SHIFTED},
	{SDLK_GREATER,		K_PERIOD | K_SHIFTED},
	{SDLK_QUESTION,		K_SLASH | K_SHIFTED},
	{SDLK_AT,		K_AT},
	/*
	   Skip uppercase letters
	 */
	{SDLK_LEFTBRACKET,	K_LEFTBRACKET},
	{SDLK_BACKSLASH,	K_BACKSLASH},
	{SDLK_RIGHTBRACKET,	K_RIGHTBRACKET},
	{SDLK_CARET,		K_CARET},
	{SDLK_UNDERSCORE,	K_UNDERSCORE},
	{SDLK_BACKQUOTE,	K_NONE},
	{SDLK_a,		K_A},
	{SDLK_b,		K_B},
	{SDLK_c,		K_C},
	{SDLK_d,		K_D},
	{SDLK_e,		K_E},
	{SDLK_f,		K_F},
	{SDLK_g,		K_G},
	{SDLK_h,		K_H},
	{SDLK_i,		K_I},
	{SDLK_j,		K_J},
	{SDLK_k,		K_K},
	{SDLK_l,		K_L},
	{SDLK_m,		K_M},
	{SDLK_n,		K_N},
	{SDLK_o,		K_O},
	{SDLK_p,		K_P},
	{SDLK_q,		K_Q},
	{SDLK_r,		K_R},
	{SDLK_s,		K_S},
	{SDLK_t,		K_T},
	{SDLK_u,		K_U},
	{SDLK_v,		K_V},
	{SDLK_w,		K_W},
	{SDLK_x,		K_X},
	{SDLK_y,		K_Y},
	{SDLK_z,		K_Z},
	{SDLK_DELETE,		K_LEFT},
	/* End of ASCII mapped keysyms */

	/* International keyboard syms */
	{SDLK_WORLD_0,		K_NONE},
	{SDLK_WORLD_1,		K_NONE},
	{SDLK_WORLD_2,		K_NONE},
	{SDLK_WORLD_3,		K_NONE},
	{SDLK_WORLD_4,		K_NONE},
	{SDLK_WORLD_5,		K_NONE},
	{SDLK_WORLD_6,		K_NONE},
	{SDLK_WORLD_7,		K_NONE},
	{SDLK_WORLD_8,		K_NONE},
	{SDLK_WORLD_9,		K_NONE},
	{SDLK_WORLD_10,		K_NONE},
	{SDLK_WORLD_11,		K_NONE},
	{SDLK_WORLD_12,		K_NONE},
	{SDLK_WORLD_13,		K_NONE},
	{SDLK_WORLD_14,		K_NONE},
	{SDLK_WORLD_15,		K_NONE},
	{SDLK_WORLD_16,		K_NONE},
	{SDLK_WORLD_17,		K_NONE},
	{SDLK_WORLD_18,		K_NONE},
	{SDLK_WORLD_19,		K_NONE},
	{SDLK_WORLD_20,		K_NONE},
	{SDLK_WORLD_21,		K_NONE},
	{SDLK_WORLD_22,		K_NONE},
	{SDLK_WORLD_23,		K_NONE},
	{SDLK_WORLD_24,		K_NONE},
	{SDLK_WORLD_25,		K_NONE},
	{SDLK_WORLD_26,		K_NONE},
	{SDLK_WORLD_27,		K_NONE},
	{SDLK_WORLD_28,		K_NONE},
	{SDLK_WORLD_29,		K_NONE},
	{SDLK_WORLD_30,		K_NONE},
	{SDLK_WORLD_31,		K_NONE},
	{SDLK_WORLD_32,		K_NONE},
	{SDLK_WORLD_33,		K_NONE},
	{SDLK_WORLD_34,		K_NONE},
	{SDLK_WORLD_35,		K_NONE},
	{SDLK_WORLD_36,		K_NONE},
	{SDLK_WORLD_37,		K_NONE},
	{SDLK_WORLD_38,		K_NONE},
	{SDLK_WORLD_39,		K_NONE},
	{SDLK_WORLD_40,		K_NONE},
	{SDLK_WORLD_41,		K_NONE},
	{SDLK_WORLD_42,		K_NONE},
	{SDLK_WORLD_43,		K_NONE},
	{SDLK_WORLD_44,		K_NONE},
	{SDLK_WORLD_45,		K_NONE},
	{SDLK_WORLD_46,		K_NONE},
	{SDLK_WORLD_47,		K_NONE},
	{SDLK_WORLD_48,		K_NONE},
	{SDLK_WORLD_49,		K_NONE},
	{SDLK_WORLD_50,		K_NONE},
	{SDLK_WORLD_51,		K_NONE},
	{SDLK_WORLD_52,		K_NONE},
	{SDLK_WORLD_53,		K_NONE},
	{SDLK_WORLD_54,		K_NONE},
	{SDLK_WORLD_55,		K_NONE},
	{SDLK_WORLD_56,		K_NONE},
	{SDLK_WORLD_57,		K_NONE},
	{SDLK_WORLD_58,		K_NONE},
	{SDLK_WORLD_59,		K_NONE},
	{SDLK_WORLD_60,		K_NONE},
	{SDLK_WORLD_61,		K_NONE},
	{SDLK_WORLD_62,		K_NONE},
	{SDLK_WORLD_63,		K_CARET},		/* szlig */
	{SDLK_WORLD_64,		K_NONE},
	{SDLK_WORLD_65,		K_NONE},
	{SDLK_WORLD_66,		K_NONE},
	{SDLK_WORLD_67,		K_NONE},
	{SDLK_WORLD_68,		K_LEFTBRACKET},		/* Auml */
	{SDLK_WORLD_69,		K_NONE},
	{SDLK_WORLD_70,		K_NONE},
	{SDLK_WORLD_71,		K_NONE},
	{SDLK_WORLD_72,		K_NONE},
	{SDLK_WORLD_73,		K_NONE},
	{SDLK_WORLD_74,		K_NONE},
	{SDLK_WORLD_75,		K_NONE},
	{SDLK_WORLD_76,		K_NONE},
	{SDLK_WORLD_77,		K_NONE},
	{SDLK_WORLD_78,		K_NONE},
	{SDLK_WORLD_79,		K_NONE},
	{SDLK_WORLD_80,		K_NONE},
	{SDLK_WORLD_81,		K_NONE},
	{SDLK_WORLD_82,		K_NONE},
	{SDLK_WORLD_83,		K_NONE},
	{SDLK_WORLD_84,		K_NONE},
	{SDLK_WORLD_85,		K_NONE},
	{SDLK_WORLD_86,		K_BACKSLASH},		/* Ouml */
	{SDLK_WORLD_87,		K_NONE},
	{SDLK_WORLD_88,		K_NONE},
	{SDLK_WORLD_89,		K_NONE},
	{SDLK_WORLD_90,		K_NONE},
	{SDLK_WORLD_91,		K_NONE},
	{SDLK_WORLD_92,		K_RIGHTBRACKET},	/* Uuml */
	{SDLK_WORLD_93,		K_NONE},
	{SDLK_WORLD_94,		K_NONE},
	{SDLK_WORLD_95,		K_NONE},

	/* Numeric keypad */
	{SDLK_KP0,		K_INSERT},
	{SDLK_KP1,		K_END},
	{SDLK_KP2,		K_DOWN},
	{SDLK_KP3,		K_PAGEDOWN},
	{SDLK_KP4,		K_LEFT},
	{SDLK_KP5,		K_NONE},
	{SDLK_KP6,		K_RIGHT},
	{SDLK_KP7,		K_CLEAR},
	{SDLK_KP8,		K_UP},
	{SDLK_KP9,		K_PAGEUP},
	{SDLK_KP_PERIOD,	K_DELETE},
	{SDLK_KP_DIVIDE,	K_SLASH},
	{SDLK_KP_MULTIPLY,	K_COLON | K_SHIFTED},
	{SDLK_KP_MINUS,		K_MINUS},
	{SDLK_KP_PLUS,		K_SEMICOLON | K_SHIFTED},
	{SDLK_KP_ENTER,		K_ENTER},
	{SDLK_KP_EQUALS,	K_MINUS | K_SHIFTED},

	/* Arrows + Home/End pad */
	{SDLK_UP,		K_UP},
	{SDLK_DOWN,		K_DOWN},
	{SDLK_RIGHT,		K_RIGHT},
	{SDLK_LEFT,		K_LEFT},
	{SDLK_INSERT,		K_INSERT},
	{SDLK_HOME,		K_CLEAR},
	{SDLK_END,		K_END},
	{SDLK_PAGEUP,		K_PAGEUP},
	{SDLK_PAGEDOWN,		K_PAGEDOWN},

	/* Function keys */
	{SDLK_F1,		K_BACKSLASH},
	{SDLK_F2,		K_RIGHTBRACKET},
	{SDLK_F3,		K_CARET},
	{SDLK_F4,		K_UNDERSCORE},
	{SDLK_F5,		K_BACKSLASH | K_SHIFTED},
	{SDLK_F6,		K_RIGHTBRACKET | K_SHIFTED},
	{SDLK_F7,		K_CARET | K_SHIFTED},
	{SDLK_F8,		K_UNDERSCORE | K_SHIFTED},
	{SDLK_F9,		K_NMI},
	{SDLK_F10,		K_RESET},
	{SDLK_F11,		K_NONE},
	{SDLK_F12,		K_NONE},
	{SDLK_F13,		K_NONE},
	{SDLK_F14,		K_NONE},
	{SDLK_F15,		K_NONE},

	/* Key state modifier keys */
	{SDLK_NUMLOCK,		K_NONE},
	{SDLK_CAPSLOCK,		K_NONE},
	{SDLK_SCROLLOCK,	K_NONE},
	{SDLK_RSHIFT,		K_SHIFT},
	{SDLK_LSHIFT,		K_SHIFT},
	{SDLK_RCTRL,		K_CTRL},
	{SDLK_LCTRL,		K_CTRL},
	{SDLK_RALT,		K_GRAPHICS},
	{SDLK_LALT,		K_GRAPHICS},
	{SDLK_RMETA,		K_GRAPHICS},
	{SDLK_LMETA,		K_GRAPHICS},
	{SDLK_LSUPER,		K_NONE},
	{SDLK_RSUPER,		K_NONE},
	{SDLK_MODE,		K_NONE},
	{SDLK_COMPOSE,		K_NONE},

	/* Miscellaneous function keys */
	{SDLK_HELP,		K_NONE},
	{SDLK_PRINT,		K_NONE},
	{SDLK_SYSREQ,		K_NONE},
	{SDLK_BREAK,		K_NONE},
	{SDLK_MENU,		K_NONE},
	{SDLK_POWER,		K_NONE},
	{SDLK_EURO,		K_NONE},
	{SDLK_UNDO,		K_NONE},

	{SDLK_UNKNOWN,		K_NONE}
};

static cgenie_keymap_t down[64];
static int32_t ndown;

static void key_dn(osd_key_t *key, cgenie_keycode_t code)
{
	int32_t i;
	uint32_t k;
	for (i = 0; i < ndown; i++)
		if (key->sym == down[i].sym)
			break;
	if (i == ndown) {
		down[ndown].sym = key->sym;
		down[ndown].key = code;
		ndown++;
	}
	if (code == K_NONE)
		return;
	k = code % K_SHIFTED;
	keymap[k/8] |= (1 << (k % 8));
	if (code & K_SHIFTED)
		keymap[K_SHIFT/8] |= (1 << (K_SHIFT % 8));
	else
		keymap[K_SHIFT/8] &= ~(1 << (K_SHIFT % 8));
}

static void key_up(osd_key_t *key)
{
	int32_t i, j;
	uint32_t k;
	cgenie_keycode_t code = K_NONE;

	for (i = 0; i < ndown; i++) {
		if (key->sym != down[i].sym)
			continue;
		code = down[i].key;
		for (j = i + 1; j < ndown; j++)
			down[j - 1] = down[j];
		ndown--;
		break;
	}
	if (code == K_NONE)
		return;
	k = code % K_SHIFTED;
	keymap[k/8] &= ~(1 << (k % 8));
	if (code & K_SHIFTED)
		keymap[K_SHIFT/8] &= ~(1 << (K_SHIFT % 8));
}

void cgenie_kbd_reset(void)
{
	memset(keymap, 0, sizeof(keymap));
}

uint8_t *cgenie_kbd_map(void)
{
	return keymap;
}

void cgenie_key_dn(void *cookie, osd_key_t *key)
{
	int32_t i;

#if	DUMP_KEYCODE
	printf("keydn: %s sym:%#x uni:%#x '%c' ",
		osd_key_name(key), key->sym, key->unicode, PRINTABLE(key->unicode));
#endif
	if (key->flags & OSD_KEY_UNICODE) {
		switch (key->unicode) {
		case   8: key_dn(key, K_LEFT); break;
		case   9: key_dn(key, K_RIGHT); break;
		case  13: key_dn(key, K_ENTER); break;
		case  27: key_dn(key, K_BREAK); break;
		case  32: key_dn(key, K_SPACE); break;
		case '!': key_dn(key, K_1 | K_SHIFTED); break;
		case '"': key_dn(key, K_2 | K_SHIFTED); break;
		case '#': key_dn(key, K_3 | K_SHIFTED); break;
		case '$': key_dn(key, K_4 | K_SHIFTED); break;
		case '%': key_dn(key, K_5 | K_SHIFTED); break;
		case '&': key_dn(key, K_6 | K_SHIFTED); break;
		case  39: key_dn(key, K_7 | K_SHIFTED); break;
		case '(': key_dn(key, K_8 | K_SHIFTED); break;
		case ')': key_dn(key, K_9 | K_SHIFTED); break;
		case '*': key_dn(key, K_COLON | K_SHIFTED); break;
		case '+': key_dn(key, K_SEMICOLON | K_SHIFTED); break;
		case ',': key_dn(key, K_COMMA); break;
		case '-': key_dn(key, K_MINUS); break;
		case '.': key_dn(key, K_PERIOD); break;
		case '/': key_dn(key, K_SLASH); break;
		case '0': key_dn(key, K_0); break;
		case '1': key_dn(key, K_1); break;
		case '2': key_dn(key, K_2); break;
		case '3': key_dn(key, K_3); break;
		case '4': key_dn(key, K_4); break;
		case '5': key_dn(key, K_5); break;
		case '6': key_dn(key, K_6); break;
		case '7': key_dn(key, K_7); break;
		case '8': key_dn(key, K_8); break;
		case '9': key_dn(key, K_9); break;
		case ':': key_dn(key, K_COLON); break;
		case ';': key_dn(key, K_SEMICOLON); break;
		case '<': key_dn(key, K_COMMA | K_SHIFTED); break;
		case '=': key_dn(key, K_MINUS | K_SHIFTED); break;
		case '>': key_dn(key, K_PERIOD | K_SHIFTED); break;
		case '?': key_dn(key, K_SLASH | K_SHIFTED); break;
		case '@': key_dn(key, K_AT); break;
		case 'A': key_dn(key, K_A | K_SHIFTED); break;
		case 'B': key_dn(key, K_B | K_SHIFTED); break;
		case 'C': key_dn(key, K_C | K_SHIFTED); break;
		case 'D': key_dn(key, K_D | K_SHIFTED); break;
		case 'E': key_dn(key, K_E | K_SHIFTED); break;
		case 'F': key_dn(key, K_F | K_SHIFTED); break;
		case 'G': key_dn(key, K_G | K_SHIFTED); break;
		case 'H': key_dn(key, K_H | K_SHIFTED); break;
		case 'I': key_dn(key, K_I | K_SHIFTED); break;
		case 'J': key_dn(key, K_J | K_SHIFTED); break;
		case 'K': key_dn(key, K_K | K_SHIFTED); break;
		case 'L': key_dn(key, K_L | K_SHIFTED); break;
		case 'M': key_dn(key, K_M | K_SHIFTED); break;
		case 'N': key_dn(key, K_N | K_SHIFTED); break;
		case 'O': key_dn(key, K_O | K_SHIFTED); break;
		case 'P': key_dn(key, K_P | K_SHIFTED); break;
		case 'Q': key_dn(key, K_Q | K_SHIFTED); break;
		case 'R': key_dn(key, K_R | K_SHIFTED); break;
		case 'S': key_dn(key, K_S | K_SHIFTED); break;
		case 'T': key_dn(key, K_T | K_SHIFTED); break;
		case 'U': key_dn(key, K_U | K_SHIFTED); break;
		case 'V': key_dn(key, K_V | K_SHIFTED); break;
		case 'W': key_dn(key, K_W | K_SHIFTED); break;
		case 'X': key_dn(key, K_X | K_SHIFTED); break;
		case 'Y': key_dn(key, K_Y | K_SHIFTED); break;
		case 'Z': key_dn(key, K_Z | K_SHIFTED); break;
		case '[': key_dn(key, K_LEFTBRACKET); break;
		case '\\': key_dn(key, K_BACKSLASH); break;
		case ']': key_dn(key, K_RIGHTBRACKET); break;
		case '^': key_dn(key, K_CARET); break;
		case '_': key_dn(key, K_UNDERSCORE); break;
		case '`': key_dn(key, K_AT | K_SHIFTED); break;
		case 'a': key_dn(key, K_A); break;
		case 'b': key_dn(key, K_B); break;
		case 'c': key_dn(key, K_C); break;
		case 'd': key_dn(key, K_D); break;
		case 'e': key_dn(key, K_E); break;
		case 'f': key_dn(key, K_F); break;
		case 'g': key_dn(key, K_G); break;
		case 'h': key_dn(key, K_H); break;
		case 'i': key_dn(key, K_I); break;
		case 'j': key_dn(key, K_J); break;
		case 'k': key_dn(key, K_K); break;
		case 'l': key_dn(key, K_L); break;
		case 'm': key_dn(key, K_M); break;
		case 'n': key_dn(key, K_N); break;
		case 'o': key_dn(key, K_O); break;
		case 'p': key_dn(key, K_P); break;
		case 'q': key_dn(key, K_Q); break;
		case 'r': key_dn(key, K_R); break;
		case 's': key_dn(key, K_S); break;
		case 't': key_dn(key, K_T); break;
		case 'u': key_dn(key, K_U); break;
		case 'v': key_dn(key, K_V); break;
		case 'w': key_dn(key, K_W); break;
		case 'x': key_dn(key, K_X); break;
		case 'y': key_dn(key, K_Y); break;
		case 'z': key_dn(key, K_Z); break;
		case '{': key_dn(key, K_LEFTBRACKET | K_SHIFTED); break;
		case '|': key_dn(key, K_BACKSLASH | K_SHIFTED); break;
		case '}': key_dn(key, K_RIGHTBRACKET | K_SHIFTED); break;
		case '~': key_dn(key, K_CARET | K_SHIFTED); break;
		case 127: key_dn(key, K_UNDERSCORE | K_SHIFTED); break;
		case 223: key_dn(key, K_UNDERSCORE); break; /* szlig */
		case 228: key_dn(key, K_BACKSLASH); break; /* auml */
		case 246: key_dn(key, K_RIGHTBRACKET); break; /* ouml */
		case 252: key_dn(key, K_CARET); break; /* uuml */
		default:
			if (key->unicode)
				printf("unicode: %d (%#x) %s\n",
					key->unicode, key->unicode, osd_key_name(key));
			goto lookup;
		}
#if	DUMP_KEYCODE
		printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
			keymap[0], keymap[1], keymap[2], keymap[3],
			keymap[4], keymap[5], keymap[6], keymap[7]);
#endif
		return;
	}
lookup:
	/* find non-unicode keyboard symbols in the lookup table */
	for (i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
		if (key->sym == map[i].sym) {
			switch (map[i].key) {
			case K_NMI:
				sys_reset(SYS_NMI);
				break;
			case K_RESET:
				sys_reset(SYS_RST);
				break;
			default:
				key_dn(key, map[i].key);
				break;
			}
		}
	}
#if	DUMP_KEYCODE
	printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		keymap[0], keymap[1], keymap[2], keymap[3],
		keymap[4], keymap[5], keymap[6], keymap[7]);
#endif
}

void cgenie_key_up(void *cookie, osd_key_t *key)
{
#if	DUMP_KEYCODE
	printf("keyup: %s sym:%#x uni:%#x '%c' ",
		osd_key_name(key), key->sym, key->unicode, PRINTABLE(key->unicode));
#endif
	key_up(key);
#if	DUMP_KEYCODE
	printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		keymap[0], keymap[1], keymap[2], keymap[3],
		keymap[4], keymap[5], keymap[6], keymap[7]);
#endif
}
