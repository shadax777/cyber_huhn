#include "g_local.h"


// all 3 shot models have their "dir" inverted which is not correct, but this
// can be fixed by the code
#define FIX_SHOT_DIR    1


//
// homing capsule + shots
//
// exploding homing capsules emit homing shots that search and hunt a target
static void HomingCapsule_Explode(entity_t *svself);  // think() after some sec.
static void HomingCapsule_Collide(entity_t *svself, entity_t *svother);

#define HOMINGSHOT_THINK_PERIOD 0.1f
#define HOMINGSHOT_LIFE_TIME    5.0f    // if older, fade away and remove

static void HomingShot_SearchTarget(entity_t *svself);  // think()
static void HomingShot_PursueTarget(entity_t *svself);  // think()
static void HomingShot_FadeAway(entity_t *svself);      // think()


//
// snowball stuff
//
static void SnowBall_EmitSnowFlakes(entity_t *svself);  // think
static void SnowBall_Burst(entity_t *svself, entity_t *svother); // onCollision


// default collision handler
//------------------------------------------------------------------------
// OnCollision - if locked a target unlock it
//             - if colliding with debris bounce off
//             - free self
//------------------------------------------------------------------------
static void OnCollision(entity_t *svself, entity_t *svother);


typedef struct
{
  subclass_t    missileclass;
  float         scale;  // for model
  float         speed;
  float         damage;
} missileinfo_t;


static const missileinfo_t s_missiles[] =
{
  { MISSILE_PLASMA,         0.25f, 300.0f, 0.55f },
  { MISSILE_GLIBBER,        0.25f, 500.0f, 2     },
  { MISSILE_HOMINGCAPSULE,  0.50f, 400.0f, 0     },
  { MISSILE_HOMINGSHOT,     0.50f, 120.0f, 5     },
  { MISSILE_SNOWBALL,       0.35f, 500.0f, 1.5f  }
};


//----------------------------------
// FindMissileInfo
//----------------------------------
static const missileinfo_t *FindMissileInfo(subclass_t missileclass)
{
  int                   i;
  const missileinfo_t   *mi;

  for(i = 0, mi = s_missiles; i < NELEMS(s_missiles); mi++)
  {
    if(mi->missileclass == missileclass)
      return mi;
  }
  return NULL;  // not found
} // FindMissileInfo


//--------------------------------------------
// BuildMissileAnglesAndAxis - derive local axis + angles from "dir"
//--------------------------------------------
static void BuildMissileAnglesAndAxis(gentity_t *missile)
{
  vec3_t    up;
#if FIX_SHOT_DIR
  vec3_t    fixright, fixup, fixdir;
#endif

  // if missile's "dir" points straight up or down, patch it to avoid
  // zero-length for "right" and "up"
  if(vec3CoLin(missile->sv->direction, AXIS[1]))
  {
    missile->sv->direction[0] += 0.1f;
    vec3Normalize(missile->sv->direction);
  }
#if FIX_SHOT_DIR
  vec3Copy(missile->sv->direction, fixdir);
  vec3Mul(fixdir, -1);
  BuildAxisByDir(fixright, fixup, fixdir);
  VectorsToAngles(fixright, fixup, fixdir, missile->sv->angles);
  // build missile's true axis
  BuildAxisByDir(missile->sv->right, up, missile->sv->direction);
#else
  BuildAxisByDir(missile->sv->right, up, missile->sv->direction);
  VectorsToAngles(missile->sv->right, up, missile->sv->direction, missile->sv->angles);
#endif
} // BuildMissileAnglesAndAxis


