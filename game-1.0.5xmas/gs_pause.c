// game code headers
#include "g_local.h"
#include "gs_main.h"
#include "script.h"
#include "client.h"
#include "menu.h"

// engine header
#include "ms.h"


static float    s_originalmastervol;
static gsid_t   s_oldgamestate;     // before pause


static void RestoreOldGameState(void)
{
  GS_ChangeState(s_oldgamestate);
}


static void AnyButton_SetFullBright(menuitem_t *self)
{
  MenuItem_SetGamma(self, 1.0f);
}


static void AnyButton_SetHalfBright(menuitem_t *self)
{
  MenuItem_SetGamma(self, 0.5f);
}


// "quit" click event
static void Quit_OnClick(menuitem_t *self)
{
  Game_Shutdown(SHUTDOWN_ABORT);
}


// "continue" click event
static void Continue_OnClick(menuitem_t *self)
{
  RestoreOldGameState();
}


static menu_t       s_pausemenu;


//--------------------------------------------------------------------
// BuildPauseMenu - create "continue" and "quit" buttons and
//                  attach handlers to them
//--------------------------------------------------------------------
static void BuildPauseMenu(void)
{
  static menuitem_t s_continueitem, s_quititem;
  const char        *str_continue, *str_quit;

  // get localized strings
  str_continue = e.lang_getlocalizedstr(STRID_CONTINUE);
  str_quit     = e.lang_getlocalizedstr(STRID_QUIT);

  if(!MenuItem_Create(&s_continueitem, IMAGE_PAUSE_BUTTON, str_continue))
  {
    // MenuItem_Create() printed the error
    Game_Shutdown(SHUTDOWN_FAILURE);
  }

  if(!MenuItem_Create(&s_quititem, IMAGE_PAUSE_BUTTON, str_quit))
  {
    // MenuItem_Create() printed the error
    Game_Shutdown(SHUTDOWN_FAILURE);
  }

  // name buttons (optional) - NOTE: this is NOT the caption seen in the game;
  //                                 just a debugging aid
  s_continueitem.name = "menuitem_continue";
  s_quititem.name     = "menuitem_quit";

  // set touch handlers
  MenuItem_SetOnTouchHandler(&s_continueitem, AnyButton_SetFullBright);
  MenuItem_SetOnTouchHandler(&s_quititem, AnyButton_SetFullBright);

  // set click handlers
  MenuItem_SetOnClickHandler(&s_continueitem, Continue_OnClick);
  MenuItem_SetOnClickHandler(&s_quititem, Quit_OnClick);

  // set idle handlers
  MenuItem_SetIdleHandler(&s_continueitem, AnyButton_SetHalfBright);
  MenuItem_SetIdleHandler(&s_quititem, AnyButton_SetHalfBright);

  // resize bounding quads of both buttons
  // (since the image is larger than the buttons appear)
  MenuItem_ResizeBQuad(&s_continueitem, 0, 0, -42, 42);
  MenuItem_ResizeBQuad(&s_quititem,     0, 0, -42, 42);

  // create pause menu framework
  Menu_Create(&s_pausemenu, VW / 2, VH / 2);

  // register buttons in pause menu
  Menu_AddItem(&s_pausemenu, &s_continueitem, -80, 0);
  Menu_AddItem(&s_pausemenu, &s_quititem,      80, 0);
} // BuildPauseMenu


//-------------------------------------------------------
// TrashPauseMenu
//-------------------------------------------------------
static void TrashPauseMenu(void)
{
  Menu_Destroy(&s_pausemenu);
} // TrashPauseMenu


//---------------------------------------------------------------------------


// max. darkening value done by pause code
#define MAX_WORLD_GAMMA 0.3f


// to backup gammas of all entities
typedef struct
{
  gentity_t *gent;
  float     gamma_backup;
} gent_gamma_t;


static gent_gamma_t s_entgammas[MAX_ENTITIES];
static int          s_numdarkened;

// !! make sure, no entitiy gets freed after this function has been called !!
// !! dangling pointers will eventually occur and data will be written to
//    them when restoring gammas of all entities!!
// spawning new entities is OK, though.
static void DarkenEntities(void)
{
  gentity_t *gent = NULL;

  s_numdarkened = 0;
  while((gent = EM_NextActiveEntity(gent)))
  {
    // DEBUG; I thought here was a bug, but seems it was not :)
    assert(gent->sv);

    s_entgammas[s_numdarkened].gent = gent;
    s_entgammas[s_numdarkened].gamma_backup = gent->sv->gamma;
    if(gent->sv->gamma > MAX_WORLD_GAMMA)
      gent->sv->gamma = MAX_WORLD_GAMMA;
    s_numdarkened++;
  }
}


// restore original gammas of entities
static void RestoreEntityGammas(void)
{
  gent_gamma_t  *cur;

  while(s_numdarkened > 0)
  {
    cur = &s_entgammas[--s_numdarkened];
    cur->gent->sv->gamma = cur->gamma_backup;
  }
}


//---------------------------------------------------------------------------


//-------------------------------------------------------------
// GS_Pause_Enter
//-------------------------------------------------------------
void GS_Pause_Enter(void)
{
  // backup game state before pause
  // note: we're NOT yet in the pause-state, but after having left
  //       this function the gamestate manager will set the pause state
  //       so, what we get here is the state where we came from
  s_oldgamestate = GS_GetCurrentState();

  e.gametime_pause();
  //e.sfx_pauseall();   // FIXME: pause in sound engine is broken
  CL_Pause();

  // reduce master volume
  s_originalmastervol = G_GetMasterVolume();
  if(s_originalmastervol > 0.3f)
    G_SetMasterVolume(0.3f);

  DarkenEntities();
  BuildPauseMenu();
  Menu_EnableRender(&s_pausemenu, 1);
} // GS_Pause_Enter


//-------------------------------------------------------------
// GS_Pause_Leave
//-------------------------------------------------------------
void GS_Pause_Leave(void)
{
  e.gametime_resume();
  //e.sfx_resumeall();

  RestoreEntityGammas();
  Menu_EnableRender(&s_pausemenu, 0);   // OBSOLETE??
  TrashPauseMenu();

  CL_Resume();
  G_SetMasterVolume(s_originalmastervol);
} // GS_Pause_Leave


//-------------------------------------------------------------
// GS_Pause_RunFrame
//-------------------------------------------------------------
void GS_Pause_RunFrame(level_t *level)
{
  Menu_RunFrame(&s_pausemenu);
} // GS_Pause_RunFrame


//-------------------------------------------------------------
// GS_Pause_KeyEvent
//-------------------------------------------------------------
void GS_Pause_KeyEvent(int key, int ispressed)
{
  switch(key)
  {
    case K_ESCAPE:
      RestoreOldGameState();
      break;
  }
} // GS_Pause_KeyEvent


//-------------------------------------------------------------
// GS_Pause_MouseEvent
//-------------------------------------------------------------
void GS_Pause_MouseEvent(msg_t msg, int dx, int dy)
{
  switch(msg)
  {
    case MSG_MOUSEMOVE:
      Menu_HandleMouseMovement(&s_pausemenu, dx, dy);
      break;

    case MSG_LBUTTONDOWN:
      Menu_HandleMouseClick(&s_pausemenu);
      break;

    default:    // shut up compiler
      break;
  }
} // GS_Pause_MouseEvent
