// gs_main.h - functions for all game states

#include "g_local.h"
#include "gs_main.h"
#include "script.h"


void (*GS_RunFrame)(level_t *level);
void (*GS_KeyEvent)(int key, int ispressed);
void (*GS_MouseEvent)(msg_t msg, int dx, int dy);


// gs_titlescreen.c
void GS_Titlescreen_Enter(void);
void GS_Titlescreen_Leave(void);
void GS_Titlescreen_RunFrame(level_t *level);
void GS_Titlescreen_KeyEvent(int key, int ispressed);
void GS_Titlescreen_MouseEvent(msg_t msg, int dx, int dy);

// gs_action.c
void GS_Action_Enter(void);
void GS_Action_Leave(void);
void GS_Action_RunFrame(level_t *level);
void GS_Action_KeyEvent(int key, int ispressed);
void GS_Action_MouseEvent(msg_t msg, int dx, int dy);

// gs_fade.c
void GS_Fade_Enter(void);
void GS_Fade_Leave(void);
void GS_Fade_RunFrame(level_t *level);
void GS_Fade_KeyEvent(int key, int ispressed);
void GS_Fade_MouseEvent(msg_t msg, int dx, int dy);

// gs_intermission.c
void GS_Intermission_Enter(void);
void GS_Intermission_Leave(void);
void GS_Intermission_RunFrame(level_t *level);
void GS_Intermission_KeyEvent(int key, int ispressed);
void GS_Intermission_MouseEvent(msg_t msg, int dx, int dy);

// gs_pause.c
void GS_Pause_Enter(void);
void GS_Pause_Leave(void);
void GS_Pause_RunFrame(level_t *level);
void GS_Pause_KeyEvent(int key, int ispressed);
void GS_Pause_MouseEvent(msg_t msg, int dx, int dy);

// gs_gameover.c
void GS_Gameover_Enter(void);
void GS_Gameover_Leave(void);
void GS_Gameover_RunFrame(level_t *level);
void GS_Gameover_KeyEvent(int key, int ispressed);
void GS_Gameover_MouseEvent(msg_t msg, int dx, int dy);


//---------------------------------------------------------------------------


typedef struct
{
  gsid_t        id;
  const char    *desc;  // description (for debugging)
  void          (*enter)(void);
  void          (*leave)(void);
  void          (*runframe)(level_t *level);
  void          (*keyevent)(int key, int ispressed);
  void          (*mouseevent)(msg_t msg, int dx, int dy);
} gamestate_t;


static gsid_t       s_pendingstateid;
static gamestate_t  *s_curgamestate;
static gamestate_t  s_gamestates[] =
{
  {
    GS_TITLESCREEN,
    "titlescreen",
    GS_Titlescreen_Enter,
    GS_Titlescreen_Leave,
    GS_Titlescreen_RunFrame,
    GS_Titlescreen_KeyEvent,
    GS_Titlescreen_MouseEvent
  },
  {
    GS_ACTION,
    "action",
    GS_Action_Enter,
    GS_Action_Leave,
    GS_Action_RunFrame,
    GS_Action_KeyEvent,
    GS_Action_MouseEvent
  },
  {
    GS_FADEIN,
    "fadein",
    GS_Fade_Enter,
    GS_Fade_Leave,
    GS_Fade_RunFrame,
    GS_Fade_KeyEvent,
    GS_Fade_MouseEvent
  },
  {
    GS_FADEOUT,
    "fadeout",
    GS_Fade_Enter,
    GS_Fade_Leave,
    GS_Fade_RunFrame,
    GS_Fade_KeyEvent,
    GS_Fade_MouseEvent
  },
  {
    GS_INTERMISSION,
    "intermission",
    GS_Intermission_Enter,
    GS_Intermission_Leave,
    GS_Intermission_RunFrame,
    GS_Intermission_KeyEvent,
    GS_Intermission_MouseEvent
  },
  {
    GS_PAUSE,
    "pause",
    GS_Pause_Enter,
    GS_Pause_Leave,
    GS_Pause_RunFrame,
    GS_Pause_KeyEvent,
    GS_Pause_MouseEvent
  },
  {
    GS_GAMEOVER,
    "gameover",
    GS_Gameover_Enter,
    GS_Gameover_Leave,
    GS_Gameover_RunFrame,
    GS_Gameover_KeyEvent,
    GS_Gameover_MouseEvent
  },
};


//-------------------------------------
// GS_Init
//-------------------------------------
void GS_Init(void)
{
  s_pendingstateid = GS_INVALID;
  GS_ChangeState(GS_TITLESCREEN);
}


//-------------------------------------
// GS_GetCurrentState
//-------------------------------------
gsid_t GS_GetCurrentState(void)
{
  return s_curgamestate->id;
}


//-------------------------------------
// GS_GetPendingState
//-------------------------------------
gsid_t GS_GetPendingState(void)
{
  return s_pendingstateid;
}


//-------------------------------------
// GS_SetPendingState
//-------------------------------------
void GS_SetPendingState(gsid_t id)
{
  s_pendingstateid = id;
}


//-------------------------------------
// GS_ChangeState
//-------------------------------------
void GS_ChangeState(gsid_t id)
{
  int   i;

  // run through all game states and stop at the desired one
  for(i = 0; i < NELEMS(s_gamestates); i++)
  {
    if(id == s_gamestates[i].id)
    {
      // leave old state
      if(s_curgamestate)
        (*s_curgamestate->leave)();

      GS_RunFrame = s_gamestates[i].runframe;
      GS_KeyEvent = s_gamestates[i].keyevent;
      GS_MouseEvent = s_gamestates[i].mouseevent;

      // enter new state
      (*s_gamestates[i].enter)();
      s_curgamestate = &s_gamestates[i];
      break;
    }
  }
  G_Assert((i < NELEMS(s_gamestates)), G_Stringf("invalid state requested: %i\n", id));
} // GS_ChangeState


//-------------------------------------------------------------------
// GS_DebugPrintCurrentState
//-------------------------------------------------------------------
void GS_DebugPrintCurrentState(void)
{
  e.conprintf("current game state: %i = %s\n", s_curgamestate->id, s_curgamestate->desc);
}

