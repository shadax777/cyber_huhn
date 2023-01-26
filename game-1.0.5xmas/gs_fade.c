// game state: fade in/out
//
// Note: whether we're fading in or out is defined by GS_FADEIN or GS_FADEOUT


// game code headers
#include "g_local.h"
#include "gs_main.h"
#include "script.h"
#include "client.h"

// engine header
#include "ms.h"


static float    s_fadevalue;    // [0..1]


static void SetEntitiesGamma(float gamma)
{
  gentity_t *cur = NULL;

  G_Assert((gamma >= 0.0f && gamma <= 1.0f), G_Stringf("%f", gamma));

  while((cur = EM_NextActiveEntity(cur)))
  {
    cur->sv->gamma = gamma;
  }
}


//-----------------------------------
// GS_Fade_Enter
//-----------------------------------
void GS_Fade_Enter(void)
{
  s_fadevalue = EM_GetSpawnGamma();

  // we need a valid state when leaving the fade state
  assert(GS_GetPendingState() != GS_INVALID);
} // GS_Fade_Enter


//-----------------------------------
// GS_Fade_Leave
//-----------------------------------
void GS_Fade_Leave(void)
{
  CL_Finish();
  EM_FreeAll();
} // GS_Fade_Leave


//-----------------------------------
// GS_Fade_RunFrame
//-----------------------------------
void GS_Fade_RunFrame(level_t *level)
{
  float timediff = *e.gametime - *e.lastgametime;
  float fadedir;
  int   mustchange = 0;

  fadedir = (GS_GetCurrentState() == GS_FADEIN) ? 1.0f : -1.0f;
  s_fadevalue += fadedir * timediff * 0.3f;

  // check if fade in either direction is over
  if(s_fadevalue < 0.0f || s_fadevalue > 1.0f)
  {
    // clamp gamma value into valid range
    s_fadevalue = (fadedir < 0) ? 0.0f : 1.0f;
    mustchange = 1;

    // HACK: we quit the game
    Game_Shutdown(SHUTDOWN_FINISH);
  }
  EM_SetSpawnGamma(s_fadevalue);
  SetEntitiesGamma(s_fadevalue);

  if(G_GetMasterVolume() > s_fadevalue)
    G_SetMasterVolume(s_fadevalue);

  EM_RunFrame();
  CL_RunFrame(0.0f);

  if(mustchange)
  {
    gsid_t  nextstate = GS_GetPendingState();

    // a valid state must be provided by previous state
    assert(nextstate != GS_INVALID);
    GS_SetPendingState(GS_INVALID); // clear pending state
    GS_ChangeState(nextstate);
    return;
  }
} // GS_Fade_RunFrame


//-----------------------------------
// GS_Fade_KeyEvent
//-----------------------------------
void GS_Fade_KeyEvent(int key, int ispressed)
{
  // still allow player control
  CL_KeyEvent(key, ispressed);
} // GS_Fade_KeyEvent


//-----------------------------------
// GS_Fade_MouseEvent
//-----------------------------------
void GS_Fade_MouseEvent(msg_t msg, int dx, int dy)
{
  // still allow player control
  CL_MouseEvent(msg, dx, dy);
} // GS_Fade_MouseEvent
