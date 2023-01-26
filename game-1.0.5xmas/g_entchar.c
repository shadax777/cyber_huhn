// entity characteristics

#include "g_local.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// kill score stuff
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// scoreinfo_t: { int value; float size; vec4_t color }
#define SCOREINFO_10        {  10, 20, COL_YELLOW }
#define SCOREINFO_25        {  25, 20, COL_YELLOW }
#define SCOREINFO_50        {  50, 20, COL_LTORANGE }
#define SCOREINFO_75        {  75, 20, COL_ORANGE }
#define SCOREINFO_100       { 100, 20, COL_LTORANGE }
#define SCOREINFO_150       { 150, 30, COL_ORANGE }
#define SCOREINFO_MINUS_50  { -50, 20, COL_RED }
#define SCOREINFO_MINUS_75  { -75, 20, COL_RED }


// score rewarded when killing an entity
typedef struct
{
  mainclass_t   mainclass;
  subclass_t    subclass;
  scoreinfo_t   scoreinfo;
} killscore_t;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// startup stuff
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// startup info for each some entity classes
typedef struct
{
  mainclass_t   mainclass;
  subclass_t    subclass;
  float         health;
  float         damage;
  void          (*start)(gentity_t *self);
} startinfo_t;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// particle emission stuff
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// entities can be divided into distinct regions, each emitting particles in
// a different color upon missile impact.
// IMPORTANT: for each set of emissions always leave an non-regional emission
// as default (in case none of the regions will be hit)
typedef struct
{
  vec4_t    partcolor;
  int       is_region;
  struct
  {
    vec3_t  pos;    // relative to entity
    float   radius;
  } region;         // ignored if !is_region
} partemission_t;


#define MAX_PARTEMISSIONS   4

typedef struct
{
  mainclass_t       mainclass;
  subclass_t        subclass;
  int               numemissions;
  partemission_t    emissions[MAX_PARTEMISSIONS];
} pain_partemission_t;



//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


static const killscore_t    s_killscores[] =
{
  { CLASS_ENEMY, ENEMY_CHICKEN,         SCOREINFO_25 },
  { CLASS_ENEMY, ENEMY_SUPERCHICKEN,    SCOREINFO_50 },
  { CLASS_ENEMY, ENEMY_BOMBCHICKEN,     SCOREINFO_75 },
  { CLASS_ENEMY, ENEMY_MOTHERCHICKEN,   SCOREINFO_100 },
  { CLASS_ENEMY, ENEMY_EGG,             SCOREINFO_100 },
  { CLASS_ENEMY, ENEMY_BALLOON,         SCOREINFO_10 },
  { CLASS_ENEMY, ENEMY_RACER01,         SCOREINFO_50 },
  { CLASS_ENEMY, ENEMY_RACER02,         SCOREINFO_50 },
  { CLASS_ENEMY, ENEMY_ENEMYSHIP,       SCOREINFO_150 },
  // NEW:
  { CLASS_ENEMY, ENEMY_XMAS_CHICKEN,    SCOREINFO_25 },
  { CLASS_ENEMY, ENEMY_XMAS_RACER01,    SCOREINFO_50 },
  { CLASS_ENEMY, ENEMY_XMAS_RACER02,    SCOREINFO_50 },

  { CLASS_FRIEND, FRIEND_ALIEN_GREEN,     SCOREINFO_MINUS_50 },
  { CLASS_FRIEND, FRIEND_ALIEN_RED,       SCOREINFO_MINUS_50 },
  { CLASS_FRIEND, FRIEND_SONDE,           SCOREINFO_MINUS_50 },
  { CLASS_FRIEND, FRIEND_SATELLITE,       SCOREINFO_MINUS_50 },
  { CLASS_FRIEND, FRIEND_ROCKET,          SCOREINFO_MINUS_50 },
  { CLASS_FRIEND, FRIEND_ASTRONAUT,       SCOREINFO_MINUS_50 },
  // NEW
  { CLASS_FRIEND, FRIEND_XMAS_SANTA,      SCOREINFO_MINUS_75 },
  { CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL,      SCOREINFO_MINUS_75 },
};


