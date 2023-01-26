#include "g_local.h"


static void SvOnCollision(entity_t *svself, entity_t *svother);
static void GOnDie(gentity_t *gself);


//--------------------------------------------------
// XMas_Tree_Start
//--------------------------------------------------
void XMas_Tree_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_NEUTRAL, NEUTRAL_XMAS_TREE), E_GetString(gself));

  gself->sv->m.scale = 0.5f;
  gself->sv->onCollision = SvOnCollision;
} // XMas_Tree_Start


static void SvOnCollision(entity_t *svself, entity_t *svother)
{
  D_CheckForDamage(G_GEntity(svself), G_GEntity(svother), NULL, GOnDie);
}


static void GOnDie(gentity_t *gself)
{
  static const struct
  {
    vec4_t  color;
    int     ammount;
  } xmasballs[] =
  {
    { COL_RED,          10  },
    //{ COL_TURQUOISE,    5  },
    //{ COL_BLUE,         5  },
    //{ COL_LTBLUE,       5  },
    { COL_YELLOW,       20 },
    { COL_LTGREEN,      10 },
  };
  int   i;

  // green particles
  Part_ScaleSize(1.5f);
  Part_EmitRadially(gself->sv->position, 50, 35.0f, g_color_dkgreen);

  // flickering x-mas balls
  for(i = 0; i < NELEMS(xmasballs); i++)
  {
    //Part_SetShader(PARTSHADER_FLICKER);
    Part_ScaleSize(0.6f);
    Part_SetFadeFactor(0.5f);
    Part_EmitRadially(gself->sv->position, xmasballs[i].ammount, 25.0f, xmasballs[i].color);
  }

  EM_Free(gself);
}
