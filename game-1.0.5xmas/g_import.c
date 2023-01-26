// g_import.c

#include <stdlib.h>
#include <windows.h>

#include "g_local.h"
#include "gs_main.h"
#include "script.h"
#include "client.h"


// dll export section
#ifdef __GNUC__
  #define EXPORT
  asm(".section .drectve");
  asm(".ascii \"-export:Game_Import\"");
  asm(".ascii \"-export:Game_Main\"");
#else
  // VC++ crap, I think
  #define EXPORT __declspec(dllexport)
#endif



game_import_t   e;


static int      s_imported = 0;     // are engine-pointers imported ?
static int      s_initialized = 0;  // is game code initialized ?

static int Init_And_Load_Level(int levelnum);


//
// g_hack_entitycache.c
//
int G_Hack_CacheClasses(const level_t *level);


//------------------------------------------------
// Game_Import - to be called once by the engine to provide engine-stuff
//------------------------------------------------
EXPORT void Game_Import(game_import_t imp)
{
  // don't import multiple times
  if(s_imported)
  {
    e.conprintf("Game_Import: pointers are already imported\n");
    return;
  }

#define CHECK(c)    \
    if(!imp.c)      \
    {               \
      MessageBox(NULL, "Game_Import: '" #c "' is NULL (which it should not!)", "tsss", MB_OK | MB_ICONSTOP);  \
      exit(EXIT_FAILURE);   \
    }

  // make sure all components are initialized
  CHECK(gametime);
  CHECK(lastgametime);
  CHECK(__spawnentity);
  CHECK(__pendremove);
  CHECK(conprintf);
  CHECK(shutdown);
  CHECK(sfx_cachefile);
  CHECK(sfx_play);
  CHECK(sfx_updatelistener3d);
  CHECK(sfx_setsoundvolume);
  CHECK(sfx_setmastervolume);
  CHECK(sfx_pauseall);
  CHECK(sfx_resumeall);
  CHECK(gametime_pause);
  CHECK(gametime_resume);
  CHECK(lang_getlocalizedstr);
  e = imp;
  s_imported = 1;
  e.conprintf("Game_Import: successfully imported all pointers\n");
  e.conprintf("Game version: %s\n", GAMEVERSION);
} // Game_Import


//------------------------------------------------
// Game_Main - to be called by the engine
//------------------------------------------------
EXPORT const void *Game_Main(int opcode, int arg0, int arg1, int arg2)
{
  if(!s_imported
  && opcode != GAME_GET_VERSION
  && opcode != GAME_MOUSE_INVERT)
  {
    MessageBox(NULL, "Game_Main: no pointers imported (shutting down)", "tsss", MB_OK | MB_ICONSTOP);
    exit(EXIT_FAILURE);
  }

  // if not yet initialized, expect command GAME_INIT
  if(!s_initialized)
  {
    if(opcode != GAME_INIT
    && opcode != GAME_GET_VERSION
    && opcode != GAME_MOUSE_INVERT)
    {
      char  s[256];

      sprintf(s, "Game_Main: requested opcode %i, but game is not initialized (exit()'ing...)", opcode);
      MessageBox(NULL, s, "tsss", MB_OK | MB_ICONSTOP);
      exit(EXIT_FAILURE);
    }
  }

  switch(opcode)
  {
    case GAME_INIT:
      if(!Init_And_Load_Level(arg0))
        return (const void *)1;
      else
        return NULL;

    case GAME_GET_CAMERA:
      return CL_GetCameraEntity()->sv;

    case GAME_KEY_EVENT:
      if(arg0 == MSG_CHAR)
      {
        assert(GS_KeyEvent);
        (*GS_KeyEvent)(arg1, arg2);
      }
      return NULL;

    case GAME_MOUSE_EVENT:  // mouse motion
      assert(GS_MouseEvent);
      (*GS_MouseEvent)(arg0, arg1, arg2);
      return NULL;

    case GAME_FRAME:
      assert(GS_RunFrame);
      (*GS_RunFrame)(G_GetCurLevel());
      return NULL;

    case GAME_GET_VERSION:  // !!this opcode doesn't need imported engine
      return GAMEVERSION;   //   functions to be imported!!

    case GAME_PARTICLE_PERCENTAGE:
      Part_SetPercentage(arg0);
      return NULL;

    case GAME_MOUSE_INVERT:
      CL_SetMouseInversion(arg0);
      return NULL;

    // 2002-10-12 requested by fps
    // CURRENTLY NOT USED
    case GAME_FRAME_END:
      return NULL;
      break;

    default:
      e.conprintf("# Game_Main: unknown opcode: %i\n", opcode);
      return NULL;
  }
} // Game_Main


//------------------------------------------------
// Init_And_Load_Level
//------------------------------------------------
static int Init_And_Load_Level(int levelnum)
{
  const level_t     *level; // loaded level
  gentity_t         *skyent;
  char              levelfilename[128];
  const classinfo_t *ci;

  e.conprintf("* Init_And_Load_Level()\n");

  if(s_initialized)
  {
    e.conprintf("# Init_And_Load_Level: WARNING: already initialized\n");
    return 1;
  }

  // attempt to load desired level
  // NOTE: it's important to load a level before any other entities that
  //       needs to be rendered (i.e. state != passive_e), since
  //       a level holds the skysphere entity which MUST come before other
  //       entities (engine restriction).
  sprintf(levelfilename, MOD_DIR "/script%i.bin", levelnum);
  e.conprintf("* Init_And_Load_Level: loading level '%s' ...", levelfilename);

  // levels #6 + #7 are x-mas levels, so set the x-mas flag accordingly
  switch(levelnum)
  {
    case 6:
    case 7:
      g_xmasrunning = 1;
      break;

    default:
      g_xmasrunning = 0;
      break;
  }
  e.conprintf("x-mas level: %s ... ", (g_xmasrunning ? "yes":"no"));

  if(!(level = G_LoadLevel(levelfilename)))
  {
    return 0;
  }
  e.conprintf("OK\n");

  //***********************************
  //
  // HACK: spawn 1 instance of each entity of the loaded level to keep the
  //       engine's cache filled.
  //
  //
  // I  H A T E  T H E  E N G I N E ' S  J A V A - O R I E N T E D  S T Y L E
  //
  // NOW, WE'VE GOT 1 FULL DAY UNTIL DEADLINE AND NOW THE BOMB, INSTANCIATED
  // BY THIS ***FUCKING*** SHIT, IS DETONATING, NOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //
  //       - THANX -
  //
  //
  // THIS MUST BE DONE BEORE **ANY** REAL ENTITY IS SPAWNED, BECAUSE THE
  // ENTITY MANAGER WILL TAKE THEESE FUCKING CACHE ENTITIES ASIDE!!!
  // THE ONLY EXCEPTION IS THE SKYSPHERE, WHICH THE ENGINE EXPECTS TO
  // BE THE VERY FIRST ENTITY.

  e.conprintf("*****************************************\n");
  e.conprintf("**** caching entities for the engine ****\n");
  if(!G_Hack_CacheClasses(level))
    return 0;
  e.conprintf("**** entities cached ****\n");
  e.conprintf("*************************\n");
  //
  //***********************************
  // now, we can fill the entity list as needed

  // spawn level's sky sphere
  ci = G_GetClassInfoByName(level->sky);
  assert(ci);
  if((skyent = EM_SpawnByClassInfo(ci)))
  {
    // everything OK
  }
  else
  {
    e.conprintf("# Init_And_Load_Level: could not load skysphere '%s'\n", level->sky);
    return 0;
  }

  G_LocalInit();

  G_CacheBGM(level->bgm);

  if(!CL_Init())
    return 0;

  // init game state module
  GS_Init();

  // start with action game state
  GS_ChangeState(GS_ACTION);

  BGM_Play(level->bgm);
  //BGM_SetVolume(0.2f);

  s_initialized = 1;
  return 1;
} // Init_And_Load_Level
