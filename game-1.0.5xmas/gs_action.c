// game state: action

// game code headers
#include "g_local.h"
#include "gs_main.h"
#include "script.h"
#include "client.h"

// engine header
#include "ms.h"


static int  s_levelrunning = 1;


//-----------------------------------
// GS_Action_Enter
//-----------------------------------
void GS_Action_Enter(void)
{
  // if level running, assume resumption from pause state
  if(s_levelrunning)
  {
    // do nothing
  }
  else
  {
    // assume new level started;

    CL_Reset();
    //+++ FIXME: add: catch all ENT_PLAYER entities -> Player_Reset()

    s_levelrunning = 1;
  }
} // GS_Action_Enter


//-----------------------------------
// GS_Action_Leave
//-----------------------------------
void GS_Action_Leave(void)
{
} // GS_Action_Leave


//-----------------------------------
// GS_Action_RunFrame
//-----------------------------------
void GS_Action_RunFrame(level_t *level)
{
  float timeleft;

  assert(level);

  timeleft = level->duration - *e.gametime;

  // if level time is up start fading
  if(timeleft <= 0)
  {
    s_levelrunning = 0;
    GS_SetPendingState(GS_ACTION);  // to start again with action after fade
    GS_ChangeState(GS_FADEOUT);
    return;
  }
  CL_RunFrame(timeleft);
  SC_RunFrame(level);
  EM_RunFrame();
} // GS_Action_RunFrame


//-----------------------------------
// GS_Action_KeyEvent
//-----------------------------------
void GS_Action_KeyEvent(int key, int ispressed)
{
  switch(key)
  {
    case K_ESCAPE:
      GS_ChangeState(GS_PAUSE);
      break;

    default:
      CL_KeyEvent(key, ispressed);
      break;
  }
} // GS_Action_KeyEvent


//-----------------------------------
// GS_Action_MouseEvent
//-----------------------------------
void GS_Action_MouseEvent(msg_t msg, int dx, int dy)
{
  CL_MouseEvent(msg, dx, dy);
} // GS_Action_MouseEvent
