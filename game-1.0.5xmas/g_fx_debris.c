#include "g_local.h"


#define FADE_DELAY  4.0f    // no. of seconds to wait until start fading

static void SV_Fade(entity_t *svself); // think()
static void SV_OnCollision(entity_t *svself, entity_t *svother);

//-------------------------------
// Debris_Emit
//-------------------------------
void Debris_Emit(const vec3_t org, int n, float maxangles, float maxrotspeed,
        subclass_t debrisclass)
{
  const classinfo_t *ci;

  ci = G_GetClassInfo(CLASS_DEBRIS, debrisclass);
  G_Assert(ci, G_Stringf("debrisclass: %i", debrisclass));

  while(n-- > 0)
  {
    gentity_t   *debris;

    if((debris = EM_SpawnByClassInfo(ci)))
    {
      int   i;

      vec3Copy(org, debris->sv->position);
      vec3NormRandom(debris->sv->direction);
      debris->sv->speed = 10 + G_Random() * 10;
      debris->sv->rotSpeed[0] = G_Random() * 2 * maxrotspeed - maxrotspeed;
      debris->sv->rotSpeed[1] = G_Random() * 2 * maxrotspeed - maxrotspeed;
      for(i = 0; i < 3; i++)
        debris->sv->angles[i] = G_Random() * 2 * maxangles - maxangles;
      debris->sv->think = SV_Fade;
      debris->sv->onCollision = SV_OnCollision;
      debris->sv->nextthink = *e.gametime + FADE_DELAY;
    }
  }
} // Debris_Emit


static void SV_Fade(entity_t *svself)
{
  if((svself->color[3] -= 1.5 * (*e.gametime - *e.lastgametime)) <= 0.f)
    EM_Free(G_GEntity(svself));
}


// if colliding with a missile, emit damage particles
static void SV_OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  //if(G_InArray(gother->classid, g_missileclasses, g_nummissileclasses))
  if(gother->mainclass == CLASS_MISSILE)
  {
    EC_CheckDamageParticles(gself, gother->sv->position, gother->damage);
  }
}
