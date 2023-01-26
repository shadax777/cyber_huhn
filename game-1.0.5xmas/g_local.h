// used by game code only

#ifndef __G_LOCAL_H
#define __G_LOCAL_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

// game headers
#include "g_shared.h"
#include "g_import.h"
#include "g_strids.h"
#include "gs_main.h"
#include "math3d_support.h"

// engine headers
#include "entity.h"
#include "math3d.h"
#include "ms.h"     // keys + msg's


#define GAMEVERSION "1.0.5 xmas"

// FIXME: should be passed by the engine upon intialization!
#define MOD_DIR     "data"


// virtual resolution
#define VW  800
#define VH  600


#ifndef NDEBUG
#  define G_Assert(expr, msg) { if(!(expr)) { e.conprintf("Game assertion failed in line %i, file %s: "#expr " (%s)\n", __LINE__, __FILE__, (msg)); assert(0); } }
#else
#  define G_Assert(expr, msg) ((void)0)
#endif


//
// colors - can be used for declaration + initialization in one pass
//
#define COL_RED         { 1, 0, 0, 1 }
#define COL_LTRED       { 0.88f, 0.31f, 0.31f, 1 }
#define COL_PINKRED     { 0.63f, 0.23f, 0.39f, 1 }
#define COL_GREEN       { 0, 1, 0, 1 }
#define COL_DKGREEN     { 0, 0.7f, 0, 1 }
#define COL_LTGREEN     { 0.27f, 0.89f, 0.25f, 1 }
#define COL_TURQUOISE   { 0, 0.63f, 0.63f, 1 }
#define COL_BLUE        { 0, 0, 1, 1 }
#define COL_LTBLUE      { 0.75f, 0.94f, 1.0f, 1 }
#define COL_YELLOW      { 1, 1, 0, 1 }
#define COL_LTORANGE    { 1, 0.75f, 0, 1 }
#define COL_ORANGE      { 1, 0.65f, 0, 1 }
#define COL_DKORANGE    { 1, 0.5f, 0, 1 }
#define COL_ORANGERED   { 1, 0.27f, 0, 1 }
#define COL_WHITE       { 1, 1, 1, 1 }
#define COL_BLACK       { 0, 0, 0, 1 }
#define COL_GREY        { 0.5f, 0.5f, 0.5f, 1 }
#define COL_BROWN       { 0.44f, 0.28f, 0.12f, 1 }


//
// sounds
//
#define S_CHICKEN_HERO1 "sounds/huhn.wav"
#define S_CHICKEN_HERO2 "sounds/huhn2.wav"
#define S_CHICKEN_SAD1  "sounds/huhn3.wav"  // not used at the moment - why?
#define S_CHICKEN_PAIN1 "sounds/huhn4.wav"
#define S_CHICKEN_PAIN2 "sounds/huhn5.wav"
#define S_CHICKEN_SAD2  "sounds/huhn6.wav"  // not used at the moment - why?

#define S_WEAPON_PLASMAGUN      "sounds/shot1.wav"
#define S_WEAPON_GLIBBERGUN     "sounds/shot2.wav"
#define S_WEAPON_HOMINGGUN      "sounds/shot3a.wav"
#define S_MISSILE_HOMINGSHOT    "sounds/shot3b.wav"
#define S_WEAPON_SNOWBALLGUN    "sounds/shot2.wav"

#define S_EXPLOSION_GENERAL     "sounds/expl_general.wav"
#define S_EXPLOSION_SHIP        "sounds/expl_ship.wav"
#define S_EXPLOSION_ROCKET      "sounds/expl_rocket.wav"

#define S_ALIEN                 "sounds/alien.wav"
#define S_SHIP                  "sounds/ship.wav"
#define S_ROCKET                "sounds/rocket.wav"


//
// weapons
//
typedef enum
{
  WP_PLASMAGUN,
  WP_GLIBBERGUN,
  WP_HOMINGGUN,     // bonus weapon
  WP_SNOWBALLGUN,   // only for the x-mas addon (level #6 + #7)
  NUM_WEAPONS
} weapontype_t;


