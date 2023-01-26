#include "g_local.h"


static void SV_OnCollision(entity_t *svself, entity_t *svother);
static void G_OnDie(gentity_t *gself);
static void SV_OnSrvFree(entity_t *svself); // free looping engine sound


//--------------------------------------
// Enemyship_Start
//--------------------------------------
void Enemyship_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_ENEMYSHIP), G_Stringf("%s", gself->classname));

  gself->sv->onCollision = SV_OnCollision;

  // create a looping engine sound
  gself->n.enemyship.enginesound = SoundLoop(gself->sv->position, gself->sv->direction, gself->sv->speed, S_SHIP);
  gself->sv->onSrvFree = SV_OnSrvFree;
} // Enemyship_Start


static void SV_OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  D_CheckForDamage(gself, gother, NULL, G_OnDie);
}


// ring-explosion: emit grey and dark-orange particles;
// spawn 4 bombchickens, heading towards the first player
static void G_OnDie(gentity_t *gself)
{
  vec3_t            dummyright, up, emissionaxes[3];
  const classinfo_t *ci;
  /*const*/ gentity_t   *player;
  int               i;
  static const vec3_t bombchick_starts[] =  // relative to enemy ship
  {
    {  110, 0,  110 },
    { -110, 0,  110 },
    { -110, 0, -110 },
    {  110, 0, -110 }
  };


  SOUND(gself->sv->position, S_EXPLOSION_SHIP);

  // build some kinda plane for the ring explosion
  BuildAxisByDir(dummyright, up, gself->sv->direction);
  vec3Copy(gself->sv->right, emissionaxes[0]);
  vec3MA(ZEROVEC, up, 0.05f, emissionaxes[1]);
  vec3Copy(gself->sv->direction, emissionaxes[2]);

// params for particle emission
#define SCALE       2.5f
#define FADEFACTOR  0.2f
#define MINSPEED    10.0f
#define MAXSPEED    40.0f

  // explode orange egg in centre
  Part_ClampMoveSpeed(MINSPEED, MAXSPEED);
  Part_ScaleSize(SCALE);
  Part_SetMinRadius(30.0);
  Part_SetFadeFactor(FADEFACTOR);
  Part_EmitRadially(gself->sv->position, 50, 45.0f, g_color_dkorange);

  // explode grey ring around egg
  Part_ClampMoveSpeed(MINSPEED, MAXSPEED);
  Part_ScaleSize(SCALE);
  Part_SetMinRadius(120.0);
  Part_SetEmissionAxes(emissionaxes);
  Part_SetFadeFactor(FADEFACTOR);
  Part_EmitRadially(gself->sv->position, 240, 170.0f, g_color_grey);

  //
  // find the first player
  //
  player = NULL;
  while((player = EM_NextActiveEntity(player)))
  {
    if(player->mainclass == CLASS_PLAYER)
      break;
  }
  assert(player);

  //
  // spawn bombchicks, heading towards player
  //
  ci = G_GetClassInfo(CLASS_ENEMY, ENEMY_BOMBCHICKEN);
  assert(ci);
  for(i = 0; i < NELEMS(bombchick_starts); i++)
  {
    gentity_t   *chick;

    if((chick = EM_SpawnByClassInfo(ci)))
    {
      vec3_t    up;

      // set spawn pos
      vec3Add(gself->sv->position, bombchick_starts[i], chick->sv->position);

      // head towards player
      vec3Sub(player->sv->position, chick->sv->position, chick->sv->direction);
      if(vec3CoLin(chick->sv->direction, AXIS[1]))
        chick->sv->direction[2] -= 5;
      vec3Normalize(chick->sv->direction);
      BuildAxisByDir(chick->sv->right, up, chick->sv->direction);
      VectorsToAngles(chick->sv->right, up, chick->sv->direction, chick->sv->angles);

      chick->sv->speed = 30;
      //Bombchicken_Start(chick);
      // gnaaa! need health for bombchicken!
      EC_StartEntity(chick);
    }
    else
    {
      e.conprintf("# enemyship.OnDie: could not spawn bombchicken '%s'\n", ci->classname);
      break;
    }
  }
  EM_Free(gself);
} // G_OnDie


// free looping engine sound
static void SV_OnSrvFree(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);

  if(gself->n.enemyship.enginesound != NULL)
  {
    EM_Free(gself->n.enemyship.enginesound);
    gself->n.enemyship.enginesound = NULL;
  }
}
