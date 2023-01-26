#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void Explode(gentity_t *self);


//----------------------------------
// Sonde_Start - start rotating around all 3 axis
//----------------------------------
void Sonde_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_SONDE), G_Stringf("%s", gself->classname));

  // set little rotation
  gself->sv->rotSpeed[0] = G_Random() * 70 - 35;
  gself->sv->rotSpeed[1] = G_Random() * 70 - 35;
  gself->sv->onCollision = OnCollision;
}


static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, NULL, Explode);
}


// emit grey particles
static void Explode(gentity_t *gself)
{
  Part_EmitRadially(gself->sv->position, 70, 30.0, g_color_grey);
  EM_Free(gself);
}

