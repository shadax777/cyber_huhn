#include "g_local.h"


#define SEARCH_PERIOD           0.3f
#define MIN_TARGET_DISTANCE     300.0f  // if below, turn and shoot

#define PLAYER_VIEWHEIGTH       5.0f    // for nicer aiming

#define NUM_SHOTS               2       // ammo
#define RELOAD_TIME             0.4f    // weapon reload time

static void StartNewSearch(gentity_t *gself);
static void SearchTarget(entity_t *svself);                 // think()
static int TargetLost(gentity_t *gself);
static void CheckTargetDistance(entity_t *svself);          // think()
static void TurnTowardsTargetAndShoot(entity_t *svself);    // think()
static void TurnBack(entity_t *svself);                     // think()


//------------------------------------------------------
// Racer02_Start
//------------------------------------------------------
void Racer02_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_RACER02) || E_IsClass(gself, CLASS_ENEMY, ENEMY_XMAS_RACER02), G_Stringf("%s", gself->classname));

  gself->n.racer02.ammo = NUM_SHOTS;
  gself->curweapon = WP_PLASMAGUN;  // NOTE: only GLIBBERGUN will check player
                                    // collison and run mush slower than normal
  gself->sv->onCollision = xxxRacer_OnCollision;
  gself->sv->think = SearchTarget;
  gself->sv->nextthink = *e.gametime + SEARCH_PERIOD;
} // Racer02_Start


//---------------------------------------------------------------------------


//---------------------------------------
// StartNewSearch
//---------------------------------------
static void StartNewSearch(gentity_t *gself)
{
  gself->sv->think = SearchTarget;
  gself->sv->nextthink = *e.gametime + SEARCH_PERIOD;
  gself->target.gent = NULL;
} // StartNewSearch


//------------------------------------------------------
// SearchTarget
//------------------------------------------------------
// FIXME: locking the first found player is dumb
static void SearchTarget(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);
  gentity_t *cur;

  assert(!gself->target.gent);

  for(cur = EM_NextActiveEntity(NULL); cur; cur = EM_NextActiveEntity(cur))
  {
    vec3_t  dist;

    // only search for players
    if(cur->mainclass != CLASS_PLAYER)
      continue;

    // forget about entities behind self
    vec3Sub(cur->sv->position, gself->sv->position, dist);
    vec3Normalize(dist);    //+++ FIXME: obsolete?
    if(vec3DotProduct(gself->sv->direction, dist) < 0)
      continue;

    // found a player in front of self
    break;
  }

  if(cur)
  {
    gself->target.gent = cur;
    gself->target.locktime = *e.gametime;
    gself->sv->think = CheckTargetDistance;
  }
  else
  {
    // search again later
    gself->sv->nextthink += 0.5f;
  }
} // SearchTarget


//------------------------------------------------------
// TargetLost - returns 1 if target is dead or replaced by a new entity
//------------------------------------------------------
static int TargetLost(gentity_t *gself)
{
  assert(gself->target.gent);

  if(gself->target.gent->gstate != ES_ACTIVE)
    return 1;

  if(gself->target.gent->timestamp >= gself->target.locktime)
    return 1;

  return 0;
} // TargetLost


//------------------------------------------------------
// CheckTargetDistance - if near enough targert entity turn towards it
//------------------------------------------------------
static void CheckTargetDistance(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);
  vec3_t    dist;

  assert(gself->target.gent);

  // if lost target, search a new one
  if(TargetLost(gself))
  {
    StartNewSearch(gself);
    return;
  }

  vec3Sub(gself->sv->position, gself->target.gent->sv->position, dist);
  if(vec3LenSquared(dist) <= MIN_TARGET_DISTANCE * MIN_TARGET_DISTANCE)
  {
    vec3Copy(gself->sv->direction, gself->n.racer02.aimdir);
    gself->sv->think = TurnTowardsTargetAndShoot;
  }
} // CheckTargetDistance


// helper for TurnTowardsTargetAndShoot() and TurnBack()
static float Turn(gentity_t *racer, const vec3_t wishdir)
{
  float     timediff = *e.gametime - *e.lastgametime;
  vec3_t    right, up;

  G_Assert(E_IsClass(racer, CLASS_ENEMY, ENEMY_RACER02) || E_IsClass(racer, CLASS_ENEMY, ENEMY_XMAS_RACER02), G_Stringf("%i %i (%s)", racer->mainclass, racer->subclass, racer->classname));

  // turn aim-dir further towards ideal dir
  vec3MA(racer->n.racer02.aimdir, wishdir, timediff * 3, racer->n.racer02.aimdir);
  vec3Normalize(racer->n.racer02.aimdir);

  // rebuild angles for renderer
  BuildAxisByDir(right, up, racer->n.racer02.aimdir);
  VectorsToAngles(right, up, racer->n.racer02.aimdir, racer->sv->angles);

  return (float)RAD2DEG(acos(vec3Cos(racer->n.racer02.aimdir, wishdir)));
} // Turn


//------------------------------------------------------
// TurnTowardsTargetAndShoot
//------------------------------------------------------
static void TurnTowardsTargetAndShoot(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);
  vec3_t    targetpoint, wishdir;
  float     angle;

  assert(gself->target.gent);

  // if lost target, search a new one
  if(TargetLost(gself))
  {
    StartNewSearch(gself);
    return;
  }

  // get ideal aim-dir
  vec3Copy(gself->target.gent->sv->position, targetpoint);
  targetpoint[1] += PLAYER_VIEWHEIGTH;
  vec3Sub(targetpoint, gself->sv->position, wishdir);
  vec3Normalize(wishdir);

  angle = Turn(gself, wishdir);

  // Note: we approximate and will never reach the ideal aim-dir, so, we
  //       have to decide if we're close enough to go into shooting lust

  if(angle <= 6)
  {
    if(!gself->n.racer02.inshootlust)
      gself->n.racer02.inshootlust = 1;
  }

  // if in shooting lust...
  if(gself->n.racer02.inshootlust)
  {
    // if some ammo left... otherwise turn back to flight direction
    if(gself->n.racer02.ammo > 0)
    {
      if(*e.gametime - gself->n.racer02.lastshoottime >= RELOAD_TIME)
      {
        vec3_t  weaponpos;

        // compute weapon pos relative to self
        vec3MA(ZEROVEC, wishdir, 40, weaponpos);
        Weapon_Fire(gself, weaponpos, wishdir);
        gself->n.racer02.ammo--;
        gself->curmuzzlenum = (gself->curmuzzlenum + 1) % NUM_MUZZLES;
        gself->n.racer02.lastshoottime = *e.gametime;
      }
    }
    else
    {
      gself->sv->think = TurnBack;
      gself->sv->nextthink = *e.gametime + 0.2f;
      return;
    }
  }
} // TurnTowardsTargetAndShoot


//--------------------------------------------
// TurnBack
//--------------------------------------------
static void TurnBack(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);
  float     angle;

  angle = Turn(gself, gself->sv->direction);
  if(angle <= 2)    // approximation
  {
    vec3_t  dummyright, up;

    // rebuild angles for renderer
    BuildAxisByDir(dummyright, up, gself->sv->direction);
    VectorsToAngles(gself->sv->right, up, gself->sv->direction, gself->sv->angles);

    // just for completness
    vec3Copy(gself->sv->direction, gself->n.racer02.aimdir);
    gself->target.gent = NULL;
    gself->sv->think = NULL;
  }
} // TurnBack
