// shared by e_aliengreen.c and e_alienred.c
// NOTE: the shaking process does NOT change the alien's direction, it only
//       changes its rotation for the renderer.


#include "g_local.h"


#define AFTERBURNER_PERIOD  0.1f
#define SOUND_PERIOD        2.0f


static void AfterBurner(gentity_t *gself);
static void OnDie(gentity_t *gself);  // ring explosion + remove alien


//-------------------------------------------
// xxxAlien_AfterBurnerAndSound - periodically emit grey particles + play
//                                sound
//-------------------------------------------
void xxxAlien_AfterBurnerAndSound(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);

  if(*e.gametime - gself->n.alien.lastparticletime >= AFTERBURNER_PERIOD)
  {
    AfterBurner(gself);
    gself->n.alien.lastparticletime = *e.gametime;
  }

  if(*e.gametime - gself->n.alien.lastsoundtime >= SOUND_PERIOD)
  {
    SOUND(gself->sv->position, S_ALIEN);
    gself->n.alien.lastsoundtime = *e.gametime;
  }
} // xxxAlien_AfterBurnerAndSound


//-------------------------------------------
// xxxAlien_OnCollision
//-------------------------------------------
void xxxAlien_OnCollision(entity_t *self, entity_t *other)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(self);
  gother = G_GEntity(other);
  D_CheckForDamage(gself, gother, NULL, OnDie);
} // xxxAlien_OnCollision


// emit particles at ass
static void AfterBurner(gentity_t *gself)
{
  vec3_t    dummyright, up, emissionaxes[3], partorg;

  BuildAxisByDir(dummyright, up, gself->sv->direction);

  // set emission axis
  vec3MA(ZEROVEC, gself->sv->right,      0.5f, emissionaxes[0]);
  vec3MA(ZEROVEC, up,                    0.5f, emissionaxes[1]);
  vec3MA(ZEROVEC, gself->sv->direction, -1.0f, emissionaxes[2]);
  Part_SetEmissionAxes(emissionaxes);

  // restrict emission to back of alien ship
  Part_RestrictToPositiveDirections(0, 0, 1);

  // set emission origin
  vec3MA(gself->sv->position, gself->sv->direction, -23.0f, partorg);
  vec3MA(partorg, up, -3.5f, partorg);

  // make particles ascend
  Part_SetGravity(-3.0f);

  Part_EmitRadially(partorg, 5, 5.0f, g_color_grey);
} // AfterBurner


// sound, emit green (red) and grey particles
static void OnDie(gentity_t *gself)
{
  const vec4_t  *color;    // color of alien's glass bell
  vec3_t        dummyright, up, emissionaxes[3];

  SOUND(gself->sv->position, S_EXPLOSION_GENERAL);

  // get glass color
  switch(gself->subclass)
  {
    case FRIEND_ALIEN_GREEN:
      color = &g_color_turquoise;;
      break;

    case FRIEND_ALIEN_RED:
      color = &g_color_pinkred;
      break;

    default:    // for future expansion
      color = &g_color_black;
      break;
  }

#define FADEFACTOR  0.2f
#define MINSPEED    5.0f
#define MAXSPEED    10.0f

  // emit glass particles
  Part_ClampMoveSpeed(MINSPEED, MAXSPEED);
  Part_SetFadeFactor(FADEFACTOR);
  Part_EmitRadially(gself->sv->position, 40, 10.0, *color);

  // build a plane for particle emission
  BuildAxisByDir(dummyright, up, gself->sv->direction);
  vec3Copy(gself->sv->right, emissionaxes[0]);
  vec3MA(ZEROVEC, up, 0.05f, emissionaxes[1]);
  vec3Copy(gself->sv->direction, emissionaxes[2]);

  // emit particles across plane
  Part_ClampMoveSpeed(MINSPEED, MAXSPEED);
  Part_SetFadeFactor(FADEFACTOR);
  Part_SetEmissionAxes(emissionaxes); // FIXME: compiler: "...incompatible pointer type"
  Part_SetMinRadius(25.0f);
  Part_EmitRadially(gself->sv->position, 80, 40.0, g_color_grey);

  EM_Free(gself);
} // OnDie
