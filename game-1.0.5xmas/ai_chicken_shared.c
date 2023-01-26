#include "g_local.h"


#define MIN_CACKLE_PERIOD   10.0f
#define MAX_CACKLE_PERIOD   20.0f


//--------------------------------------------------------
// xxxChicken_RandomlyCackle - think()
//--------------------------------------------------------
void xxxChicken_RandomlyCackle(entity_t *svself)
{
  const char    *soundname;
  float         cackleperiod;

  soundname = G_Random() > 0.5f ? S_CHICKEN_HERO1 : S_CHICKEN_HERO2;
  SOUND(svself->position, soundname);

  cackleperiod = G_Random() * (MAX_CACKLE_PERIOD - MIN_CACKLE_PERIOD);
  svself->nextthink = *e.gametime + MIN_CACKLE_PERIOD + cackleperiod;
} // xxxChicken_RandomlyCackle


//--------------------------------------------------------
// xxxChicken_PlayRandomPainSound
//--------------------------------------------------------
void xxxChicken_PlayRandomPainSound(gentity_t *gself)
{
  const char    *painsound;

  // HACK: we must regulate pain sounds to not play too many in too short time
  if(*e.gametime - gself->lastpainsoundtime < 0.35f)
    return;

  painsound = G_Random() > 0.5f ? S_CHICKEN_PAIN1 : S_CHICKEN_PAIN2;
  SOUND(gself->sv->position, painsound);
  gself->lastpainsoundtime = *e.gametime;
}
