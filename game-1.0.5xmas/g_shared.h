// shared by game code, compiler and score-tool

#ifndef __G_SHARED_H
#define __G_SHARED_H

#include <stddef.h>



// game headers
#include "math3d_support.h"

// engine headers
#include "entity.h"
#include "math3d.h"


#define NELEMS(a)   (sizeof(a) / sizeof(a[0]))

#define SIGN(x) ((x) < 0 ? -1 : 1)


//
// entity main classes
//
typedef enum
{
  CLASS_ENEMY = 1,
  CLASS_FRIEND,
  CLASS_NEUTRAL,
  CLASS_MISSILE,
  CLASS_DEBRIS,
  CLASS_RENDERONLY,    // particles + feathers
  CLASS_PLAYER,
  CLASS_CLIENT,

  CLASS_ITEM,       // bonus weapon in rucksack, time-freezer, etc.
  CLASS_IMAGE,
  CLASS_TEXT,

  // special classes
  CLASS_SOUND,

  NUM_ENTMAINCLASSES
} mainclass_t;


//
// entity sub classes
//
typedef enum
{
  // CLASS_PLAYER
  PLAYER_CLIENT = 1,
  PLAYER_BOT,

  // CLASS_CLIENT
  CLIENT_COCKPITMODEL,
  CLIENT_ARROWMODEL,
  CLIENT_WEAPONMODEL,
  CLIENT_CAMERA,

  // CLASS_ENEMY
  ENEMY_CHICKEN,       // default chicken; no weapon
  ENEMY_SUPERCHICKEN,  // like ENT_CHICKEN; superman-cape; no weapon
  ENEMY_BOMBCHICKEN,   // default chicken; weapon: bomb
  ENEMY_MOTHERCHICKEN, // big chicken, constantly emitting eggs
  ENEMY_EGG,           // emits some default chickens upon destruction
  ENEMY_BALLOON,       // metal-look; 1 chick
  ENEMY_RACER01,       // little glider; chick stands on it
  ENEMY_RACER02,       // like ENT_RACER02; chick *sits* on it + attacks player
  ENEMY_ENEMYSHIP,     // looks like a big egg, surrounded by a ring
  // NEW
  ENEMY_XMAS_CHICKEN,
  ENEMY_XMAS_RACER01,
  ENEMY_XMAS_RACER02,

  // CLASS_FRIEND
  FRIEND_ALIEN_GREEN,   // little, fast saucer; glass dome; green alien; no weapon
  FRIEND_ALIEN_RED,     // like ENT_ALIEN_GREEN; red alien
  FRIEND_SONDE,         // egg-shaped; metallic
  FRIEND_SATELLITE,     // normal satellite; penalization upon destruction
  FRIEND_ROCKET,        // friendly rocket; penalization upon destruction
  FRIEND_ASTRONAUT,     // flying through space; penalization upon destruction
  // NEW:
  FRIEND_XMAS_SANTA,
  FRIEND_XMAS_SANTA_SPECIAL,

  // CLASS_NEUTRAL
  NEUTRAL_COMET,        // little rock with small craters
  // NEW:
  NEUTRAL_XMAS_TREE,

  NEUTRAL_PLANET01,
  NEUTRAL_PLANET02,
  NEUTRAL_PLANET03,
  NEUTRAL_PLANET04,
  NEUTRAL_PLANET05,
  NEUTRAL_PLANET06,
  NEUTRAL_PLANET07,
  // NEW:
  NEUTRAL_PLANET08,
  NEUTRAL_SKYSPHERE01,
  NEUTRAL_SKYSPHERE02,
  NEUTRAL_SKYSPHERE03,
  NEUTRAL_SKYSPHERE04,
  NEUTRAL_SKYSPHERE05,
  NEUTRAL_SKYSPHERE06,
  NEUTRAL_SKYSPHERE07,

  // CLASS_MISSILE
  MISSILE_PLASMA,
  MISSILE_GLIBBER,
  MISSILE_HOMINGCAPSULE,
  MISSILE_HOMINGSHOT,
  MISSILE_SNOWBALL,

  // CLASS_DEBRIS
  DEBRIS_BALLOONSEAT,
  DEBRIS_RACER,
  DEBRIS_SATELLITE,
  DEBRIS_ROCKETWINGS,

  // CLASS_RENDERONLY
  RONLY_PARTICLE,
  RONLY_FEATHER01,
  RONLY_FEATHER02,

  // CLASS_TEXT
  TEXT_TECH,

  // CLASS_IMAGE
  IMAGE_MOUSECURSOR,
  IMAGE_ICON_PLASMA,
  IMAGE_ICON_GLIBBER,
  IMAGE_ICON_HOMING,
  IMAGE_ICON_SNOWBALL,
  IMAGE_PAUSE_BUTTON,

  // CLASS_SOUND
  SOUND_ANYTHING,

  // CLASS_ITEM
  ITEM_BONUSWEAPON,

  NUM_ENTSUBCLASSES
} subclass_t;


typedef struct
{
  // game-only
  mainclass_t   mainclass;
  subclass_t    subclass;

  // compiler / game
  const char        *classname;

  // game-only
  const char        *medianame;
  entitytype_t      sv_etype;
  objectsubtype_t   sv_objtype;

  // compiler-only
  int               is_scriptable;
} classinfo_t;


float G_Random(void);   // [0.0001 .. 1.0]
int G_Round(float value);
const char *v3s(const vec3_t v3);
const char *v4s(const vec4_t v4);
const char *G_Stringf(const char *fmt, ...);    // returns a formatted string
int G_Stricmp(const char *s, const char *t);    // case-insensitive strcmp
void G_Strncpyz(char *dest, const char *src, size_t max);   // strncpy() with guaranteed trailing zero

const classinfo_t *G_GetClassInfo(mainclass_t entclass, subclass_t subclass);
const classinfo_t *G_GetClassInfoByName(const char *classname);
const classinfo_t *G_Hack_GetClassInfoByIndex(int i); // fuck


#endif  /* !__G_SHARED_H */
