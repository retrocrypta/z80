typedef enum {
	HC_NOTHING,		/* hit nothing */
	HC_BORDER,		/* hit a window's border */
	HC_BUTTON,		/* hit a button (face) */
	HC_CHECK,		/* hit a check box's or radio button's spot */
	HC_ICON,		/* hit an iconic window */
	HC_TITLE,		/* hit a window's title */
	HC_MENU,		/* hit a window's menu bar */
	HC_TOOLBAR,		/* hit a window's tool bar */
	HC_STATUSBAR,		/* hit a window's status bar */
	HC_SYSMENU,		/* hit a window's system menu button */
	HC_CLOSE,		/* hit a window's close button */
	HC_MINMAX,		/* hit a window's minimize/maximize button */
	HC_HS_LEFT,		/* hit a window's horizontal scrollbar left button */
	HC_HS_RIGHT,		/* hit a window's horizontal scrollbar right button */
	HC_HS_PRIOR,		/* hit a window's horizontal scrollbar prior area */
	HC_HS_NEXT,		/* hit a window's horizontal scrollbar next area */
	HC_HS_THUMB,		/* hit a window's horizontal scrollbar thumb button */
	HC_VS_UP,		/* hit a window's vertical scrollbar up button */
	HC_VS_DOWN,		/* hit a window's vertical scrollbar down button */
	HC_VS_PRIOR,		/* hit a window's vertical scrollbar prior area */
	HC_VS_NEXT,		/* hit a window's vertical scrollbar next area */
	HC_VS_THUMB,		/* hit a window's vertical scrollbar thumb button */
	HC_SIZE_NW,		/* hit a window's resize north-west edge */
	HC_SIZE_N,		/* hit a window's resize north edge */
	HC_SIZE_NE,		/* hit a window's resize north-east edge */
	HC_SIZE_E,		/* hit a window's resize east edge */
	HC_SIZE_SE,		/* hit a window's resize south-east edge */
	HC_SIZE_S,		/* hit a window's resize south edge */
	HC_SIZE_SW,		/* hit a window's resize south-west edge */
	HC_SIZE_W,		/* hit a window's resize west edge */
	HC_SPARE,
	HC_CLIENT		/* hit a window's client area */
}	hitcode_t;

typedef struct {
	int32_t x;
	int32_t y;
}	pwm_point_t;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
}	pwm_rect_t;

typedef struct {
	unsigned 

typedef struct {
	pwm_rect_t rc;

