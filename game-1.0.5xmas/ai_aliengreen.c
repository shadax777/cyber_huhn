#include "g_local.h"


void AlienGreen_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_ALIEN_GREEN), G_Stringf("%s", gself->classname));

  gself->sv->think = xxxAlien_AfterBurnerAndSound;
  gself->sv->onCollision = xxxAlien_OnCollision;
}