//------------------------------------
// FireMissile - backend for Missile_FirePlasma(), Missile_FireHomingCapsule(),
//               Missile_FireGlibber() and Missile_FireSnowball
//------------------------------------
static gentity_t *FireMissile(subclass_t    missileclass,
                              const vec3_t  org,
                              const vec3_t  dir,
                              gentity_t     *owner)
{
  const missileinfo_t   *mi;
  const classinfo_t     *ci;
  gentity_t             *missile;

  mi = FindMissileInfo(missileclass);
  G_Assert(mi, G_Stringf("invalid missile class: %i", missileclass));

  ci = G_GetClassInfo(CLASS_MISSILE, mi->missileclass);
  G_Assert(ci, G_Stringf("missileclass: %i", mi->missileclass));

  if((missile = EM_SpawnByClassInfo(ci)))
  {
    vec3Copy(org, missile->sv->position);
    vec3Copy(dir, missile->sv->direction);

    BuildMissileAnglesAndAxis(missile);

    missile->sv->m.subtype = projectile_e; // for the engine to optimize col. det.
    missile->sv->onCollision = OnCollision;
    missile->sv->speed = mi->speed;
    missile->sv->m.scale = mi->scale;

    missile->damage = mi->damage;
    missile->owner = owner;

    // DEBUG
    //e.conprintf("// FireMissile: missile->sv->direction = %s  (%p)\n", v3s(missile->sv->direction), missile);
  }
  else
  {
    e.conprintf("# FireMissile: could not spawn missile '%s'\n", ci->classname);
  }
  return missile;
} // FireMissile



#define PLAYER_RADIUS   30  // - to detect collision with players
#define PLAYER_RADIUS_SQUARED   (PLAYER_RADIUS * PLAYER_RADIUS)


// we must manually check for player collision since the player is not included
// in the engine's collision code (player has no model)
//---------------------------------------
// CheckPlayersCollision - if near enough players, explode and shake them
//---------------------------------------
static void CheckPlayersCollision(entity_t *svself)
{
  gentity_t *gself, *gent = NULL;
  int       missile_dying = 0;  // to prevent missile from multiple deaths

  gself = G_GEntity(svself);

  while((gent = EM_NextActiveEntity(gent)))
  {
    vec3_t  dist;

    if(gent->mainclass != CLASS_PLAYER)
      continue;

    vec3Sub(gent->sv->position, gself->sv->position, dist);
    if(vec3LenSquared(dist) <= PLAYER_RADIUS_SQUARED)
    {
      vec3_t    partpos;

      // emit particles in front of player (client)
      vec3MA(gent->sv->position, gent->sv->direction, 40, partpos);
      EC_CheckDamageParticles(gent, partpos, gself->damage * 2);

      SOUND(partpos, S_EXPLOSION_GENERAL);

      Player_Shake(gent, 0.5f, 60.0f);
      if(!missile_dying)
      {
        if(gself->sv->onCollision)
          gself->sv->onCollision(gself->sv, gent->sv);
        missile_dying = 1;
      }
    }
  }
} // CheckPlayersCollision


//---------------------------------------------------------------------------


//--------------------------------------------------------------
// Missile_FirePlasma
//--------------------------------------------------------------
void Missile_FirePlasma(const vec3_t org, const vec3_t dir, gentity_t *owner)
{
  gentity_t *plasma;

  plasma = FireMissile(MISSILE_PLASMA, org, dir, owner);

  // always check for collision with player if owner is no player
  // and run much slower than normally
  if(owner->mainclass != CLASS_PLAYER)
  {
    assert(!plasma->sv->think);
    plasma->sv->think = CheckPlayersCollision;
    plasma->sv->speed *= 0.35f;
  }
} // Missile_FirePlasma


//--------------------------------------------------------------
// Missile_FireGlibber
//--------------------------------------------------------------
void Missile_FireGlibber(const vec3_t org, const vec3_t dir, gentity_t *owner)
{
  FireMissile(MISSILE_GLIBBER, org, dir, owner);
} // Missile_FireGlibber


//--------------------------------------------------------------
// Missile_FireHomingCapsule
//--------------------------------------------------------------
void Missile_FireHomingCapsule(const vec3_t org, const vec3_t dir, gentity_t *owner)
{
  gentity_t *bomb;

  if((bomb = FireMissile(MISSILE_HOMINGCAPSULE, org, dir, owner)))
  {
    bomb->sv->think = HomingCapsule_Explode;
    bomb->sv->nextthink = *e.gametime + 0.6f;
    bomb->sv->onCollision = HomingCapsule_Collide;
  }
} // Missile_FireHomingEmitter