//
// particle shaders
//
typedef enum
{
  PARTSHADER_NONE,
  PARTSHADER_FLICKER,   // needs flicker perdiod (maybe write Part_SetFlickerPeriod() ?)
  PARTSHADER_SNOWFLAKE, // tottering particle
  NUM_PARTSHADERS
} partshader_t;


//
// values returned by D_CheckForDamage()
//
enum
{
  DAMAGE_NONE,  // no damage is inflicted
  DAMAGE_PAIN,  // damage is inflicted, but victim is not yet killed
  DAMAGE_KILL   // damage inflicted + killed victim
};


//
// params for Game_Shutdown()
//
enum
{
  SHUTDOWN_FINISH,  // user finished level
  SHUTDOWN_ABORT,   // user aborted level
  SHUTDOWN_FAILURE, // something went wrong and the game code shut down itself
};


//
// score info - to be used when spawning the score in the world
//
typedef struct
{
  int       value;
  float     size;   // initial size
  vec4_t    color;
} scoreinfo_t;


typedef struct gentity_s    gentity_t;


//////////////////////////////////////////////////////
//
// native data structures of different entity classes
//
//////////////////////////////////////////////////////

typedef struct  // used only by player_t
{
  void  (*oldthink)(entity_t *svself);  // think() before shaker started
  float starttime, duration;
  float nextupdatetime;
  float maxamplitude;
  float pitch, yaw;
} shaker_t;


typedef struct
{
  vec3_t    aimdir;     // when turning towards target entity
  int       ammo;
  int       inshootlust;
  float     lastshoottime;
} racer02_t;


typedef struct
{
  gentity_t *enginesound;   // looping
} enemyship_t;


typedef struct
{
  float lastparticletime;
  float lastsoundtime;
} alien_t;  // red + green


typedef struct
{
  gentity_t  *enginesound;
} rocket_t;


typedef struct
{
  int           score;
  int           num_consecutive_chickenkills;
  struct client_s   *client;
  shaker_t  shaker;
} player_t;


typedef struct
{
  int       state;      // c.f. e_weapon.c
  vec3_t    origpos;    // pos before recoil
  float     maxdist;    // max distance to original pos; if reached move back
} weaponmodel_t;


typedef struct
{
  partshader_t  shader;
  vec3_t        spawnpos;
  float         fadefactor, gravity;
  float         fadetime;   // when the particle shall start to fade away

  // native particle data
  union
  {
    struct
    {
      float period; // NOT USED ATM SINCE FLICKER PARTICLES AREN'T USED.
      float nexttime;
    } flicker;

    struct
    {
      int   initialized;
      int   isclockwise;        // rotation; set via random
      float wave;
      float maxtraveldistance;  // how far the snowflake may fall before fading away
    } snowflake;
  } n;
} particle_t;


typedef struct
{
  vec3_t    spawnpos;
  float     waveangle;
  int       curgoodienum;
  float     nextemissiontime;
} xmas_santa_special_t;


//////////////////////////////////////////////////////
//
// a game entity
//
//////////////////////////////////////////////////////
struct gentity_s
{
  entity_t  *sv;                // the only part visible on the server side

  enum
  {
    ES_FREE = 0,
    ES_ACTIVE,
    ES_PENDINGFREE
  }                 gstate;     // leading 'g' to not confuse with sv->state

  mainclass_t       mainclass;
  subclass_t        subclass;

  // for debugging
  const char        *classname, *medianame;

  float             timestamp;
  float             lastpainsoundtime;  // used to regulate pain sounds
  gentity_t         *owner;
  struct
  {
    gentity_t       *gent;
    float           locktime;
  } target;
  float             health, damage;

  gentity_t         *item;
  void              (*OnReceiveItem)(gentity_t *gself, const gentity_t *item);
  void              (*OnItemExpires)(gentity_t *gself, const gentity_t *item);

