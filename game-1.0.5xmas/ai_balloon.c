#include "g_local.h"


// I still haven't decided whether to spawn feathers or a chicken upon death
#define SPAWN_FEATHERS  1


static void OnCollision(entity_t *svself, entity_t *svother);
static void StartYawing(gentity_t *gself, const gentity_t *shot);
static void DecreaseYawing(entity_t *svself); // think()
static void OnDie(gentity_t *gself);


//--------------------------------------
// Balloon_Start
//--------------------------------------
void Balloon_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_BALLOON), G_Stringf("%s", gself->classname));

  // reset angles which were set by spawn event, to make the balloon look as if
  // it was collinear to the default y-axis
  vec3Copy(ZEROVEC, gself->sv->angles);
  gself->sv->onCollision = OnCollision;
} // Balloon_Start


//--------------------------------------
// OnCollision
//--------------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);

  if(D_CheckForDamage(gself, gother, NULL, OnDie) == DAMAGE_PAIN)
  {
    StartYawing(gself, gother);
  }
} // OnCollision


//--------------------------------------
// StartYawing
//--------------------------------------
static void StartYawing(gentity_t *gself, const gentity_t *shot)
{
  vec3_t    dist;
  float     angle;

  vec3Sub(gself->sv->position, ZEROVEC, dist);
  angle = AngleAroundAxisNum(dist, shot->sv->direction, 1);
  gself->sv->rotSpeed[1] += angle * 5;
  gself->sv->think = DecreaseYawing;
} // StartYawing


//----------------------------------------
// DecreaseYawing - constantly decrease balloon's yawing speed
//----------------------------------------
static void DecreaseYawing(entity_t *svself)
{
  vec_t oldyaw = svself->rotSpeed[1];
  float timediff = *e.gametime - *e.lastgametime;

  svself->rotSpeed[1] -= SIGN(svself->rotSpeed[1]) * timediff * fabs(svself->rotSpeed[1]) * 0.7f;

  // if rotation direction changed or became very small, we stop rotating
  if(SIGN(svself->rotSpeed[1]) != SIGN(oldyaw) || fabs(svself->rotSpeed[1]) < 1.0f)
  {
    svself->rotSpeed[1] = 0.0f;
    svself->think = NULL;
  }
} // DecreaseYawing


//--------------------------------------
// OnDie - emit some red + grey particles and feathers (or a chicken?)
//        (haven't decided yet)
//--------------------------------------
static void OnDie(gentity_t *self)
{
  vec3_t            pos;

#if !SPAWN_FEATHERS
  gentity_t         *chicken;
#endif

#define PARTSCALE   1.5f

  // red particles on top
  vec3Copy(self->sv->position, pos);
  pos[1] += 40;
  Part_ScaleSize(PARTSCALE);
  Part_SetMinRadius(40);
  Part_EmitRadially(pos, 60, 50.0, g_color_ltred);

  // grey particles on bottom
  pos[1] = self->sv->position[1] - 30;
  Part_ScaleSize(PARTSCALE);
  Part_EmitRadially(pos, 40, 50.0, g_color_grey);
  pos[1] += 20; // fixes ballon seat debris which has its centre somewhere on
                // its y-axis
  //Debris_Emit(pos, 1, 0.0f, 0.0f, CLASSNAME_DEBRIS01);
  Debris_Emit(pos, 1, 0.0f, 0.0f, DEBRIS_BALLOONSEAT);

#if SPAWN_FEATHERS
  pos[1] = self->sv->position[1] - 30;
  Feather_EmitFeathers(pos, 25);
#else   // spawn chicken
  if((chicken = EM_SpawnModel(CLASS_ENEMY, ENEMY_CHICKEN)))
  {
    vec3_t  up;

    vec3Copy(self->sv->position, chicken->sv->position);
    chicken->sv->position[1] -= 30;
    vec3NormRandom(chicken->sv->direction);
    chicken->sv->rotSpeed[0] = G_Random() * 30 + 30;
    chicken->sv->rotSpeed[1] = G_Random() * 30 + 30;
    BuildAxisByDir(chicken->sv->right, up, chicken->sv->direction);
    VectorsToAngles(chicken->sv->right, up, chicken->sv->direction, chicken->sv->angles);
    chicken->sv->speed = 5 + G_Random() * 10;
    chicken->sv->m.animSpeed = 2.0;

    //Chicken_Start(chicken);
    // gnaaa!!, chicken needs health
    EC_StartEntity(chicken);
  }
#endif

  EM_Free(self);
} // OnDie
