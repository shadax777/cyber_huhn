// shared by game code, compiler and score-tool

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "g_shared.h"

static const classinfo_t    s_classinfos[] =
{
  //
  // player
  //
  { CLASS_PLAYER, PLAYER_CLIENT, "player_client", "(no media)", general_e, _invalid_e, 0 },
  { CLASS_PLAYER, PLAYER_BOT,    "player_bot",    "(no media)", general_e, _invalid_e, 0 },

  //
  // client
  //
  { CLASS_CLIENT, CLIENT_COCKPITMODEL, "client_cockpit",      "cockpit",    object_e, fixed_e, 0 },
  { CLASS_CLIENT, CLIENT_ARROWMODEL,   "client_arrow",        "arrow",      object_e, normal_e, 0 },
  { CLASS_CLIENT, CLIENT_WEAPONMODEL,  "client_weapon",       "weapon",     object_e, fixed_e, 0 },
  { CLASS_CLIENT, CLIENT_CAMERA,       "client_camera",       "(no media)", general_e, _invalid_e, 0 },

  //
  // enemies
  //
  { CLASS_ENEMY, ENEMY_CHICKEN,       "chicken",       "chicken",       object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_SUPERCHICKEN,  "superchicken",  "superchicken",  object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_BOMBCHICKEN,   "bombe",         "bombe",         object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_MOTHERCHICKEN, "motherchicken", "motherchicken", object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_EGG,           "egg",           "egg",           object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_BALLOON,       "ballon",        "ballon",        object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_RACER01,       "racer01",       "racer01",       object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_RACER02,       "racer02",       "racer02",       object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_ENEMYSHIP,     "enemy-ship",    "enemy-ship",    object_e, normal_e, 1 },
  // NEW:
  { CLASS_ENEMY, ENEMY_XMAS_CHICKEN,  "xmas_chicken",  "chicken_xmas",  object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_XMAS_RACER01,  "xmas_racer01",  "racer01_xmas",  object_e, normal_e, 1 },
  { CLASS_ENEMY, ENEMY_XMAS_RACER02,  "xmas_racer02",  "racer02_xmas",  object_e, normal_e, 1 },

  //
  // friends
  //
  { CLASS_FRIEND, FRIEND_ALIEN_GREEN, "alien01",    "alien01",        object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_ALIEN_RED,   "alien02",    "alien02",        object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_SONDE,       "sonde",      "sonde",          object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_SATELLITE,   "satellit",   "satellit",       object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_ROCKET,      "rakete",     "rakete",         object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_ASTRONAUT,   "astronaut",  "astronaut",      object_e, normal_e, 1 },
  // NEW
  { CLASS_FRIEND, FRIEND_XMAS_SANTA,         "xmas_santa",         "weihnachtsmann", object_e, normal_e, 1 },
  { CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL, "xmas_santa_special", "weihnachtsmann", object_e, normal_e, 1 },

  //
  // neutrals
  //
  { CLASS_NEUTRAL, NEUTRAL_COMET,       "komet",       "komet",       object_e, normal_e, 1 },
  // NEW
  { CLASS_NEUTRAL, NEUTRAL_XMAS_TREE,   "xmas_tree",   "tannenbaum",  object_e, normal_e, 1 },

  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE01, "skysphere01", "skysphere01", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE02, "skysphere02", "skysphere02", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE03, "skysphere03", "skysphere03", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE04, "skysphere04", "skysphere04", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE05, "skysphere05", "skysphere05", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE06, "skysphere06", "skysphere06", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_SKYSPHERE07, "skysphere07", "skysphere07", object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET01,    "p01",    "p01",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET02,    "p02",    "p02",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET03,    "p03",    "p03",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET04,    "p04",    "p04",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET05,    "p05",    "p05",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET06,    "p06",    "p06",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET07,    "p07",    "p07",         object_e, normal_e, 1 },
  { CLASS_NEUTRAL, NEUTRAL_PLANET08,    "p08",    "erde",        object_e, normal_e, 1 },

  //
  // missiles
  //
  { CLASS_MISSILE, MISSILE_PLASMA,        "missile_plasma",        "shot01",   object_e, projectile_e, 0 },
  { CLASS_MISSILE, MISSILE_GLIBBER,       "missile_glibber",       "shot03",   object_e, projectile_e, 0 },
  { CLASS_MISSILE, MISSILE_HOMINGCAPSULE, "missile_homingcapsule", "shot02",   object_e, projectile_e, 0 },
  { CLASS_MISSILE, MISSILE_HOMINGSHOT,    "missile_homingshot",    "shot02",   object_e, projectile_e, 0 },
  { CLASS_MISSILE, MISSILE_SNOWBALL,      "missile_snowball",      "snowball", object_e, projectile_e, 0 },

  //
  // debris
  //
  { CLASS_DEBRIS, DEBRIS_BALLOONSEAT, "debris_balloonseat", "truemmer01", object_e, normal_e, 0 },
  { CLASS_DEBRIS, DEBRIS_RACER,       "debris_racer",       "truemmer02", object_e, normal_e, 0 },
  { CLASS_DEBRIS, DEBRIS_SATELLITE,   "debris_satellite",   "truemmer03", object_e, normal_e, 0 },
  { CLASS_DEBRIS, DEBRIS_ROCKETWINGS, "debris_rocketwings", "truemmer04", object_e, normal_e, 0 },

  //
  // render-onlies
  //
  { CLASS_RENDERONLY, RONLY_FEATHER01, "ronly_feather01", "feather01",  object_e,   normal_e,   0 },
  { CLASS_RENDERONLY, RONLY_FEATHER02, "ronly_feather02", "feather02",  object_e,   normal_e,   0 },
  { CLASS_RENDERONLY, RONLY_PARTICLE,  "ronly_particle",  "(no media)", particle_e, _invalid_e, 0 },

  //
  // fonts
  //
  { CLASS_TEXT, TEXT_TECH,   "text_font_tech",   "tech",      screenmsg_e, _invalid_e, 0 },

  //
  // images
  //
  // NOTE: image media names are case-sensitive!!!
  { CLASS_IMAGE, IMAGE_MOUSECURSOR,   "image_mousecursor",   "cursor",       image_e, _invalid_e, 0 },
  { CLASS_IMAGE, IMAGE_ICON_PLASMA,   "image_icon_plasma",   "shot01icon",   image_e, _invalid_e, 0 },
  { CLASS_IMAGE, IMAGE_ICON_GLIBBER,  "image_icon_glibber",  "Shot03icon",   image_e, _invalid_e, 0 },
  { CLASS_IMAGE, IMAGE_ICON_HOMING,   "image_icon_homing",   "Shot02icon",   image_e, _invalid_e, 0 },
  { CLASS_IMAGE, IMAGE_ICON_SNOWBALL, "image_icon_snowball", "snowball01",   image_e, _invalid_e, 0 },
  { CLASS_IMAGE, IMAGE_PAUSE_BUTTON,  "image_pause_button",  "pause_button", image_e, _invalid_e, 0 },

  //
  // items
  //
  { CLASS_ITEM, ITEM_BONUSWEAPON, "item_bomusweapon", "(no media)", general_e, _invalid_e, 0 },
};


