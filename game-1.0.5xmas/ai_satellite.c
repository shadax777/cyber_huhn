#include "g_local.h"


static void OnCollision(entity_t *svself, entity_t *svother);
static void Explode(gentity_t *gself);


//---------------------------------------------
// Satellite_Start
//---------------------------------------------
void Satellite_Start(gentity_t *gself)
{
  int   i;

  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_SATELLITE), G_Stringf("%s", gself->classname));

  // set random angles for renderer
  for(i = 0; i < 3; i++)
    gself->sv->angles[i] = G_Random() * 360;

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


// emit grey particles + debris
static void Explode(gentity_t *gself)
{
  Part_EmitRadially(gself->sv->position, 70, 50.0, g_color_grey);
  Debris_Emit(gself->sv->position, 1, 100.0f, 40.0f, DEBRIS_SATELLITE);
  EM_Free(gself);
}
