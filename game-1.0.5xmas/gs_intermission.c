
// game code headers
#include "g_local.h"
#include "gs_main.h"
#include "script.h"
#include "client.h"

// engine header
#include "ms.h"


static float    s_entertime;


static gentity_t    *s_intermissiontext;


void GS_Intermission_Enter(void)
{
/*
  s_entertime = *e.gametime;

  // attempt to spawn persistent "Intermission..." text
  assert(!s_intermissiontext);
  if((s_intermissiontext = EM_SpawnText(TEXT_TECH)))
  {
    s_intermissiontext->sv->position[0] = 400;
    s_intermissiontext->sv->position[1] = 300;
    E_Text_SetSize(s_intermissiontext, 50);
    E_Text_EnableShadow(s_intermissiontext, 1);
    E_Text_Sprintf(s_intermissiontext, "Intermission");
  }
*/
}


void GS_Intermission_Leave(void)
{
/*
  if(s_intermissiontext)
  {
    EM_Free(s_intermissiontext);
    s_intermissiontext = NULL;
  }
*/
}


// display intermission screen for 2 seconds, then start next level
void GS_Intermission_RunFrame(level_t *level)
{
/*
  if(*e.gametime - s_entertime >= 2)
  {
    GS_ChangeState(GS_ACTION);
    return;
  }
*/
}


void GS_Intermission_KeyEvent(int key, int ispressed)
{
}


void GS_Intermission_MouseEvent(msg_t msg, int dx, int dy)
{
}
