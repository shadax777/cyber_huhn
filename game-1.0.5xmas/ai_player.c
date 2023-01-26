#include "g_local.h"
#include "client.h"


#define MAX_ANGLE   85

#define SHAKE_PERIOD        0.025f


static void UpdateAngle(gentity_t *gself, int axisnum, float angdiff);
static void ShakeSetDir(gentity_t *gself);
static void ShakeRunFrame(entity_t *svself);    // think()
static void ShakeStop(gentity_t *gself);
static void OnReceiveItem(gentity_t *gself, const gentity_t *item);
static void OnItemExpires(gentity_t *gself, const gentity_t *item);


//-------------------------------------------
// Player_Start
//-------------------------------------------
void Player_Start(gentity_t *gself)
{
  e.conprintf("* Player_Start\n");
  gself->OnReceiveItem = OnReceiveItem;
  gself->OnItemExpires = OnItemExpires;
} // Player_Start


//-------------------------------------------
// Player_Shake - start shaking the player (e.g. when a bombchicken hits him)
//-------------------------------------------
void Player_Shake(gentity_t *gself, float duration, float maxamplitude)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));

  gself->n.player.shaker.starttime = *e.gametime;
  gself->n.player.shaker.duration = duration;
  gself->n.player.shaker.maxamplitude = maxamplitude;
  ShakeSetDir(gself);

  if(!Player_IsShaking(gself))
    gself->n.player.shaker.oldthink = gself->sv->think;

  gself->sv->think = ShakeRunFrame;
  gself->n.player.shaker.nextupdatetime = *e.gametime + SHAKE_PERIOD;
} // Player_Shake


//-------------------------------------------
// Player_IsShaking
//-------------------------------------------
int Player_IsShaking(const gentity_t *gself)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));

  return gself->sv->think == ShakeRunFrame; // WTF ?!?
} // Player_IsShaking


//-------------------------------------------
// Player_Yaw
//-------------------------------------------
void Player_Yaw(gentity_t *gself, float ang)
{
  UpdateAngle(gself, 1, ang);
}


//-------------------------------------------
// Player_Pitch
//-------------------------------------------
void Player_Pitch(gentity_t *gself, float ang)
{
  UpdateAngle(gself, 0, ang);
}


//-------------------------------------------
// Player_Finish
//-------------------------------------------
void Player_Finish(gentity_t *gself)
{
  if(Player_IsShaking(gself))
    ShakeStop(gself);
  gself->sv->think = NULL;
}


//-------------------------------------------
// Player_Reset
//-------------------------------------------
void Player_Reset(gentity_t *gself)
{
  if(Player_IsShaking(gself))
    ShakeStop(gself);
  gself->n.player.score = 0;
}


//-------------------------------------------
// Player_OnHavingKilled - update score + check if bonus weapon can be given
//-------------------------------------------
void Player_OnHavingKilled(gentity_t *gself, const gentity_t *corpse)
{
  const scoreinfo_t *si;

  if((si = EC_GetKillScoreInfo(corpse->mainclass, corpse->subclass)))
  {
    if((gself->n.player.score += si->value) < 0)
      gself->n.player.score = 0;
  }

  // if player has not yet the bonus weapon, track the no. of consecutive
  // chicken killings and see if we hit the limit
  if(!E_HasItem(gself, ITEM_BONUSWEAPON))
  {
    // track consecutive chicken killings
    if(corpse->mainclass == CLASS_ENEMY
    && (corpse->subclass == ENEMY_CHICKEN || corpse->subclass == ENEMY_XMAS_CHICKEN))
    {
      if(++gself->n.player.num_consecutive_chickenkills == 10)
      {
        // bonus weapon!
        I_GiveItem(gself, ITEM_BONUSWEAPON);

        // reset chicken killing statistics
        gself->n.player.num_consecutive_chickenkills = 0;
      }
    }
    else
    {
      // we hit anything else than a chicken
      gself->n.player.num_consecutive_chickenkills = 0;
    }
  }
  CL_OnPlayerHasKilled(gself->n.player.client, corpse);
} // Player_OnHavingKilled


