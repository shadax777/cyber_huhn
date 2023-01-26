// shared by e_racer01.c and e_racer02.c

#include "g_local.h"


static void OnDamage(gentity_t *gself, float damage);
static void OnDie(gentity_t *gself);


//-----------------------------------------
// xxxRacer_OnCollision
//-----------------------------------------
void xxxRacer_OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, OnDamage, OnDie);
} // xxxRacer_OnCollision


// emit some feathers, play pain sound
void OnDamage(gentity_t *gself, float damage)
{
  float nfeathers;

  xxxChicken_PlayRandomPainSound(gself);
  nfeathers = G_Round(damage * (5 + G_Random() * 5));
  Feather_EmitFeathers(gself->sv->position, nfeathers);
}


// emit grey particles + feathers + debris + sound
static void OnDie(gentity_t *gself)
{
  //xxxChicken_PlayRandomPainSound(self);
  SOUND(gself->sv->position, S_EXPLOSION_GENERAL);
  Part_EmitRadially(gself->sv->position, 60, 50.0f, g_color_grey);
  Feather_EmitFeathers(gself->sv->position, 30);
  Debris_Emit(gself->sv->position, 1, 0.0f, 100.0f, DEBRIS_RACER);
  EM_Free(gself);
}
