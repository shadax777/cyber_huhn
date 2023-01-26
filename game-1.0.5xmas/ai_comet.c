#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void OnDie(gentity_t *gself);


void Comet_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_NEUTRAL, NEUTRAL_COMET), G_Stringf("%s", gself->classname));

  gself->sv->onCollision = OnCollision;
  gself->sv->rotSpeed[1] = G_Random() * 50 - 25;
}


static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, NULL, OnDie);
}


// emit brown particles
static void OnDie(gentity_t *gself)
{
  Part_EmitRadially(gself->sv->position, 70, 30.0, g_color_brown);
  EM_Free(gself);
}