//--------------------------------------------------------------
// Missile_FireSnowBall
//--------------------------------------------------------------
void Missile_FireSnowBall(const vec3_t org, const vec3_t dir, gentity_t *owner)
{
  gentity_t *snowball;

  if((snowball = FireMissile(MISSILE_SNOWBALL, org, dir, owner)))
  {
    snowball->sv->think = SnowBall_EmitSnowFlakes;
    snowball->sv->onCollision = SnowBall_Burst;
  }
} // Missile_FireSnowball


//---------------------------------------------------------------------------


//--------------------------------------------------------------
// HomingCapsule_Explode - emit several yellow homing shots, searching and
//                         pursuing entities
//--------------------------------------------------------------
static void HomingCapsule_Explode(entity_t *svself)
{
  gentity_t *gself;
  int       i;

  gself = G_GEntity(svself);
  SOUND(gself->sv->position, S_MISSILE_HOMINGSHOT);
  for(i = 0; i < 6; i++)
  {
    vec3_t      dir, spawnpos;
    gentity_t   *mis;

    vec3NormRandom(dir);
    vec3MA(gself->sv->position, dir, 10.0f, spawnpos);
    if((mis = FireMissile(MISSILE_HOMINGSHOT, spawnpos, dir, gself->owner)))
    {
      mis->sv->think = HomingShot_SearchTarget;
    }
  }
  EM_Free(gself);
} // HomingCapsule_Explode


//--------------------------------------------------------------
// HomingCapsule_Collide
//--------------------------------------------------------------
static void HomingCapsule_Collide(entity_t *svself, entity_t *svother)
{
  HomingCapsule_Explode(svself);
}


static gentity_t    *s_lasttargetent = NULL;
static float        s_lastlocktime = 0.0f;


//---------------------------------------------------
// HomingShot_SearchTarget
//---------------------------------------------------
static void HomingShot_SearchTarget(entity_t *svself)
{
  gentity_t         *gself;
  gentity_t         *cur, *candidate = NULL;
  float             candidatedist;

  gself = G_GEntity(svself);

  assert(!gself->target.gent);

  // if too old, fade away
  if(gself->timestamp + HOMINGSHOT_LIFE_TIME < *e.gametime)
  {
    gself->sv->think = HomingShot_FadeAway;
    return;
  }

  // check if lost last target
  if(s_lasttargetent)
  {
    if(s_lasttargetent->gstate != ES_ACTIVE
    || s_lasttargetent->timestamp >= s_lastlocktime)
      s_lasttargetent = NULL;
  }

  // search the nearest enemy: one time around the list, starting behind the
  //                           last locked one
  for(cur = EM_NextActiveEntity(s_lasttargetent); cur!=s_lasttargetent; cur = EM_NextActiveEntity(cur))
  {
    if(!cur)
      continue;

    if(cur->mainclass == CLASS_ENEMY)
    {
      if(!candidate)
      {
        vec3_t  distvec;

        candidate = cur;
        vec3Sub(candidate->sv->position, gself->sv->position, distvec);
        candidatedist = vec3Len(distvec);
      }
      else
      // check if current enemy is nearer than previous candidate
      {
        vec3_t  distvec;

        vec3Sub(cur->sv->position, gself->sv->position, distvec);
        if(vec3Len(distvec) < candidatedist)
        {
          candidate = cur;
          vec3Sub(candidate->sv->position, gself->sv->position, distvec);
          candidatedist = vec3Len(distvec);
        }
      }
    }
  }

  // if no entity was found except the already-locked one, take it
  if(!candidate)
  {
    //if(s_lasttargetent && G_InArray(s_lasttargetent->classid, g_enemyclasses, g_numenemyclasses))
    //  candidate = s_lasttargetent;
    if(s_lasttargetent && s_lasttargetent->mainclass == CLASS_ENEMY)
      candidate = s_lasttargetent;
  }

  // if we found a suitable enemy lock and pursue it
  if(candidate)
  {
    gself->target.gent = candidate;
    gself->target.locktime = *e.gametime;
    gself->sv->think = HomingShot_PursueTarget;
    gself->sv->nextthink = *e.gametime + HOMINGSHOT_THINK_PERIOD;
    s_lasttargetent = candidate;
    s_lastlocktime = *e.gametime;
  }

  // if no target found, try again at some later time
  if(!gself->target.gent)
    gself->sv->nextthink = *e.gametime + 0.1;
} // HomingShot_SearchTarget


