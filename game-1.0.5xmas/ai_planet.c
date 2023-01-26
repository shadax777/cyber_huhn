#include "g_local.h"


// scale down to a reasonable size and set rotation around default y-axis
void Planet_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert((gself->mainclass == CLASS_NEUTRAL && (gself->subclass >= NEUTRAL_PLANET01 && gself->subclass <= NEUTRAL_PLANET08)), G_Stringf("%s", gself->classname));

  gself->sv->rotSpeed[1] = 2;
  E_Model_SetScale(gself, 0.5f);
  gself->sv->m.zbuff = false;
}
