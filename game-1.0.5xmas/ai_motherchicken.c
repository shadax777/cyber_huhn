#include "g_local.h"

#define EGGEMISSION_PERIOD  8.0f

static void EmitEgg(entity_t *svself);
static void OnCollision(entity_t *svself, entity_t *svother);
static void OnDamage(gentity_t *gself, float damage);
static void OnDie(gentity_t *gself);


//--------------------------
// Motherchicken_Start
//--------------------------
void Motherchicken_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_ENEMY, ENEMY_MOTHERCHICKEN), G_Stringf("%s", gself->classname));

  gself->sv->m.scale = 5.0f;
  gself->sv->m.animSpeed = 1.5f + G_Random() * 0.25f;
  gself->sv->think = EmitEgg;
  gself->sv->nextthink = *e.gametime + EGGEMISSION_PERIOD;
  gself->sv->onCollision = OnCollision;
} // Motherchicken_Start


//---------------------------------------------------------------------------


//-----------------------------------
// EmitEgg
//-----------------------------------
static void EmitEgg(entity_t *svself)
{
  gentity_t         *gself, *egg;
  float             tmp;
  const char        *soundname;

  gself = G_GEntity(svself);
  if((egg = EM_SpawnModel(CLASS_ENEMY, ENEMY_EGG)))
  {
    vec3_t  up, eggpos;

    // position the egg behind and a bit beneath the motherchicken
    vec3CrossProduct(gself->sv->right, gself->sv->direction, up);
    vec3MA(gself->sv->position, gself->sv->direction, -60, eggpos);
    vec3MA(eggpos, up, -20, eggpos);
    vec3Copy(eggpos, egg->sv->position);

    // set egg direction + speed
    vec3Copy(gself->sv->direction, egg->sv->direction);
    vec3Mul(egg->sv->direction, -1);
    egg->sv->speed = 10.0f;

    //Egg_Start(egg);
    // gnaaaa! EC_StartEntity() must provide the egg's health
    EC_StartEntity(egg);
  }

  // play one of 4 random sounds
  tmp = G_Random();
  if(tmp < 0.25f)
    soundname = S_CHICKEN_SAD1;
  else if(tmp < 0.50f)
    soundname = S_CHICKEN_SAD2;
  else if(tmp < 0.75f)
    soundname = S_CHICKEN_HERO1;
  else
    soundname = S_CHICKEN_HERO2;
  // play the sound twice to make it appear louder
  SOUND(gself->sv->position, soundname);
  SOUND(gself->sv->position, soundname);
  gself->sv->nextthink = *e.gametime + EGGEMISSION_PERIOD;
} // EmitEgg



// the motherchicken is so huge that we should emit feather not at its
// origin, but at the impact point of the shot
static vec3_t   s_impactpoint;


//--------------------------------
// OnCollision
//--------------------------------
static void OnCollision(entity_t *svself, entity_t *svother)
{
  gentity_t *gself, *gother;

  gself = G_GEntity(svself);
  gother = G_GEntity(svother);
  vec3Copy(gother->sv->position, s_impactpoint);
  D_CheckForDamage(gself, gother, OnDamage, OnDie);
}


//--------------------------------
// OnDamage - emit feathers + sound
//--------------------------------
static void OnDamage(gentity_t *gself, float damage)
{
  int   nfeathers;

  xxxChicken_PlayRandomPainSound(gself);
  nfeathers = G_Round(damage * (5 + G_Random() * 5));
  nfeathers *= 2;   // because the motherchicken is so huge
  Feather_EmitFeathers(s_impactpoint, nfeathers);
}


//--------------------------------
// OnDie
//--------------------------------
static void OnDie(gentity_t *gself)
{
  xxxChicken_PlayRandomPainSound(gself);
  Feather_EmitFeathers(gself->sv->position, 75);
  EM_Free(gself);
} // OnDie
