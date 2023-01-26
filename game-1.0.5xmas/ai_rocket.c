#include "g_local.h"


static void AfterBurner(entity_t *svself);  // think()
static void OnCollision(entity_t *svself, entity_t *svother);
static void Explode(gentity_t *gself);
static void OnSvFree(entity_t *svself);     // remove looping engine sound


//--------------------------
// Rocket_Start
//--------------------------
void Rocket_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_ROCKET), G_Stringf("%s", gself->classname));

  gself->sv->onCollision = OnCollision;
  gself->sv->onSrvFree = OnSvFree;

  // create a looping engine sound
  gself->n.rocket.enginesound = SoundLoop(gself->sv->position, gself->sv->direction, gself->sv->speed, S_ROCKET);
  gself->sv->think = AfterBurner;
} // Rocket_Start


//---------------------------------------------------------------------------

// peridically emit particles
static void AfterBurner(entity_t *svself)
{
  vec3_t    dummyright, up, emissionaxes[3], partorg;

  BuildAxisByDir(dummyright, up, svself->direction);

  // set emission axis
  vec3MA(ZEROVEC, svself->right, 0.05f, emissionaxes[0]);
  vec3MA(ZEROVEC, up,            0.05f, emissionaxes[1]);
  vec3MA(ZEROVEC, svself->direction, -1.0f, emissionaxes[2]);
  Part_SetEmissionAxes(emissionaxes);

  // restrict emission to back of rocket
  Part_RestrictToPositiveDirections(0, 0, 1);

  // set min/max emission speed
  Part_ClampMoveSpeed(80.0f, 100.0f);

  // set emission origin
  vec3MA(svself->position, svself->direction, -40.0f, partorg);

  Part_EmitRadially(partorg, 10, 5.0f, g_color_yellow);
  svself->nextthink = *e.gametime + 0.1f;
}


// if player's shot destroys the rocket penalize him
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, NULL, Explode);
} // OnCollision


// emit red and yellow particles, and debris
static void Explode(gentity_t *gself)
{
  vec3_t    pos;    // of debris

  SOUND(gself->sv->position, S_EXPLOSION_ROCKET);

#define PARTSCALE   1.5f

  // yellow particles
  Part_ScaleSize(PARTSCALE);
  Part_EmitRadially(gself->sv->position, 40, 50.0, g_color_yellow);

  // red particles
  Part_ScaleSize(PARTSCALE);
  Part_EmitRadially(gself->sv->position, 40, 50.0, g_color_red);

  // debris near afterburner
  vec3MA(gself->sv->position, gself->sv->direction, -20.0f, pos);
  Debris_Emit(pos, 4, 100.0f, 40.0f, DEBRIS_ROCKETWINGS);
  EM_Free(gself);
}


// remove looping engine sound
static void OnSvFree(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);

  if(gself->n.rocket.enginesound != NULL)
  {
    EM_Free(gself->n.rocket.enginesound);
    gself->n.rocket.enginesound = NULL;
  }
}
