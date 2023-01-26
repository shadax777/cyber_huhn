# Makefile for the game dll, using gcc/win32
# beware: this not a very elegant Makefile, just a quit hack

# set this to 1 to compile with optimization and unstripped symbols
RELEASE = 1


ifdef PWD	# crude way to distinguish a unix shell from MSDOS, but working for me
  UNIX = 1
else
  UNIX = 0
endif


CC = gcc
DEFINES = -DSC_GAME
ifeq ($(RELEASE),1)
  DEFINES += -DNDEBUG
  CFLAGS = $(DEFINES) -Wall -c -I ../ -O3 -funroll-loops -fno-defer-pop
  LFLAGS = -s
else
  CFLAGS = $(DEFINES) -Wall -c -I ../
  LFLAGS =
endif


# ouput directory for object files - please create it if not yet existing
OBJDIR = game_objs

AI_OBJS = \
	$(OBJDIR)/ai_player.o \
	$(OBJDIR)/ai_chicken_shared.o \
	$(OBJDIR)/ai_chicken.o \
	$(OBJDIR)/ai_superchicken.o \
	$(OBJDIR)/ai_bombchicken.o \
	$(OBJDIR)/ai_motherchicken.o \
	$(OBJDIR)/ai_egg.o \
	$(OBJDIR)/ai_balloon.o \
	$(OBJDIR)/ai_racer_shared.o \
	$(OBJDIR)/ai_racer01.o \
	$(OBJDIR)/ai_racer02.o \
	$(OBJDIR)/ai_enemyship.o \
	$(OBJDIR)/ai_alien_shared.o \
	$(OBJDIR)/ai_aliengreen.o \
	$(OBJDIR)/ai_alienred.o \
	$(OBJDIR)/ai_sonde.o \
	$(OBJDIR)/ai_satellite.o \
	$(OBJDIR)/ai_rocket.o \
	$(OBJDIR)/ai_astronaut.o \
	$(OBJDIR)/ai_comet.o \
	$(OBJDIR)/ai_planet.o \
	$(OBJDIR)/ai_weaponmodel.o \
	$(OBJDIR)/ai_xmas_santa.o \
	$(OBJDIR)/ai_xmas_santa_special.o \
	$(OBJDIR)/ai_xmas_tree.o

FX_OBJS = \
	$(OBJDIR)/g_fx_score.o \
	$(OBJDIR)/g_fx_feather.o \
	$(OBJDIR)/g_fx_particle.o \
	$(OBJDIR)/g_fx_debris.o

SOUND_OBJS = \
	$(OBJDIR)/s_sound.o \
	$(OBJDIR)/s_bgm.o

SC_OBJS = \
	$(OBJDIR)/sc_io.o

GS_OBJS = \
	$(OBJDIR)/gs_main.o \
	$(OBJDIR)/gs_titlescreen.o \
	$(OBJDIR)/gs_intermission.o \
	$(OBJDIR)/gs_action.o \
	$(OBJDIR)/gs_fade.o \
	$(OBJDIR)/gs_pause.o \
	$(OBJDIR)/gs_gameover.o

MISC_OBJS = \
	$(OBJDIR)/g_hack_entitycache.o \
	$(OBJDIR)/g_misc.o \
	$(OBJDIR)/g_shared.o \
	$(OBJDIR)/g_script.o \
	$(OBJDIR)/g_import.o \
	$(OBJDIR)/g_entmanage.o \
	$(OBJDIR)/g_entchar.o \
	$(OBJDIR)/g_items.o \
	$(OBJDIR)/g_missiles.o \
	$(OBJDIR)/g_weapons.o \
	$(OBJDIR)/g_damage.o \
	$(OBJDIR)/g_level.o \
	$(OBJDIR)/client.o \
	$(OBJDIR)/g_xmas_snowcloud.o \
	$(OBJDIR)/menu.o \
	$(OBJDIR)/math3d.o \
	$(OBJDIR)/math3d_support.o

OBJS = $(MISC_OBJS) $(AI_OBJS) $(FX_OBJS) $(SOUND_OBJS) $(SC_OBJS) $(GS_OBJS)

DEF = game.def
ifeq ($(UNIX),1)
  DLL = ../data/game.dll
else
  DLL = ..\data\game.dll
endif

all:	$(DLL)

$(DLL):	$(OBJS)
	@echo - - - - - - - - - - - - - - - -
	dlltool --output-def $(DEF) $^
	dllwrap --def $(DEF) $(LFLAGS) -o $@ $^


# math3d is shared by the engine and dll code
$(OBJDIR)/math3d.o:	../math3d.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o:	%.c g_local.h g_shared.h
	$(CC) $(CFLAGS) $< -o $@

clean:
ifeq ($(UNIX),1)
	rm -f $(OBJDIR)/*.o $(DEF) $(DLL)
else
	if exist $(OBJDIR)\*.o del $(OBJDIR)\*.o
	if exist $(DEF) del $(DEF)
	if exist $(DLL) del $(DLL)
endif
