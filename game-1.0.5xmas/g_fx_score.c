// score entities, spawned if an entity dies and rewards the inflictor
// with bonus points

#include "g_local.h"


static void EnlargeAndFade(entity_t *svself);


//---------------------------------
// Score - spawn score entity a given position
//---------------------------------
void Score(const vec3_t org, int points, float initsize, const vec4_t color)
{
  gentity_t *score;

  if((score = EM_SpawnText(TEXT_TECH)))
  {
    vec3Copy(org, score->sv->position);
    vec4Copy(color, score->sv->color);
    E_Text_SetSize(score, initsize);    // grows permanently
    E_Text_Enable3D(score, 1);
    E_Text_EnableShadow(score, 1);
    E_Text_Sprintf(score, "%i", points);
    score->sv->think = EnlargeAndFade;
  }
} // Score


//----------------------------------------------------------------------


// enlarge and fade away until no longer visible; then remove
static void EnlargeAndFade(entity_t *svself)
{
  float timediff = *e.gametime - *e.lastgametime;

  // enlarge
  svself->t.size += timediff * 20;

  // fade away
  if((svself->color[3] -= timediff * 0.7f) <= 0)
    EM_Free(G_GEntity(svself));
}