static const startinfo_t    s_startinfos[] =
{
  { CLASS_ENEMY, ENEMY_CHICKEN,       2,  0, Chicken_Start },
  { CLASS_ENEMY, ENEMY_SUPERCHICKEN,  3,  0, Superchicken_Start },
  { CLASS_ENEMY, ENEMY_BOMBCHICKEN,   2,  0, Bombchicken_Start },
  { CLASS_ENEMY, ENEMY_MOTHERCHICKEN,15,  0, Motherchicken_Start },
  { CLASS_ENEMY, ENEMY_EGG,          12,  0, Egg_Start },
  { CLASS_ENEMY, ENEMY_BALLOON,       5,  0, Balloon_Start },
  { CLASS_ENEMY, ENEMY_RACER01,       4,  2, Racer01_Start },
  { CLASS_ENEMY, ENEMY_RACER02,       4,  2, Racer02_Start },
  { CLASS_ENEMY, ENEMY_ENEMYSHIP,    15, 10, Enemyship_Start },
  // NEW:
  { CLASS_ENEMY, ENEMY_XMAS_CHICKEN,  2,  0, Chicken_Start },
  { CLASS_ENEMY, ENEMY_XMAS_RACER01,  4,  2, Racer01_Start },
  { CLASS_ENEMY, ENEMY_XMAS_RACER02,  4,  2, Racer02_Start },

  { CLASS_FRIEND, FRIEND_ALIEN_GREEN,   2,  5, AlienGreen_Start },
  { CLASS_FRIEND, FRIEND_ALIEN_RED,     2,  5, AlienRed_Start },
  { CLASS_FRIEND, FRIEND_SONDE,         1,  0, Sonde_Start },
  { CLASS_FRIEND, FRIEND_SATELLITE,     1,  0, Satellite_Start },
  { CLASS_FRIEND, FRIEND_ROCKET,        1, 10, Rocket_Start },
  { CLASS_FRIEND, FRIEND_ASTRONAUT,     1,  0, Astronaut_Start },
  // NEW
  { CLASS_FRIEND, FRIEND_XMAS_SANTA,    2.1f,   1, XMas_Santa_Start },
  { CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL,    2.1f,   1, XMas_Santa_Special_Start },

  { CLASS_NEUTRAL, NEUTRAL_COMET,         1,  1, Comet_Start },
  // NEW
  { CLASS_NEUTRAL, NEUTRAL_XMAS_TREE,     5,  0, XMas_Tree_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET01,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET02,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET03,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET04,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET05,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET06,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET07,      0,  0, Planet_Start },
  { CLASS_NEUTRAL, NEUTRAL_PLANET08,      0,  0, Planet_Start },
};


