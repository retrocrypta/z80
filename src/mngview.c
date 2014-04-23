/* ed:set tabstop=8 noexpandtab: */
/**************************************************************************
 *
 * mngview.c	Tool to play Mings (MNG files) with SDL
 *
 * Copyright by Juergen Buchmueller <pullmoll@t-online.de>
 *
 **************************************************************************/
#include "mng.h"
#include <SDL.h>

#if	DEBUG
void logprintf(int ll, const char *tag, const char *fmt, ...)
{
	va_list ap;

	if (ll < 3)
		return;
	fprintf(stdout, "%-8s ", tag);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}
#endif

static char title[256];
static SDL_Surface *screen;

#define	MAX_DIRTY 128
static SDL_Rect dirty[MAX_DIRTY];
static uint32_t ndirty;
int verbose;

int mng_callback(mng_t *mng, void *cookie, mng_info_t info, void *param)
{
	char *filename = (char *)cookie;
	static uint32_t ticks0;
	SDL_Event ev;
	SDL_Surface *surface;
	SDL_Rect src, dst;
	uint32_t flags;
	char *s, *p;

	switch (info) {
	case MNG_INFO_MHDR:
		if (verbose)
			printf("MHDR found\n");
		flags = SDL_HWSURFACE | SDL_ASYNCBLIT;
		screen = SDL_SetVideoMode(mng->w, mng->h, 0, flags);
		if (NULL == screen) {
			fprintf(stderr, "SDL_SetVideoMode(%d,%d,%d,%#x) failed\n",
				mng->w, mng->h, 0, flags);
			exit(1);
		}
		ticks0 = SDL_GetTicks();
		break;
	case MNG_INFO_TERM:
		if (verbose)
			printf("TERM found\n");
		break;
	case MNG_INFO_BACK:
		if (verbose)
			printf("BACK found\n");
		break;
	case MNG_INFO_PHYG:
		if (verbose)
			printf("PHYG found\n");
		break;
	case MNG_INFO_TEXT:
		if (verbose) {
			s = (char *)param;
			p = s + strlen(s) + 1;
			printf("TEXT found: %s=%s\n", s, p);
		}
		break;
	case MNG_INFO_PLTE:
		if (verbose)
			printf("PLTE found\n");
		break;
	case MNG_INFO_TRNS:
		if (verbose)
			printf("TRNS found\n");
		break;
	case MNG_INFO_TIME:
		if (verbose)
			printf("TIME found\n");
		break;
	case MNG_INFO_FRAM:
		if (verbose)
			printf("FRAM found\n");
		snprintf(title, sizeof(title), "%s: %u/%u; %u Hz",
			filename, mng->fno, mng->fcount, mng->ticks);
		if (ndirty > 0) {
			SDL_UpdateRects(screen, ndirty, dirty);
			ndirty = 0;
		}
		if (mng->ifdelay) {
			uint32_t ticks1;
			int32_t delay = mng->ticks ? 1000 / mng->ticks : 1;
			SDL_WM_SetCaption(title, title);
			ticks1 = SDL_GetTicks();
			delay = delay * mng->ifdelay;
			delay = delay - (ticks1 - ticks0);
			ticks0 = ticks1;
			if (delay > 0)
				SDL_Delay(delay);
		}
		break;
	case MNG_INFO_DEFI:
		if (verbose)
			printf("DEFI found\n");
		if (mng->lno > 1)
			break;
		dst.x = 0;
		dst.y = 0;
		dst.w = mng->w;
		dst.h = mng->h;
		SDL_FillRect(screen, &dst,
			SDL_MapRGB(screen->format, mng->pal[0], mng->pal[1], mng->pal[2]));
		SDL_UpdateRects(screen, 1, &dst);
		break;
	case MNG_INFO_IHDR:
		if (verbose)
			printf("IHDR found\n");
		src.x = 0;
		src.y = 0;
		dst.w = src.w = mng->r_cb - mng->l_cb;
		dst.h = src.h = mng->b_cb - mng->t_cb;
		dst.x = mng->xloc;
		dst.y = mng->yloc;
		if (src.w > 0 && src.h > 0) {
			surface = SDL_CreateRGBSurfaceFrom(mng->img + dst.y * mng->stride + dst.x * 4,
				src.w, src.h, 32, mng->stride, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
			if (NULL == surface) {
				fprintf(stderr, "SDL_CreateRGBSurfaceFrom(%p,%d,%d,%d,%#x,...) failed\n",
					mng->img, mng->w, mng->h, 32, mng->stride);
				exit(1);
			}
			SDL_BlitSurface(surface, &src, screen, &dst);
			SDL_FreeSurface(surface);
			if (ndirty == MAX_DIRTY) {
				SDL_UpdateRects(screen, ndirty, dirty);
				ndirty = 0;
			}
			dirty[ndirty] = dst;
			ndirty++;
		}
		break;
	case MNG_INFO_MEND:
		if (verbose)
			printf("MEND found\n");
		return 1;
	}

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYDOWN:
			switch (ev.key.keysym.sym) {
			case SDLK_ESCAPE:
				exit(1);
			default:
				/* ignore */
				break;
			}
			break;
		case SDL_QUIT:
			exit(1);
		}
	}
	return 0;
}

int usage(int argc, char **argv)
{
	char *program;
	char *slash;

	slash = strrchr(argv[0], '/');
	if (NULL == slash)
		slash = strrchr(argv[0], '\\');
	if (NULL == slash)
		program = argv[0];
	else
		program = slash + 1;
	printf("usage: %s [options] mngfile [mngfile ...]\n", program);
	return 0;
}

int main(int argc, char **argv)
{
	int i, rc;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_QUIT, SDL_ENABLE);

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'h':
				usage(argc, argv);
				return 0;
			default:
				usage(argc, argv);
				return 1;
			}
			continue;
		}
		rc = mng_read(argv[i], argv[i], mng_callback);
		if (rc < 0)
			fprintf(stderr, "reading '%s' returned %d\n", argv[i], rc);
	}

	return 0;
}