  //int               weapon_available[NUM_WEAPONS];  // not used at the moment
  weapontype_t      curweapon;
  int               curmuzzlenum;

  // used as backup of server callbacks when entities fade-in after spawn;
  // the fader will restore their original pointers when fade is over
  struct
  {
    float   nextthinktime;
    void (*Think)(entity_t *);
    void (*OnCollision)(entity_t *, entity_t *);
    void (*OnSrvFree)(entity_t *);
  } sv_callbacks;

  //void      (*OnWorldClip)(gentity_t *gself);
  void      (*OnFree)(gentity_t *gself);

  union     // native data
  {
    racer02_t               racer02;
    enemyship_t             enemyship;
    alien_t                 alien;
    rocket_t                rocket;
    player_t                player;
    weaponmodel_t           weaponmodel;
    particle_t              particle;
    xmas_santa_special_t    santa;  // FIXME: should be renamed
  } n;
};


//---------------------------------------------------------------------------


//
// g_import.c
//
extern game_import_t    e;          // g_import.c - stuff from the engine


//
// g_misc.c
//
extern int              g_xmasrunning;  // to indicate whether we're running one
                                        // of the x-mas levels
extern const vec4_t     g_color_red;
extern const vec4_t     g_color_ltred;
extern const vec4_t     g_color_pinkred;
extern const vec4_t     g_color_green;
extern const vec4_t     g_color_dkgreen;
extern const vec4_t     g_color_ltgreen;
extern const vec4_t     g_color_turquoise;
extern const vec4_t     g_color_blue;
extern const vec4_t     g_color_ltblue;
extern const vec4_t     g_color_yellow;
extern const vec4_t     g_color_ltorange;
extern const vec4_t     g_color_orange;
extern const vec4_t     g_color_dkorange;
extern const vec4_t     g_color_orangered;
extern const vec4_t     g_color_white;
extern const vec4_t     g_color_black;
extern const vec4_t     g_color_grey;
extern const vec4_t     g_color_brown;


//---------------------------------------------------------------------------
//
// management stuff
//
//---------------------------------------------------------------------------

//
// g_misc.c
//
void G_LocalInit(void);
void Game_Shutdown(int status);// quit game and print player's score to stdout
                            // for the launcher
/*const*/ vec3_t *G_GetWorldCentre(void);
float G_GetWorldRadius(void);
int G_EntInsideSphere(const gentity_t *gent, const vec3_t org, float radius);
int E_IsClass(const gentity_t *gent, mainclass_t mainclass, subclass_t subclass);
const char *E_GetString(const gentity_t *gent); // description of gent
int E_HasItem(const gentity_t *gent, subclass_t itemclass);
void G_CacheBGM(const char *bgmfilename);
float G_GetMasterVolume(void);
void G_SetMasterVolume(float vol);


//
// g_entmanage.c
//
void EM_RunFrame(void);
void EM_SetSpawnGamma(float gamma);
float EM_GetSpawnGamma(void);
// allocators
gentity_t *EM_SpawnByClassInfo(const classinfo_t *ci);
gentity_t *EM_SpawnModel(mainclass_t mainclass, subclass_t subclass);
gentity_t *EM_SpawnSound(const char *filename);  // does NOT automatically play
gentity_t *EM_SpawnImage(subclass_t imageclass);
gentity_t *EM_SpawnText(subclass_t textclass);
gentity_t *EM_SpawnParticle(void);
// EM_Free - request server to free the server-part of the
//           entity and mark it to get free on game side on next frame
void EM_Free(gentity_t *gent);
void EM_FreeAll(void);
gentity_t *G_GEntity(entity_t *svent);
gentity_t *EM_NextActiveEntity(gentity_t *last);
int EM_NumEntities(void);    // mainly for debugging; not very cheap to call
int EM_NumRenderEntities(void);// mainly for debugging; not very cheap to call
// manipulators
void E_Model_SetScale(gentity_t *ent, float scale);
void E_Model_SetAnimSpeed(gentity_t *ent, float animspeed);
void E_Sound_Play(gentity_t *ent, int looped);
void E_Sound_SetVolume(gentity_t *ent, float vol);
void E_Image_SetDimension(gentity_t *ent, float width, float height);
void E_Text_Sprintf(gentity_t *ent, const char *fmt, ...);
void E_Text_SetSize(gentity_t *ent, float size);
void E_Text_EnableShadow(gentity_t *ent, int enable);
void E_Text_Enable3D(gentity_t *ent, int enable);
void E_Particle_SetSize(gentity_t *ent, float size);