static pain_partemission_t s_pain_partemissions[] =
{
  {
    CLASS_PLAYER, PLAYER_CLIENT, 1,
    {
      { COL_LTBLUE, 0 }
    }
  },

  {
    CLASS_PLAYER, PLAYER_BOT, 1,
    {
      { COL_LTBLUE, 0 }
    }
  },

  {
    CLASS_ENEMY, ENEMY_EGG, 1,
    {
      { COL_WHITE, 0 }
    }
  },

  {
    CLASS_ENEMY, ENEMY_BALLOON, 2,
    {
      { COL_LTRED, 1, { {0,40,0}, 50.0f } },
      { COL_GREY,  0 }
    }
  },

  {
    CLASS_ENEMY, ENEMY_RACER01, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_ENEMY, ENEMY_RACER02, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_ENEMY, ENEMY_ENEMYSHIP, 2,
    {
      { COL_DKORANGE, 1, { {0,0,0}, 70.0f } },  // orange "egg" in the middle
      { COL_GREY,     0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_ALIEN_GREEN, 2,
    {
      { COL_TURQUOISE, 1, { {0,20,0}, 20.0f } },
      { COL_GREY,      0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_ALIEN_RED, 2,
    {
      { COL_PINKRED, 1, { {0,20,0}, 20.0f } },
      { COL_GREY,    0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_SONDE, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_SATELLITE, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_ROCKET, 1,
    {
      { COL_YELLOW, 0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_ASTRONAUT, 1,
    {
      { COL_WHITE, 0 }
    }
  },

  {
    CLASS_NEUTRAL, NEUTRAL_COMET, 1,
    {
      { COL_BROWN, 0 }
    }
  },

  {
    CLASS_DEBRIS, DEBRIS_BALLOONSEAT, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_DEBRIS, DEBRIS_RACER, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_DEBRIS, DEBRIS_SATELLITE, 1,
    {
      { COL_GREY, 0 }
    }
  },

  {
    CLASS_DEBRIS, DEBRIS_ROCKETWINGS, 1,
    {
      { COL_RED, 0 }
    }
  },

  {
    CLASS_FRIEND, FRIEND_XMAS_SANTA, 2,
    {
      { COL_BROWN, 1, { {0,0,30}, 50.0f } },    // rentiers
      { COL_RED,   0 },                         // santa + sled
    }
  },

  {
    CLASS_FRIEND, FRIEND_XMAS_SANTA_SPECIAL, 2,
    {
      { COL_BROWN, 1, { {0,0,30}, 50.0f } },    // rentiers
      { COL_RED,   0 },                         // santa + sled
    }
  },

  {
    CLASS_NEUTRAL, NEUTRAL_XMAS_TREE, 3,
    {
      { COL_YELLOW,  1, { {0,40,0}, 15.0f } },  // golden star in tree crown
      { COL_BROWN,   1, { {0,-40,0}, 10.0f } }, // brown trunk stub on bottom
      { COL_DKGREEN, 0 }                        // rest of tree
    }
  },

};



//------------------------------------------------
// EC_StartEntity - call startup function (if one exists) for given entity
//                 and provide health
//------------------------------------------------
void EC_StartEntity(gentity_t *gent)
{
  int               i;
  const startinfo_t *si;

  assert(gent && gent->sv);

  for(i = 0, si = s_startinfos; i < NELEMS(s_startinfos); i++, si++)
  {
    //if(!G_Stricmp(s_startinfos[i].entname, gent->sv->className))
    if(si->mainclass == gent->mainclass && si->subclass == gent->subclass)
    {
      gent->health = si->health;
      gent->damage = si->damage;
      si->start(gent);
      break;
    }
  }
} // EC_StartEntity


//------------------------------------------
// FindPainPartEmission
//------------------------------------------
static const pain_partemission_t *FindPainPartEmission(mainclass_t mainclass,
                                                       subclass_t  subclass)
{
  int               i;
  const pain_partemission_t *ppe;

  for(i = 0, ppe = s_pain_partemissions; i < NELEMS(s_pain_partemissions); i++, ppe++)
  {
    if(ppe->mainclass == mainclass && ppe->subclass == subclass)
      return ppe;
  }
  return NULL;
} // FindPainPartEmission


//-----------------------------------------------
// EC_CheckDamageParticles
//-----------------------------------------------
void EC_CheckDamageParticles(const gentity_t *victim, const vec3_t impactpoint, float damage)
{
  const pain_partemission_t *ppe;

  if((ppe = FindPainPartEmission(victim->mainclass, victim->subclass)))
  {
    int             i;
    const vec4_t    *partcolor = NULL;


    // run through the table of emissions; if we find the impact
    // point lying in an emission -region-, take its particle color, otherwise
    // take the color of the general emission
    for(i = 0; i < ppe->numemissions; i++)
    {
      if(ppe->emissions[i].is_region)
      {
        vec3_t  regionpos;  // in world
        vec3_t  dist;       // distance: impact point <-> region in world

        //
        // see if impact point lies inside emission region
        //

      #if 0
        // THIS CODE IS INCORRECT; THE REAL REGION ORIGIN MUST RESIDE
        // IN THE COORDINATE SYSTEM DEFINED BY ENTITIY'S "right", "up" & "dir"
        vec3Add(victim->sv->position, ppe->emissions[i].region.pos, regionpos);
      #else
        int     k;
        vec3_t  victim_right, victim_up;

        // get region origin in world
        BuildAxisByDir(victim_right, victim_up, victim->sv->direction);
        vec3Copy(victim->sv->position, regionpos);
        for(k = 0; k < 3; k++)
        {
          regionpos[k] += victim_right[k]          * ppe->emissions[i].region.pos[0];
          regionpos[k] += victim_up[k]             * ppe->emissions[i].region.pos[1];
          regionpos[k] += victim->sv->direction[k] * ppe->emissions[i].region.pos[2];
        }
      #endif

        vec3Sub(impactpoint, regionpos, dist);
        if(vec3Len(dist) <= ppe->emissions[i].region.radius)
        {
          partcolor = &ppe->emissions[i].partcolor;
          break;    // no need to search for another emission region
        }
      }
      else
      {
        partcolor = &ppe->emissions[i].partcolor;
      }
    }

    // if we found an emisison region or took the general emission
    // emit particles
    if(partcolor != NULL)
      Part_EmitRadially(impactpoint, G_Round(damage * 16), 30.0f, *partcolor);
  }
} // EC_CheckDamageParticles


//-----------------------------------------------
// EC_OnHavingKilled
//-----------------------------------------------
void EC_OnHavingKilled(gentity_t *attacker, const gentity_t *corpse)
{
  switch(attacker->mainclass)
  {
    case CLASS_PLAYER:
      Player_OnHavingKilled(attacker, corpse);
      break;

    default:
      break;    // nothing
  }
} // EC_OnHavingKilled


//-----------------------------------------------
// EC_GetKillScoreInfo - returns NULL if no kill score is assigned to given entity
//-----------------------------------------------
const scoreinfo_t *EC_GetKillScoreInfo(mainclass_t mainclass, subclass_t subclass)
{
  int               i;
  const killscore_t *ks;

  for(i = 0, ks = s_killscores; i < NELEMS(s_killscores); i++, ks++)
  {
    if(ks->mainclass == mainclass && ks->subclass == subclass)
      return &ks->scoreinfo;
  }
  return NULL;
}
