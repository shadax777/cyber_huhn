#include "g_local.h"


static void FadeAndSlowdown(entity_t *self);   // think()


//--------------------------------------------
// Feather_EmitFeathers
//--------------------------------------------
void Feather_EmitFeathers(const vec3_t org, int n)
{
  const classinfo_t *ci_feather1, *ci_feather2;

  ci_feather1 = G_GetClassInfo(CLASS_RENDERONLY, RONLY_FEATHER01);
  ci_feather2 = G_GetClassInfo(CLASS_RENDERONLY, RONLY_FEATHER02);
  assert(ci_feather1);
  assert(ci_feather2);

  while(n-- > 0)
  {
    gentity_t   *feather;
    //const char  *modelname = G_Random() > 0.5 ? CLASSNAME_FEATHER02 : CLASSNAME_FEATHER01;

    const classinfo_t   *ci = G_Random() > 0.05 ? ci_feather1 : ci_feather2;

    if((feather = EM_SpawnByClassInfo(ci)) != NULL)
    {
      int   k;

      // set feather's direction
      for(k = 0; k < 3; k++)
      {
        if(!(feather->sv->direction[k] = G_Random() - 0.5f))
          feather->sv->direction[k] = 0.0001f;
      }
      vec3Normalize(feather->sv->direction);
      // move feather a bit away from emission centre
      vec3MA(org, feather->sv->direction, 20, feather->sv->position);

      feather->sv->rotSpeed[0] = G_Random() * 200 - 100;
      feather->sv->rotSpeed[1] = G_Random() * 200 - 100;

      feather->sv->angles[0] = G_Random() * 360.0f;
      feather->sv->angles[1] = G_Random() * 360.0f;

      feather->sv->speed = G_Random() * 60;
      feather->sv->think = FadeAndSlowdown;
      feather->sv->color[3] = 0.8f;  // will fade even more in feather->think()
    }
  }
} // Feather_EmitFeathers


//---------------------------------------------------------------------------


static void FadeAndSlowdown(entity_t *self)
{
  float diff = *e.gametime - *e.lastgametime;

  if((self->color[3] -= diff * 0.5f) <= 0.0)
  {
    EM_Free(G_GEntity(self));
  }
  else
  {
    if(fabs(self->speed) > 0)
    {
      self->speed *= 1.0f - ((*e.gametime - *e.lastgametime) * 2);
    }
  }
}