//---------------------------------------------------------------------------


//-------------------------------------------
// UpdateAngle - helper for Player_Yaw() and Player_Pitch()
//-------------------------------------------
static void UpdateAngle(gentity_t *gself, int axisnum, float angdiff)
{
  vec3_t    up;

  assert(axisnum == 0
      || axisnum == 1
      || axisnum == 2);
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));

  gself->sv->angles[axisnum] += angdiff;
  if(gself->sv->angles[axisnum] > MAX_ANGLE)
  {
    gself->sv->angles[axisnum] = MAX_ANGLE;
  }
  else if(gself->sv->angles[axisnum] < -MAX_ANGLE)
  {
    gself->sv->angles[axisnum] = -MAX_ANGLE;
  }
  BuildAxisByAngles(gself->sv->angles[0], gself->sv->angles[1], gself->sv->angles[2],
                    gself->sv->right, up, gself->sv->direction);
} // UpdateAngle


//-------------------------------------
// ShakeSetDir - set new shake direction + next update time
//-------------------------------------
static void ShakeSetDir(gentity_t *gself)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));

  gself->n.player.shaker.pitch = G_Random() * (2 * gself->n.player.shaker.maxamplitude) - gself->n.player.shaker.maxamplitude;
  gself->n.player.shaker.yaw =   G_Random() * (2 * gself->n.player.shaker.maxamplitude) - gself->n.player.shaker.maxamplitude;
} // ShakeSetDir


//-----------------------------------------------------------------
// ShakeRunFrame - think() function:
//                 move player's "dir" along shake's "dir",
//                 check if shake update is needed
//-----------------------------------------------------------------
static void ShakeRunFrame(entity_t *svself)
{
  gentity_t *gself;
  float     timediff = *e.gametime - *e.lastgametime;

  assert(svself);
  gself = G_GEntity(svself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));

  // check if shake is totally over
  if(*e.gametime >= gself->n.player.shaker.starttime + gself->n.player.shaker.duration)
  {
    //gself->sv->think = gself->n.player.shaker.oldthink;
    ShakeStop(gself);
    return;
  }

  // we must FIRST shake the player, and THEN check if it's time to set new
  // shake direction because slow machines might exceed the update period and
  // change direction without having shaked even once.

  Player_Pitch(gself, gself->n.player.shaker.pitch * timediff);
  Player_Yaw(gself, gself->n.player.shaker.yaw * timediff);

  if(*e.gametime >= gself->n.player.shaker.nextupdatetime)
  {
    ShakeSetDir(gself);
    gself->n.player.shaker.nextupdatetime = *e.gametime + SHAKE_PERIOD;
  }
} // ShakeRunFrame


//-------------------------------------
// ShakeStop
//-------------------------------------
static void ShakeStop(gentity_t *gself)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%i ('%s')", gself->mainclass, gself->classname));
  assert(Player_IsShaking(gself));

  gself->sv->think = gself->n.player.shaker.oldthink;
} // ShakeStop


//-------------------------------------
// OnReceiveItem
//-------------------------------------
static void OnReceiveItem(gentity_t *gself, const gentity_t *item)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%s", E_GetString(gself)));

//  e.conprintf("* player.c::OnReceiveItem\n");
  CL_OnPlayerReceivesItem(gself->n.player.client, item);
}


//-------------------------------------
// OnItemExpires
//-------------------------------------
static void OnItemExpires(gentity_t *gself, const gentity_t *item)
{
  assert(gself);
  G_Assert(gself->mainclass == CLASS_PLAYER, G_Stringf("%s", E_GetString(gself)));

//  e.conprintf("* player.c::OnItemExpires\n");
  CL_OnPlayerItemExpires(gself->n.player.client, item);
}