// define this <>0 to search after a given period of time and not om each
// frame
#define PERIODICAL_SEARCH   1


//----------------------------------------------------
// HomingShot_PursueTarget - pursue target and fade away if too old
//----------------------------------------------------
static void HomingShot_PursueTarget(entity_t *svself)
{
  gentity_t *gself;
  vec3_t    wishdir;    // the ideal direction vector for pursuit

  gself = G_GEntity(svself);

  // if too old, fade away
  if(gself->timestamp + HOMINGSHOT_LIFE_TIME < *e.gametime)
  {
    gself->sv->think = HomingShot_FadeAway;
    return;
  }

  assert(gself->target.gent);

  // if target is just about to die or a new entity took its slot, we
  // must search a new target
  if(gself->target.gent->gstate != ES_ACTIVE
   || gself->target.gent->timestamp >= gself->target.locktime)
  {
    gself->sv->think = HomingShot_SearchTarget;
    gself->target.gent = NULL;
    return;
  }

  // get ideal pursuit vector
  vec3Sub(gself->target.gent->sv->position, gself->sv->position, wishdir);
  vec3Normalize(wishdir);

  // sanity check: if we're so near our target that no ideal pursuit vector
  //               could be built, the missile is supposed to collide this frame
  if(!vec3Len(wishdir))
  {
    return;
  }

  // if ideal and current pursuit vectors are collinear...
  if(vec3CoLin(wishdir, gself->sv->direction))
  {
    vec3_t  diffvec;

    vec3Sub(gself->sv->direction, wishdir, diffvec);

    // if already pointing into ideal direction, don't adjust anymore;
    // otherwise we must patch the current direction vector a bit to be able
    // to compute a new direction
    if(!vec3Len(diffvec))
    {
    #if PERIODICAL_SEARCH
      gself->sv->nextthink += HOMINGSHOT_THINK_PERIOD;
    #endif
      return;
    }
    else
    {
      int   i;

      for(i = 0; i < 3; i++)
        gself->sv->direction[i] += 0.01;
      vec3Normalize(gself->sv->direction);
    }
  }

  // build new "dir" for pursuit while guaranteeing not too fast turn

#if PERIODICAL_SEARCH
  vec3MA(wishdir, gself->sv->direction, 3, gself->sv->direction);
#else
  vec3MA(gself->sv->direction, wishdir, 3*(*e.gametime - *e.lastgametime), gself->sv->direction);
#endif
  vec3Normalize(gself->sv->direction);
  BuildMissileAnglesAndAxis(gself);

#if PERIODICAL_SEARCH
  gself->sv->nextthink += HOMINGSHOT_THINK_PERIOD;
#endif
} // HomingShot_PursueTarget


//---------------------------------------
// HomingShot_FadeAway
//---------------------------------------
static void HomingShot_FadeAway(entity_t *svself)
{
  gentity_t *gself;
  float     col;
  int       stop_fading = 0, i;

  gself = G_GEntity(svself);

  col = gself->sv->color[0];
  if((col -= (*e.gametime - *e.lastgametime)) <= 0.0f)
  {
    col = 0.0f;
    stop_fading = 1;
  }

  for(i = 0; i < 3; i++)
    gself->sv->color[i] = col;

  if(stop_fading)
  {
    // if self was the last who has locked a target, release the target, so
    // others can lock it again
    if(gself->target.gent == s_lasttargetent)
      s_lasttargetent = NULL;

    EM_Free(gself);
  }
} // HomingShot_FadeAway


//---------------------------------------------------------------------------


#define SNOWFLAKE_SIZE          (0.5f + G_Random() * 0.5f)
#define SNOWFLAKE_FADEFACTOR    0.3f


