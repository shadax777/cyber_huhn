#include "g_local.h"


void Racer01_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_RACER01) || E_IsClass(gself, CLASS_ENEMY, ENEMY_XMAS_RACER01), G_Stringf("%s", gself->classname));

  gself->sv->onCollision = xxxRacer_OnCollision;
}