//
// g_entchar.c
//
void EC_StartEntity(gentity_t *gent);    // call startup func (if one exists) for given entity
                                        // and provide health
void EC_CheckDamageParticles(const gentity_t *victim, const vec3_t impactpoint, float damage);
void EC_OnHavingKilled(gentity_t *attacker, const gentity_t *corpse);
const scoreinfo_t *EC_GetKillScoreInfo(mainclass_t mainclass, subclass_t subclass);


//
// g_level.c
//
const struct level_s *G_LoadLevel(const char *filename);
int G_NumLoadedLevels(void);
void G_FreeLevels(void);
struct level_s *G_GetCurLevel(void);
struct level_s *G_AdvanceLevel(void);   // returns NULL if no more levels


//
// debugging aids (g_ent.c)
//
void G_ReportNonPersistentEntities(void);
void G_ReportPersistentEntities(void);
void G_ReportAllEntities(void);


//---------------------------------------------------------------------------

//
// g_item.c
//
void I_GiveItem(gentity_t *receiver, subclass_t itemclass);
float I_GetRemainingTime(const gentity_t *item);


//
// g_missile.c
//
void Missile_FirePlasma(const vec3_t org, const vec3_t dir, gentity_t *owner);
void Missile_FireGlibber(const vec3_t org, const vec3_t dir, gentity_t *owner);
void Missile_FireHomingCapsule(const vec3_t org, const vec3_t dir, gentity_t *owner);
void Missile_FireSnowBall(const vec3_t org, const vec3_t dir, gentity_t *owner);


//
// g_weapons.c
//
#define NUM_MUZZLES     2

// Weapon_Fire - "weaponpos" is relative to entity
void Weapon_Fire(gentity_t *ent, const vec3_t weaponpos, const vec3_t dir);
subclass_t Weapon_GetIconClass(weapontype_t wptype);
float Weapon_GetShotPeriod(weapontype_t wptype);


//
// g_damage.c
//
// if "other" has a damage factor, inflict damage on "self";
// retuens DAMAGE_NONE if no damage was inficted, DAMAGE_PAIN if "self" took
// damage but is not dead, DAMAGE_KILL if "self" died due to damage inflicion
int D_CheckForDamage(gentity_t *self,
                     gentity_t *other,
                     void (*OnDamage)(gentity_t *self, float damage),
                     void (*OnDie)(gentity_t *self));



//---------------------------------------------------------------------------
//
// sound
//
//---------------------------------------------------------------------------

//
// s_sound.c
//
void      Sound(const vec3_t pos, const vec3_t dir, float speed, const char *filename);
gentity_t *SoundLoop(const vec3_t pos, const vec3_t dir, float speed, const char *fn);
#define SOUND(pos, filename)    Sound((pos), ZEROVEC, 0.0f, (filename))

//
// s_bgm.c
//
void BGM_Play(const char *filename);
void BGM_Stop(void);
void BGM_SetVolume(float vol);


//---------------------------------------------------------------------------
//
// FX
//
//---------------------------------------------------------------------------

//
// g_fx_particle.c
//
void Part_SetPercentage(int percent);   // will stay persistent
void Part_SetShader(partshader_t shader);   // default: PARTSHADER_NONE
void Part_SetOnFree(void (*OnFree)(gentity_t *gself));
void Part_ScaleSize(float scale);       // default: 1.0f
void Part_SetMinRadius(float minrad);   // default: 0.0f
void Part_ClampMoveSpeed(float min, float max);     // default: [20..30]
void Part_SetEmissionAxes(/*const*/ vec3_t axes[3]);// default: AXIS (math3d.c)
void Part_RestrictToPositiveDirections(int along_right, int along_up,
            int along_dir);             // default: 0, 0, 0