//---------------------------------------------------
// random number stuff:
// I had to wrote my own random number generator, because it's not
// guaranteed that feeding stdlib's seed() with same seed value on any
// system will give equal results.

#define MY_RAND_MAX     32767

// from KallistiOS
#define RNDC 1013904223
#define RNDM 1164525

static int MyRand(void)
{
  static unsigned int   s = 1;

  s = s * RNDM + RNDC;
  return (int)(s & MY_RAND_MAX);
}

//--------------------------------------------------
// G_Random - return random number in the range [0.0001 .. 1.0]
//--------------------------------------------------
float G_Random(void)
{
  return (float)(MyRand() % 10000 + 1) / 10000.0f;
}


//----------------------------------------
// G_Round
//----------------------------------------
int G_Round(float value)
{
  float vabs = fabs(value);
  int   r;

  if(vabs - (int)vabs >= 0.5f)
    r = (int)vabs + 1;
  else
    r = (int)vabs;

  return r * SIGN(value);
} // G_Round


//--------------------------------------------------
// v3s - return given vec3_t as string
//--------------------------------------------------
const char *v3s(const vec3_t v3)
{
  static char   text[128];

  sprintf(text, "%f %f %f", v3[0], v3[1], v3[2]);
  return text;
} // v3s


//--------------------------------------------------
// v4s - return given vec4_t as string
//--------------------------------------------------
const char *v4s(const vec4_t v4)
{
  static char   text[128];

  sprintf(text, "%f %f %f %f", v4[0], v4[1], v4[2], v4[3]);
  return text;
} // v4s


//----------------------------------------------------
// G_Stringf - returns a formatted string
//----------------------------------------------------
const char *G_Stringf(const char *fmt, ...)
{
  va_list       ap;
  static char   text[1024];

  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  return text;
} // G_Stringf


//--------------------------------------------------
// G_Stricmp - case-insensitive strcmp
//--------------------------------------------------
int G_Stricmp(const char *s, const char *t)
{
  while(*s && *t && (tolower(*s) == tolower(*t)))
    s++, t++;
  return *s - *t;
} // G_Stricmp


//--------------------------------------------------
// G_Strncpyz - strncpy() with guaranteed trailing zero
//--------------------------------------------------
void G_Strncpyz(char *dest, const char *src, size_t max)
{
  if(max > 0)
  {
    strncpy(dest, src, max);
    dest[max - 1] = '\0';
  }
} // G_Strncpyz


//--------------------------------------------------
// G_GetClassInfo
//--------------------------------------------------
const classinfo_t *G_GetClassInfo(mainclass_t mainclass, subclass_t subclass)
{
  int               i;
  const classinfo_t *ci;

  for(i = 0, ci = s_classinfos; i < NELEMS(s_classinfos); i++, ci++)
  {
    if(ci->mainclass == mainclass && ci->subclass == subclass)
      return ci;
  }
  return NULL;
} // G_GetClassInfo


//--------------------------------------------------
// G_GetClassInfoByName
//--------------------------------------------------
const classinfo_t *G_GetClassInfoByName(const char *classname)
{
  int               i;
  const classinfo_t *ci;

  assert(classname);

  for(i = 0, ci = s_classinfos; i < NELEMS(s_classinfos); i++, ci++)
  {
    if(!G_Stricmp(ci->classname, classname))
      return ci;
  }
  return NULL;
} // G_GetClassInfoByName


// ONLY FOR THE FUCKING HACK TO KEEP THE ENGINE'S ENTITY CACHE FILLED!!!
//--------------------------------------------------
// G_Hack_GetClassInfoByIndex
//--------------------------------------------------
const classinfo_t *G_Hack_GetClassInfoByIndex(int i)
{
  if(i >= 0 && i < NELEMS(s_classinfos))
    return s_classinfos + i;
  else
    return NULL;
} // G_Hack_GetClassInfoByIndex
