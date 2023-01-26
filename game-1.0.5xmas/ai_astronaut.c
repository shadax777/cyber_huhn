#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void OnDie(gentity_t *gself);


void Astronaut_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_ASTRONAUT), G_Stringf("%s", gself->classname));

  gself->sv->rotSpeed[0] = G_Random() * 60;
  gself->sv->rotSpeed[1] = G_Random() * 60;
  gself->sv->onCollision = OnCollision;
}


static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, NULL, OnDie);
}


// emit white particles (red particles would be way cooler, hehe ;))
static void OnDie(gentity_t *gself)
{
  Part_EmitRadially(gself->sv->position, 70, 35.0, g_color_white);
  EM_Free(gself);
}
