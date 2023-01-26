#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void OnDamage(gentity_t *gself, float damage);
static void OnDie(gentity_t *gself);


//--------------------------
// Chicken_Start
//--------------------------
void Chicken_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_CHICKEN) || E_IsClass(gself, CLASS_ENEMY, ENEMY_XMAS_CHICKEN), G_Stringf("%s", gself->classname));

  gself->sv->m.animSpeed = 1.5f + G_Random() * 0.25f;
  gself->sv->think = xxxChicken_RandomlyCackle;
  gself->sv->nextthink = *e.gametime + G_Random() * 3.0f;
  gself->sv->onCollision = OnCollision;
} // Chicken_Start


//---------------------------------------------------------------------------


//--------------------------------
// OnCollision
//--------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, OnDamage, OnDie);
}


//--------------------------------
// OnDamage - emit feathers + speed up + sound
//--------------------------------
static void OnDamage(gentity_t *gself, float damage)
{
  int   nfeathers;
  float oldanimspeed, oldspeed;

  xxxChicken_PlayRandomPainSound(gself);
  nfeathers = G_Round(damage * (5 + G_Random() * 5));
  Feather_EmitFeathers(gself->sv->position, nfeathers);

  oldanimspeed = gself->sv->m.animSpeed;
  oldspeed = gself->sv->speed;
  gself->sv->m.animSpeed += damage;
  gself->sv->speed *= (gself->sv->m.animSpeed / oldanimspeed);

  // don't exceed certain speed
#define MAX_SPEED   60.0f
  if(gself->sv->speed > MAX_SPEED)
  {
    if(oldspeed > MAX_SPEED)    // happens if the initial speed was > MAX_SPEED
      gself->sv->speed = oldspeed;
    else
      gself->sv->speed = MAX_SPEED;
  }
}


//--------------------------------
// OnDie
//--------------------------------
static void OnDie(gentity_t *gself)
{
  xxxChicken_PlayRandomPainSound(gself);
  Feather_EmitFeathers(gself->sv->position, 25);
  EM_Free(gself);
} // OnDie
