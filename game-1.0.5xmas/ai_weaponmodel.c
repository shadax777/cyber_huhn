// recoil of weapon model due to shots

#include "g_local.h"


// weapon model state
enum
{
  WMS_NONE,
  WMS_RECOILING,
  WMS_MOVINGBACK
};


#define MAX_MOVEBACK_SPEED  1.5f    // when moving back to orig. pos


// think(): wait until reached max. distance, then slowly move back
static void ProceedRecoil(entity_t *svself);


//----------------------------------
// Weaponmodel_Recoil
//----------------------------------
void Weaponmodel_Recoil(gentity_t *gself, float maxdist, float recoilspeed)
{
  // if freshly recoiling, backup original pos.
  if(gself->n.weaponmodel.state == WMS_NONE)
  {
    vec3Copy(gself->sv->position, gself->n.weaponmodel.origpos);
  }
  gself->n.weaponmodel.state = WMS_RECOILING;
  gself->n.weaponmodel.maxdist = maxdist;
  gself->sv->speed -= recoilspeed;
  gself->sv->think = ProceedRecoil;
} // Weaponmodel_Recoil


// ProceedRecoil - wait until reached max. distance, then slowly move back
static void ProceedRecoil(entity_t *svself)
{
  float     timediff = *e.gametime - *e.lastgametime;
  gentity_t *gself;

  gself = G_GEntity(svself);

  if(gself->n.weaponmodel.state == WMS_RECOILING)
  {
    vec3_t  dist;

    // if reached max. distance slow down and move back
    vec3Sub(gself->sv->position, gself->n.weaponmodel.origpos, dist);
    if(vec3Len(dist) >= gself->n.weaponmodel.maxdist)
    {
      vec3MA(gself->n.weaponmodel.origpos, gself->sv->direction,
             -gself->n.weaponmodel.maxdist, gself->sv->position);
      gself->n.weaponmodel.state = WMS_MOVINGBACK;
      gself->sv->speed = 0.0f;
    }
  }
  else
  {
    vec3_t  olddir, curdir, sumdir;

    // if reached orig. pos or even further, stop;
    // otherwise speed up moving back


    // create vector "old pos" <- "original pos" and normalize
    vec3Sub(gself->sv->oldPosition, gself->n.weaponmodel.origpos, olddir);
    vec3Normalize(olddir);

    // create vector "current pos" <- "original pos" and normalize
    vec3Sub(gself->sv->position, gself->n.weaponmodel.origpos, curdir);
    vec3Normalize(curdir);

    // add both vectors to find out if they're pointing in different directions
    vec3Add(olddir, curdir, sumdir);
    if(vec3Len(sumdir) <= 1.0f)
    {
      // stop moving
      vec3Copy(gself->n.weaponmodel.origpos, gself->sv->position);
      gself->sv->speed = 0.0f;
      gself->sv->think = NULL;
    }
    else
    {
      // speed up moving back
      if((gself->sv->speed += timediff * 200) > MAX_MOVEBACK_SPEED)
      {
        gself->sv->speed = MAX_MOVEBACK_SPEED;
      }
    }
  }
} // ProceedRecoil
