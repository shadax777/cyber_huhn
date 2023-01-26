
#include "g_local.h"



// muzzle position: relative to entity's weapon position
#define MUZZLE_X    (7.0f)
#define MUZZLE_Y    (-5.0f)
#define MUZZLE_Z    (30.0f)


// missiles need a target point somewhere in the distance;
// this is mainly a hack to make the missiles emitted by altering muzzles
// cross somewhere in the distance
#define TARGET_DIST (200.0f)


typedef struct
{
  weapontype_t  wptype;
  subclass_t    iconclass;
  float         shotperiod;
  void          (*FireMissile)(const vec3_t org, const vec3_t dir, gentity_t *owner);
  const char    *soundname;
  vec3_t        muzzles[NUM_MUZZLES];
} weaponinfo_t;


// available weapons
static const weaponinfo_t   s_weapons[NUM_WEAPONS] =
{
  {
    WP_PLASMAGUN,
    IMAGE_ICON_PLASMA,
    0.1f,
    Missile_FirePlasma,
    S_WEAPON_PLASMAGUN,
    {
      {             // left muzzle position
        -MUZZLE_X,  // on entity's right axis
        MUZZLE_Y,   // on entity's up axis
        MUZZLE_Z    // on entity's direction axis
      },
      {             // right muzzle position
        MUZZLE_X,
        MUZZLE_Y,
        MUZZLE_Z
      }
    }
  },

  {
    WP_GLIBBERGUN,
    IMAGE_ICON_GLIBBER,
    0.5f,
    Missile_FireGlibber,
    S_WEAPON_GLIBBERGUN,
    {
      { -MUZZLE_X, MUZZLE_Y, MUZZLE_Z + 5 }, // +5 into screen because the
      {  MUZZLE_X, MUZZLE_Y, MUZZLE_Z + 5 }  // green shot models are bigger
    }
  },

  {
    WP_HOMINGGUN,
    IMAGE_ICON_HOMING,
    0.75f,
    Missile_FireHomingCapsule,
    S_WEAPON_HOMINGGUN,
    {
      { -MUZZLE_X, MUZZLE_Y, MUZZLE_Z },
      {  MUZZLE_X, MUZZLE_Y, MUZZLE_Z }
    }
  },

  {
    WP_SNOWBALLGUN,
    IMAGE_ICON_SNOWBALL,
    0.35f,
    Missile_FireSnowBall,
    S_WEAPON_SNOWBALLGUN,
    {
      { -MUZZLE_X, MUZZLE_Y, MUZZLE_Z + 5}, // bit more into screen
      {  MUZZLE_X, MUZZLE_Y, MUZZLE_Z + 5}
    }
  }
};


//----------------------------------------
// FindWeaponInfo
//----------------------------------------
static const weaponinfo_t *FindWeaponInfo(weapontype_t wptype)
{
  int   i;

  for(i = 0; i < NUM_WEAPONS; i++)
  {
    if(s_weapons[i].wptype == wptype)
      return s_weapons + i;
  }
  return NULL;
} // FindWeaponInfo



// FIXME: "dir" should come from entity, but some entities cannot
//        affort to change their flight direction just for shooting into a
//        different direction; besides, shoots from alternating muzzles should
//        not be parallel which should only be defined from outside
//-------------------------------------------------------
// Weapon_Fire
//
// Note: "weaponpos" is relative to entity
// also note: shot rate ignored; it must be handled by caller via
//            Weapon_GetShotPeriod()
//-------------------------------------------------------
void Weapon_Fire(gentity_t *ent, const vec3_t weaponpos, const vec3_t dir)
{
  const weaponinfo_t    *wi;
  vec3_t                right, up, missileorg, missiletarget, missiledir;
  int                   i;

  // sanity check: make sure a valid muzzle is active
  if(ent->curmuzzlenum < 0 || ent->curmuzzlenum >= NUM_WEAPONS)
  {
    e.conprintf("# Weapon_Fire: '%s': invalid muzzle no.: %i (expected 0..%i)\n",
                ent->sv->className, ent->curmuzzlenum, NUM_MUZZLES - 1);
    return;
  }

  wi = FindWeaponInfo(ent->curweapon);
  if(!wi)
  {
    e.conprintf("# Weapon_Fire: '%s': invalid weapon: %i (expected 0..%i)\n",
                ent->sv->className, ent->curweapon, NUM_WEAPONS - 1);
    return;
  }

  // build "right" and "up" for correct muzzle pos
  BuildAxisByDir(right, up, dir);

  // build missile origin
  vec3Add(ent->sv->position, weaponpos, missileorg);
  for(i = 0; i < 3; i++)
  {
    missileorg[i] += right[i] * wi->muzzles[ent->curmuzzlenum][0];
    missileorg[i] += up[i]    * wi->muzzles[ent->curmuzzlenum][1];
    missileorg[i] += dir[i]   * wi->muzzles[ent->curmuzzlenum][2];
  }

  // build aim point in the distance
  vec3MA(weaponpos, dir, TARGET_DIST, missiletarget);

  // build true missile "dir" according to aim point
  vec3Sub(missiletarget, missileorg, missiledir);
  vec3Normalize(missiledir);

  wi->FireMissile(missileorg, missiledir, ent);
  SOUND(missileorg, wi->soundname);
} // Weapon_Fire


//-------------------------------------------------------------------
// Weapon_GetIconClass
//-------------------------------------------------------------------
subclass_t Weapon_GetIconClass(weapontype_t wptype)
{
  int   i;

  for(i = 0; i < NUM_WEAPONS; i++)
  {
    if(s_weapons[i].wptype == wptype)
      return s_weapons[i].iconclass;
  }
  G_Assert(0, G_Stringf("weapontype: %i", wptype));
  return 0; // shut up compiler
} // Weapon_GetIconClass


//-------------------------------------------
// Weapon_GetShotPeriod
//-------------------------------------------
float Weapon_GetShotPeriod(weapontype_t wptype)
{
  int   i;

  for(i = 0; i < NUM_WEAPONS; i++)
  {
    if(s_weapons[i].wptype == wptype)
    {
      return s_weapons[i].shotperiod;
    }
  }
  G_Assert(0, G_Stringf("weapontype: %i", wptype));
  return 0; // shut up compiler
} // Weapon_GetReloadTime
