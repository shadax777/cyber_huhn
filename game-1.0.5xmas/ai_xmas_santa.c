#include "g_local.h"


static void SvOnCollision(entity_t *svself, entity_t *svother);
static void GOnDie(gentity_t *gself);


//--------------------------------------------------
// XMas_Santa_Start
//--------------------------------------------------
void XMas_Santa_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_XMAS_SANTA), E_GetString(gself));

  gself->sv->onCollision = SvOnCollision;
} // XMas_Santa_Start


static void SvOnCollision(entity_t *svself, entity_t *svother)
{
  D_CheckForDamage(G_GEntity(svself), G_GEntity(svother), NULL, GOnDie);
}


static void GOnDie(gentity_t *gself)
{
  // red particles
  Part_EmitRadially(gself->sv->position, 60, 40.0f, g_color_red);

  // yellow particles (less than red ones)
  Part_EmitRadially(gself->sv->position, 15, 40.0f, g_color_yellow);

  EM_Free(gself);
}