//-------------------------------------------------------------------
// SnowBall_EmitSnowFlakes - think(); periodically emit snow flakes
//-------------------------------------------------------------------
static void SnowBall_EmitSnowFlakes(entity_t *svself)   // think
{
  vec3_t    emissionaxes[3], partorg;
  int       i;

  // make snowflakes fall straight down
  AxesCopy(AXIS, emissionaxes);
  vec3Clear(emissionaxes[0]);
  vec3Mul(emissionaxes[1], -1);
  vec3Clear(emissionaxes[2]);
  Part_SetEmissionAxes(emissionaxes);
  Part_RestrictToPositiveDirections(0, 1, 0);

  Part_SetShader(PARTSHADER_SNOWFLAKE);
  Part_ScaleSize(SNOWFLAKE_SIZE);
  Part_ClampMoveSpeed(5.0f, 10.0f);
  Part_SetFadeFactor(SNOWFLAKE_FADEFACTOR);

  // manually randomize particle origin since we're only emitting
  // along a single axis
  vec3Copy(svself->position, partorg);
  for(i = 0; i < 3; i++)
  {
    partorg[i] += G_Random() * 10 - 5;
  }
  Part_EmitRadially(partorg, 1, 15.0f, g_color_white);
  svself->nextthink = *e.gametime + 0.06f;
} // SnowBall_EmitSnowFlakes


//-------------------------------------------------------------------
// SnowBall_Burst - onCollision(); emit a load of snowflakes and remove self
//-------------------------------------------------------------------
static void SnowBall_Burst(entity_t *svself, entity_t *svother)
{
  gentity_t *gself;
  vec3_t    emissionaxes[3];

  assert(svself);

  gself = G_GEntity(svself);

  // set emission axes
  vec3Copy(AXIS[0], emissionaxes[0]);
  vec3MA(ZEROVEC, AXIS[1], 0.5f, emissionaxes[1]);
  vec3Copy(AXIS[2], emissionaxes[2]);
  Part_SetEmissionAxes(emissionaxes);

  Part_SetShader(PARTSHADER_SNOWFLAKE);
  Part_ScaleSize(SNOWFLAKE_SIZE);
  Part_SetFadeTime(*e.gametime + 0.5f); // wait 1/2 sec before fading away
  Part_SetFadeFactor(SNOWFLAKE_FADEFACTOR);
  Part_SetGravity(0.4f);
  Part_ClampMoveSpeed(5.0f, 15.0f);
  Part_EmitRadially(gself->sv->position, 30, 7.0f, g_color_white);
  EM_Free(gself);
} // SnowBall_Burst


//---------------------------------------------------------------------------


//----------------------------------------------------
// BounceOff
//----------------------------------------------------
static void BounceOff(gentity_t *gself, const gentity_t *gother)
{
  vec3_t    dist;
  float     pitch, yaw;

  vec3Sub(gother->sv->position, ZEROVEC, dist);
  vec3Normalize(dist);

  // look onto the z/y plane
  pitch = AngleAroundAxisNum(dist, gself->sv->direction, 0);

  // look onto the x/z plane
  yaw   = AngleAroundAxisNum(dist, gself->sv->direction, 1);

  vec3Rotate(gself->sv->direction, pitch * 5,   0, 0);
  vec3Rotate(gself->sv->direction,     0, yaw * 5, 0);

  BuildMissileAnglesAndAxis(gself);

  // move shot into its new direction to not collide again immediately
  vec3MA(gself->sv->position, gself->sv->direction, 50.0f, gself->sv->position);
} // BounceOff


//------------------------------------------------------------------------
// OnCollision - if locked a target unlock it
//             - if colliding with debris bounce off
//             - free self
//------------------------------------------------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  assert(svself);
  assert(svother);

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);

  // if self was the last who has locked a target,
  // release the target, so others can lock it again
  if(gself->target.gent == s_lasttargetent)
    s_lasttargetent = NULL;

  // if other is debris bounce off; otherwise remove missile
  if(gother->mainclass == CLASS_DEBRIS)
  {
    BounceOff(gself, gother);
  }
  else
  {
    EM_Free(gself);
  }
} // OnCollision