void Part_SetFadeFactor(float factor);  // default: 1.0f
void Part_SetGravity(float gravity);    // default: 0.0f
void Part_SetFadeTime(float starttime); // default: 0.0f i.e. start immediately

void Part_SnowFlake_SetMaxTravelDistance(float dist);   // default: 0.0f

void Part_EmitRadially(const vec3_t org, int n, float maxrad, const vec4_t col);

//
// g_fx_feather.c
//
void Feather_EmitFeathers(const vec3_t org, int n);

//
// g_fx_debris.c
//
void Debris_Emit(const vec3_t org, int n, float maxangles, float maxrotspeed,
        subclass_t debrisclass);

//
// g_fx_score.c
//
void Score(const vec3_t org, int points, float initsize, const vec4_t color);


//---------------------------------------------------------------------------
//
// entities
//
//---------------------------------------------------------------------------

//
// ai_player.c
//
void Player_Start(gentity_t *gself);
void Player_Shake(gentity_t *gself, float duration, float maxamplitude);
int Player_IsShaking(const gentity_t *gself);
void Player_Yaw(gentity_t *gself, float ang);
void Player_Pitch(gentity_t *gself, float ang);
void Player_Finish(gentity_t *gself);
void Player_Reset(gentity_t *gself);
void Player_OnHavingKilled(gentity_t *gself, const gentity_t *corpse);

//
// ai_chicken_shared.c
//
void xxxChicken_RandomlyCackle(entity_t *svself);   // think()
void xxxChicken_PlayRandomPainSound(gentity_t *gself);

//
// ai_chicken.c
//
void Chicken_Start(gentity_t *self);

//
// ai_superchicken.c
//
void Superchicken_Start(gentity_t *self);

//
// ai_bombchicken.c
//
void Bombchicken_Start(gentity_t *self);

//
// ai_motherchicken.c
//
void Motherchicken_Start(gentity_t *self);

//
// ai_egg.c
//
void Egg_Start(gentity_t *self);

//
// ai_balloon.c
//
void Balloon_Start(gentity_t *self);

//
// ai_racer_shared.c
//
void xxxRacer_OnCollision(entity_t *svself, entity_t *svother);

//
// ai_racer01.c
//
void Racer01_Start(gentity_t *self);

//
// ai_racer02.c
//
void Racer02_Start(gentity_t *self);

//
// ai_enemyship.c
//
void Enemyship_Start(gentity_t *self);

//
// ai_alien_shared.c
//
void xxxAlien_AfterBurnerAndSound(entity_t *svself);
void xxxAlien_OnCollision(entity_t *svself, entity_t *svother);

//
// ai_aliengreen.c
//
void AlienGreen_Start(gentity_t *self);

//
// ai_alienred.c
//
void AlienRed_Start(gentity_t *self);

//
// ai_sonde.c
//
void Sonde_Start(gentity_t *self);

//
// ai_satellite.c
//
void Satellite_Start(gentity_t *self);

//
// ai_rocket.c
//
void Rocket_Start(gentity_t *self);

//
// ai_astronaut.c
//
void Astronaut_Start(gentity_t *self);

//
// ai_comet.c
//
void Comet_Start(gentity_t *self);

//
// ai_planet.c
//
void Planet_Start(gentity_t *self);

//
// ai_weaponmodel.c
//
void Weaponmodel_Recoil(gentity_t *gself, float maxdist, float recoilspeed);

//
// ai_xmas_santa.c
//
void XMas_Santa_Start(gentity_t *gself);

//
// ai_xmas_santa_special.c
//
void XMas_Santa_Special_Start(gentity_t *gself);

//
// ai_xmas_tree.c
//
void XMas_Tree_Start(gentity_t *gself);


#endif  /* !__G_LOCAL_H */
