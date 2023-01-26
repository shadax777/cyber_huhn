#include "g_local.h"
#include "client.h"


int            g_xmasrunning;   // to indicate whether we're running one
                                // of the x-mas levels
                                // set when the import module loads a level

//
// colors
//
const vec4_t    g_color_red         = COL_RED;
const vec4_t    g_color_ltred       = COL_LTRED;
const vec4_t    g_color_pinkred     = COL_PINKRED;
const vec4_t    g_color_green       = COL_GREEN;
const vec4_t    g_color_dkgreen     = COL_DKGREEN;
const vec4_t    g_color_ltgreen     = COL_LTGREEN;
const vec4_t    g_color_turquoise   = COL_TURQUOISE;
const vec4_t    g_color_blue        = COL_BLUE;
const vec4_t    g_color_ltblue      = COL_LTBLUE;
const vec4_t    g_color_yellow      = COL_YELLOW;
const vec4_t    g_color_ltorange    = COL_LTORANGE;
const vec4_t    g_color_orange      = COL_ORANGE;
const vec4_t    g_color_dkorange    = COL_DKORANGE;
const vec4_t    g_color_orangered   = COL_ORANGERED;
const vec4_t    g_color_white       = COL_WHITE;
const vec4_t    g_color_black       = COL_BLACK;
const vec4_t    g_color_grey        = COL_GREY;
const vec4_t    g_color_brown       = COL_BROWN;

//---------------------------------------------------------------------------

//
// world stuff
//

#define WORLD_BBOX_MAX  (400.0f)

static vec3_t   s_worldcentre;
static float    s_worldradius;      // (outer) radius of world sphere


//
// sound
//

static float s_mastervolume = 1.0f;


//---------------------------------------------------------------------------

//
// g_hack_entitycache.c
//
void G_Hack_FreeCachedClasses(void);    // free acquired memory


//
// g_entmanage.c
//
void EM_Hack_FreeCachedEntities(void);



//------------------------------------------------
// CacheSounds
//------------------------------------------------
static void CacheSounds(void)
{
  e.sfx_cachefile(S_CHICKEN_HERO1);
  e.sfx_cachefile(S_CHICKEN_HERO2);
  e.sfx_cachefile(S_CHICKEN_SAD1);
  e.sfx_cachefile(S_CHICKEN_PAIN1);
  e.sfx_cachefile(S_CHICKEN_PAIN2);
  e.sfx_cachefile(S_CHICKEN_SAD2);

  e.sfx_cachefile(S_WEAPON_PLASMAGUN);
  e.sfx_cachefile(S_WEAPON_GLIBBERGUN);
  e.sfx_cachefile(S_WEAPON_HOMINGGUN);
  e.sfx_cachefile(S_MISSILE_HOMINGSHOT);

  e.sfx_cachefile(S_EXPLOSION_GENERAL);
  e.sfx_cachefile(S_EXPLOSION_SHIP);
  e.sfx_cachefile(S_EXPLOSION_ROCKET);

  e.sfx_cachefile(S_ALIEN);
  e.sfx_cachefile(S_SHIP);
  e.sfx_cachefile(S_ROCKET);
} // CacheSounds


//----------------------------------------
// G_LocalInit
//----------------------------------------
void G_LocalInit(void)
{
  vec3_t    temp;

  vec3Copy(ZEROVEC, s_worldcentre);

  // compute world sphere's outer radius
  temp[0] = temp[1] = temp[2] = WORLD_BBOX_MAX;
  s_worldradius = vec3Len(temp);

  CacheSounds();
} // G_LocalInit


//--------------------------------------------
// Game_Shutdown - quit game and print player's score to stdout for the launcher
//--------------------------------------------
void Game_Shutdown(int status)
{
  const gentity_t   *player;
  int               playerscore;

  // get player entitiy to read + print his score
  player = CL_GetPlayerEntity();
  assert(player);
  assert(player->gstate == ES_ACTIVE);
  playerscore = player->n.player.score;

  BGM_Stop();
  CL_Shutdown();

  EM_FreeAll();
  G_Hack_FreeCachedClasses();
  EM_Hack_FreeCachedEntities();

  G_FreeLevels();

// local varibale in g_import.c
//  s_initialized = 0;

  switch(status)
  {
    case SHUTDOWN_FINISH:
      printf("$$score %i\n", playerscore);
      fflush(stdout);
      break;

    case SHUTDOWN_ABORT:
      printf("$$abort\n");
      fflush(stdout);
      break;

    case SHUTDOWN_FAILURE:
      printf("$$failure\n");
      fflush(stdout);
      break;

    default:
      assert(0);    // cannot happen
      break;
  }
  e.shutdown();
} // Game_Shutdown


//-----------------------------------------------
// G_GetWorldCentre
//-----------------------------------------------
/*const*/ vec3_t *G_GetWorldCentre(void)
{
  return &s_worldcentre;
} // G_GetWorldCentre


//-----------------------------------------------
// G_GetWorldRadius
//-----------------------------------------------
float G_GetWorldRadius(void)
{
  return s_worldradius;
} // G_GetWorldRadius


//----------------------------------------------------------------
// G_EntInsideSphere
//----------------------------------------------------------------
int G_EntInsideSphere(const gentity_t *gent, const vec3_t centre, float radius)
{
  vec3_t    dist;   // distance entity <-> sphere org

  vec3Sub(gent->sv->position, centre, dist);
  return vec3LenSquared(dist) <= radius * radius;
} // G_EntInsideSphere


//----------------------------------------------------------------
// E_IsClass
//----------------------------------------------------------------
int E_IsClass(const gentity_t *gent, mainclass_t mainclass, subclass_t subclass)
{
  return gent->mainclass == mainclass && gent->subclass == subclass;
}


//----------------------------------------------------------------
// E_GetString
//----------------------------------------------------------------
const char *E_GetString(const gentity_t *gent)
{
  static char   text[2048];

  if(gent != NULL)
  {
    sprintf(text, "classname = '%s'  medianame = '%s'  mainclass = %i  subclass = %i  sv = %p\n",
      gent->classname, gent->medianame, gent->mainclass, gent->subclass, gent->sv);
  }
  else
  {
    sprintf(text, "[E_GetString: got NULL]\n");
  }
  return text;
} // E_GetString



//----------------------------------------------------------------
// E_HasItem
//----------------------------------------------------------------
int E_HasItem(const gentity_t *gent, subclass_t itemclass)
{
  assert(gent);

  return (gent->item != NULL) && (gent->item->subclass == itemclass);
}


//-----------------------------------------------
// G_CacheBGM
//-----------------------------------------------
void G_CacheBGM(const char *bgmfilename)
{
  e.sfx_cachefile(bgmfilename);
}


//-----------------------------------------------
// G_GetMasterVolume
//-----------------------------------------------
float G_GetMasterVolume(void)
{
  return s_mastervolume;
}


//-----------------------------------------------
// G_SetMasterVolume
//-----------------------------------------------
void G_SetMasterVolume(float vol)
{
  if(vol >= 0.0f && vol <= 1.0f)
  {
    e.sfx_setmastervolume(vol);
    s_mastervolume = vol;
  }
  else
  {
    e.conprintf("# G_SetMasterVolume: invalud volume: %f (expected 0.0f .. 1.0f)\n", vol);
  }
} // G_SetMasterVolume
