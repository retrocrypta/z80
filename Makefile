APPLICATION=	z80
VERSION_MAJOR=	0
VERSION_MINOR=	3
VERSION_MICRO=	1
VERSION=	$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

DEBUG?=		1

ifeq ($(shell uname -o 2>/dev/null),Cygwin)
WINDOWS?=	1
else
WINDOWS?=	0
endif

CC=		@gcc
CFLAGS=

LD=		@gcc
LDFLAGS=

SRC:=		src
OBJ:=		obj
BIN:=		bin
INCLUDES:=	-I./include -I/usr/local/include -I/usr/pkg/include
LIBS:=		-lz

#CFLAGS+=	-DDEBUG=$(DEBUG)

# SDL libraries and cflags
SDL_LIB:=	$(shell sdl2-config --libs)
SDL_INC:=	$(shell sdl2-config --cflags)

# Expat libraries and cflags
EXPAT_LIB:=	-L/usr/pkg/lib -lexpat

ifeq	($(strip $(WINDOWS)),1)
EXE=		.exe
CFLAGS+=	-mno-cygwin
LDFLAGS+=	-mno-cygwin
else
EXE=
endif

ifeq	($(strip $(DEBUG)),1)
CFLAGS+= 	-O0 -fno-strict-aliasing -g -Wall $(SDL_INC) $(INCLUDES)
LDFLAGS+=
else
CFLAGS+= 	-O2 -fno-strict-aliasing -fomit-frame-pointer -Wall $(SDL_INC) $(INCLUDES)
LDFLAGS+=	-s
endif

OSD_OBJS= $(OBJ)/osd.o $(OBJ)/osd_bitmap.o $(OBJ)/osd_font.o $(OBJ)/osd_gui.o

TRS80_OBJS=	$(OBJ)/trs80/main.o $(OBJ)/trs80/kbd.o $(OBJ)/trs80/fdc.o $(OBJ)/trs80/cas.o \
		$(OBJ)/system.o $(OBJ)/timer.o $(OBJ)/image.o \
		$(OBJ)/floppy.o $(OBJ)/crc.o $(OBJ)/wd179x.o \
		$(OBJ)/z80.o $(OBJ)/z80dasm.o \
		$(OSD_OBJS)

CGENIE_OBJS=	$(OBJ)/cgenie/main.o $(OBJ)/cgenie/kbd.o $(OBJ)/cgenie/fdc.o $(OBJ)/cgenie/cas.o\
		$(OBJ)/system.o $(OBJ)/timer.o $(OBJ)/image.o \
		$(OBJ)/floppy.o $(OBJ)/crc.o $(OBJ)/wd179x.o \
		$(OBJ)/mc6845.o $(OBJ)/ay8910.o \
		$(OBJ)/z80.o $(OBJ)/z80dasm.o \
		$(OSD_OBJS)

DMKTOOL_OBJS=	$(OBJ)/dmktool.o

CAS2XML_OBJS=	$(OBJ)/cas2xml.o $(OBJ)/sha1.o

XML2CAS_OBJS=	$(OBJ)/xml2cas.o $(OBJ)/sha1.o $(OBJ)/unicode.o

CMD2CAS_OBJS=	$(OBJ)/cmd2cas.o

DZ80_OBJS=	$(OBJ)/dz80.o $(OBJ)/z80dasm.o


all:	.dirs $(BIN)/cgenie$(EXE) $(BIN)/trs80$(EXE) $(BIN)/dmktool$(EXE) \
	$(BIN)/cas2xml$(EXE) $(BIN)/xml2cas$(EXE) \
	$(BIN)/cmd2cas$(EXE) $(BIN)/dz80$(EXE)

.dirs:
	@mkdir -p $(OBJ) 2>/dev/null
	@mkdir -p $(OBJ)/trs80 2>/dev/null
	@mkdir -p $(OBJ)/cgenie 2>/dev/null
	@mkdir -p $(BIN) 2>/dev/null

$(BIN)/trs80$(EXE):	$(TRS80_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(SDL_LIB) $(LIBS)

$(BIN)/cgenie$(EXE):	$(CGENIE_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(SDL_LIB) $(LIBS)

$(BIN)/dmktool$(EXE):	$(DMKTOOL_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN)/cas2xml$(EXE):	$(CAS2XML_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN)/xml2cas$(EXE):	$(XML2CAS_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(EXPAT_LIB) $(LIBS)

$(BIN)/cmd2cas$(EXE):	$(CMD2CAS_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(EXPAT_LIB) $(LIBS)

$(BIN)/dz80$(EXE):	$(DZ80_OBJS)
	@echo "==> linking $@"
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJ)/%.o:	$(SRC)/%.c
	@echo "==> compiling $@"
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(OBJ) $(BIN) *.core `find . -iname "*.bck"`

dist:	clean
	(cd ..; tar cvzf $(APPLICATION)-$(VERSION).tgz \
		z80/COPYING z80/README z80/Makefile \
		z80/include z80/src \
		z80/trs80/trs80.rom z80/trs80/trs80.chr \
			z80/trs80/cas/README \
			z80/trs80/fd/README \
		z80/cgenie/cgenie.rom z80/cgenie/cgenie.chr \
		z80/cgenie/cgdos.rom z80/cgenie/cgenie.def \
			z80/cgenie/cas/README \
			z80/cgenie/cas/androm.cas \
			z80/cgenie/cas/chop83.cas \
			z80/cgenie/cas/paint.cas \
			z80/cgenie/cas/paint2.cas \
			z80/cgenie/cas/saug.cas \
			z80/cgenie/fd/README \
			z80/cgenie/fd/fd0.img \
			z80/cgenie/fd/fd1.img \
			z80/cgenie/fd/fd2.img \
			z80/cgenie/fd/fd3.img \
		)

winsetup:	all
	windows/setup.sh -dir $(shell pwd) -app $(APPLICATION) -ver $(VERSION)
