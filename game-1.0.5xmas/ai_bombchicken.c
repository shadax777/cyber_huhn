#include "g_local.h"


#define PLAYER_RADIUS   30  // - to detect collision with players
                            // - large enough to detect collision even when
                            //   bombchicken is moving incredibly fast
                            //   (and even on so slow machines, where a
                            //    reasonable playing would be impossible)
// so we can work with squared vector length:
#define PLAYER_RADIUS_SQUARED   (PLAYER_RADIUS * PLAYER_RADIUS)


// we must manually check for player collision since the player is not included
// in the engine's collision code (player has no model)
static void CheckPlayersCollision(entity_t *svself); // think()

static void OnCollision(entity_t *svself, entity_t *svother);
static void OnDamage(gentity_t *gself, float damage);
static void OnDie(gentity_t *gself);


//--------------------------
// Bombchicken_Start - head towards player
//--------------------------
void Bombchicken_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert((gself->subclass == ENEMY_BOMBCHICKEN), G_Stringf("%s", gself->sv->className));

  gself->sv->m.animSpeed = 1.5f + G_Random() * 0.25f;
  gself->sv->think = CheckPlayersCollision;
  gself->sv->onCollision = OnCollision;  // for collision with others than player
} // Bombchicken_Start


//---------------------------------------------------------------------------


//---------------------------------------
// CheckPlayersCollision - if near enough players, explode and shake them
//---------------------------------------
static void CheckPlayersCollision(entity_t *svself)
{
  gentity_t *gent = NULL;
  int       chick_dying = 0;    // to prevent chick from multiple deaths

  while((gent = EM_NextActiveEntity(gent)))
  {
    vec3_t  dist;

    if(gent->mainclass != CLASS_PLAYER)
      continue;

    vec3Sub(gent->sv->position, svself->position, dist);
    if(vec3LenSquared(dist) <= PLAYER_RADIUS_SQUARED)
    {
      Player_Shake(gent, 1.5f, 150.0f);
      if(!chick_dying)
      {
        OnDie(G_GEntity(svself));
        chick_dying = 1;
      }
    }
  }
} // CheckPlayersCollision


//--------------------------------
// OnCollision
//--------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, OnDamage, OnDie);
}


//--------------------------------
// OnDamage
//--------------------------------
static void OnDamage(gentity_t *gself, float damage)
{
  int   nfeathers;

  xxxChicken_PlayRandomPainSound(gself);
  nfeathers = G_Round(damage * (5 + G_Random() * 5));
  Feather_EmitFeathers(gself->sv->position, nfeathers);
}


//--------------------------------
// OnDie
//--------------------------------
static void OnDie(gentity_t *gself)
{
  SOUND(gself->sv->position, S_EXPLOSION_GENERAL);
  Feather_EmitFeathers(gself->sv->position, 25);
  EM_Free(gself);
}
