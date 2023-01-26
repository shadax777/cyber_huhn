// like normal santa, but emitting chicken, trees, etc. and moving round its origin

#include "g_local.h"



typedef struct
{
  float         delay;
  mainclass_t   mainclass;
  subclass_t    subclass;
} goodie_t;


// santa's rucksack full of goodies; note: once all goodies have been emitted it
//                                         will restart from the beginning
static goodie_t s_rucksack[] =
{
  { 0.5f, CLASS_NEUTRAL, NEUTRAL_XMAS_TREE },
  { 0.7f, CLASS_ENEMY,   ENEMY_XMAS_CHICKEN },
  { 1.0f, CLASS_ENEMY,   ENEMY_BOMBCHICKEN },
  { 0.7f, CLASS_ENEMY,   ENEMY_EGG },
  { 0.7f, CLASS_ENEMY,   ENEMY_XMAS_CHICKEN },
  { 0.2f, CLASS_NEUTRAL, NEUTRAL_XMAS_TREE },
  { 0.3f, CLASS_ENEMY,   ENEMY_XMAS_RACER02 },
  { 0.7f, CLASS_ENEMY,   ENEMY_XMAS_CHICKEN },
  { 0.3f, CLASS_ENEMY,   ENEMY_XMAS_RACER01 },
};


// where to spawn the goodies relative to santa's pos in world
static const vec3_t s_santa_rucksack_pos = { 0, 50, -70 };


static void SvOnCollision(entity_t *svself, entity_t *svother);
static void GOnDie(gentity_t *gself);

static void Race(entity_t *svself); // think

static void CheckNewGoodieEmission(gentity_t *gself);


#define RACE_RADIUS     60.0f



//--------------------------------------------------
// XMas_Santa_Special_Start
//--------------------------------------------------
void XMas_Santa_Special_Start(gentity_t *gself)
{
  assert(gself);
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL), E_GetString(gself));

  vec3Copy(gself->sv->position, gself->n.santa.spawnpos)
  gself->n.santa.waveangle = 0.0f;
  gself->n.santa.curgoodienum = 0;
  gself->n.santa.nextemissiontime = *e.gametime + s_rucksack[0].delay + 2.0f; // wait further 2 secs

  gself->sv->speed = 0.0f;  // otherwise the enigne woudl move him along his "dir"
  gself->sv->think = Race;
  gself->sv->onCollision = SvOnCollision;
} // XMas_Santa_Special_Start


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


//---------------------------------------------------------------------
// Race
//---------------------------------------------------------------------
static void Race(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);
  float     timediff = *e.gametime - *e.lastgametime;
  vec3_t    spawndist;  // spawn pos <-> actual pos  to derive "direction" in conjunction with AXIS[1]
  vec3_t    up;

  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL), E_GetString(gself));

  // wave: [0..180[ degrees
  if((gself->n.santa.waveangle += timediff * 2) >= 180.0f)
    gself->n.santa.waveangle -= 180.0f;

  // put onto new pos
  vec3Copy(gself->n.santa.spawnpos, gself->sv->position);
  gself->sv->position[0] += sin(gself->n.santa.waveangle) * RACE_RADIUS;
  gself->sv->position[2] += cos(gself->n.santa.waveangle) * RACE_RADIUS;

  // adjust "direction" (for goodie emission)
  vec3Sub(gself->sv->position, gself->n.santa.spawnpos, spawndist);
  vec3Normalize(spawndist);
  // gnaaaa, vec3CrossProduct() mixed the input vectors
  vec3CrossProduct(spawndist, AXIS[1], gself->sv->direction);

  // adjust render angles
  BuildAxisByDir(gself->sv->right, up, gself->sv->direction);
  VectorsToAngles(gself->sv->right, up, gself->sv->direction, gself->sv->angles);

  CheckNewGoodieEmission(gself);
} // Race


//---------------------------------------------------------------------
// CheckNewGoodieEmission
//---------------------------------------------------------------------
static void CheckNewGoodieEmission(gentity_t *gself)
{
  G_Assert(E_IsClass(gself, CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL), E_GetString(gself));

  if(gself->n.santa.nextemissiontime <= *e.gametime)
  {
    const goodie_t      *goodie;
    gentity_t           *ent;   // that gets emitted
    const classinfo_t   *ci;

    goodie = &s_rucksack[gself->n.santa.curgoodienum];
    ci = G_GetClassInfo(goodie->mainclass, goodie->subclass);
    G_Assert(ci, G_Stringf("%i %i", goodie->mainclass, goodie->subclass));

    if((ent = EM_SpawnByClassInfo(ci)))
    {
      vec3_t    santa_right, santa_up;
      int       i;

      BuildAxisByDir(santa_right, santa_up, gself->sv->direction);

      // put ent at santa's pos... and ...
      vec3Copy(gself->sv->position, ent->sv->position);

      // ... and shift it to santa's ass + a bit higher
      for(i = 0; i < 3; i++)
      {
        ent->sv->position[i] += santa_right[i]          * s_santa_rucksack_pos[0];
        ent->sv->position[i] += santa_up[i]             * s_santa_rucksack_pos[1];
        ent->sv->position[i] += gself->sv->direction[i] * s_santa_rucksack_pos[2];
      }

      // set ent direction -against- santa's direction, with 45° upwards
      vec3MA(santa_up, gself->sv->direction, -1.0f, ent->sv->direction);
      vec3Normalize(ent->sv->direction);

      // rotation + speed
      ent->sv->rotSpeed[0] = G_Random() * 30 + 30;
      ent->sv->rotSpeed[1] = G_Random() * 30 + 30;
      ent->sv->speed = 30 + G_Random() * 15;

      EC_StartEntity(ent);
    }
    gself->n.santa.curgoodienum = (gself->n.santa.curgoodienum + 1) % NELEMS(s_rucksack);
    gself->n.santa.nextemissiontime = *e.gametime + s_rucksack[gself->n.santa.curgoodienum].delay;
  }
}

