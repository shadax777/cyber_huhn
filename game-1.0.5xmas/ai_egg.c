#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void StartRotation(gentity_t *self, const gentity_t *other);
static void DecreaseRotation(entity_t *svself); // think()
static void OnDie(gentity_t *self);      // emit some chickens


#define MAX_ROTSPEED    250.0f


//----------------------------
// Egg_Start
//----------------------------
void Egg_Start(gentity_t *self)
{
  assert(self);
  G_Assert(E_IsClass(self, CLASS_ENEMY, ENEMY_EGG), G_Stringf("%s", self->classname));

  self->sv->onCollision = OnCollision;

  // set initial rotation
  self->sv->rotSpeed[0] = G_Random() * MAX_ROTSPEED / 3;
  self->sv->rotSpeed[1] = G_Random() * MAX_ROTSPEED / 3;
  if(vec3CoLin(self->sv->direction, AXIS[1]))
    self->sv->rotSpeed[2] = G_Random() * MAX_ROTSPEED / 3;  // looks better

  self->sv->angles[0] = G_Random() * 360;
  self->sv->angles[1] = G_Random() * 360;
} // Egg_Start


//----------------------------------------------------
// OnCollision - if taken damage but not killed, change "dir" and
//               rotation according to "other"
//----------------------------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  if(D_CheckForDamage(gself, gother, NULL, OnDie) == DAMAGE_PAIN)
  {
    vec3_t  temp;

    // set new speed + direction
    vec3Add(gself->sv->direction, gother->sv->direction, temp);
    gself->sv->speed += vec3Len(temp) * 3.0f;
    vec3Normalize(temp);
    vec3Copy(temp, gself->sv->direction);

    StartRotation(gself, gother);
  }
} // OnCollision


//-------------------------------------------------------
// StartRotation - set rotation according to impact point
//-------------------------------------------------------
static void StartRotation(gentity_t *self, const gentity_t *other)
{
  vec3_t    dist;
  float     pitch, yaw;

  // FIXME: it's not safe to assume shot came from { 0, 0, 0 }
  vec3Sub(self->sv->position, ZEROVEC, dist);

  pitch = AngleAroundAxisNum(dist, other->sv->direction, 0);
  yaw   = AngleAroundAxisNum(dist, other->sv->direction, 1);
  self->sv->rotSpeed[0] += pitch * 50;
  self->sv->rotSpeed[1] += yaw * 50;

  // clamp rotation speeds
  if(fabs(self->sv->rotSpeed[0]) > MAX_ROTSPEED)
    self->sv->rotSpeed[0] = SIGN(self->sv->rotSpeed[0]) * 200.0f;
  if(fabs(self->sv->rotSpeed[1]) > MAX_ROTSPEED)
    self->sv->rotSpeed[1] = SIGN(self->sv->rotSpeed[1]) * 200.0f;
  self->sv->think = DecreaseRotation;
} // StartRotation


//----------------------------------------
// DecreaseRotation - constantly decrease egg's rotation speeds
//----------------------------------------
static void DecreaseRotation(entity_t *self)
{
  float timediff = *e.gametime - *e.lastgametime;
  int   i;

  for(i = 0; i < 3; i++)
  {
    vec_t   oldrotspeed = self->rotSpeed[i];

    self->rotSpeed[i] -= SIGN(self->rotSpeed[i]) * timediff * fabs(self->rotSpeed[i]) * 0.3f;

    // if rotation direction changed or became very small, we stop rotating
    if(SIGN(self->rotSpeed[i]) != SIGN(oldrotspeed) || fabs(self->rotSpeed[i]) < 1.0f)
      self->rotSpeed[i] = 0.0f;
    }

  // if no longer rotating around any axis...
  if(!vec3Len(self->rotSpeed))
    self->think = NULL;
} // DecreaseRotation


//----------------------------------------
// OnDie - emit 6 chickens and particles
//----------------------------------------
static void OnDie(gentity_t *self)
{
  const classinfo_t *info;
  int               i;

  info = G_GetClassInfo(CLASS_ENEMY, ENEMY_CHICKEN);
  assert(info);
  for(i = 0; i < 6; i++)
  {
    gentity_t   *chick;

    if((chick = EM_SpawnByClassInfo(info)))
    {
      vec3_t    up;

      vec3NormRandom(chick->sv->direction);

      // move chick a bit away from centre of egg
      vec3MA(self->sv->position, chick->sv->direction, 30, chick->sv->position);

      BuildAxisByDir(chick->sv->right, up, chick->sv->direction);
      VectorsToAngles(chick->sv->right, up, chick->sv->direction, chick->sv->angles);

      chick->sv->rotSpeed[0] = G_Random() * 30 + 30;
      chick->sv->rotSpeed[1] = G_Random() * 30 + 30;
      chick->sv->speed = 15 + G_Random() * 10;

      //Chicken_Start(chick);
      // gnaaaa! the chick must get health
      EC_StartEntity(chick);
    }
    else
    {
      e.conprintf("e_egg.c: OnDie: could not spawn chicken '%s'\n", info->classname);
      break;
    }
  }
  Part_EmitRadially(self->sv->position, 70, 50.0, g_color_white);
  EM_Free(self);
} // OnDie
