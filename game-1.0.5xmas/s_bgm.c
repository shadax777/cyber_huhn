#include "g_local.h"


static gentity_t    *s_bgm = NULL;


//--------------------------------
// BGM_Play
//--------------------------------
void BGM_Play(const char *filename)
{
  BGM_Stop();
  s_bgm = SoundLoop(ZEROVEC, ZEROVEC, 0.0, filename);
}


//--------------------------------
// BGM_Stop
//--------------------------------
void BGM_Stop(void)
{
  if(s_bgm)
  {
    EM_Free(s_bgm);
    s_bgm = NULL;
  }
}


//--------------------------------
// BGM_SetVolume
//--------------------------------
void BGM_SetVolume(float vol)
{
  if(!s_bgm)
    return;

  if(vol < 0 || vol > 1.0)
    return;

  E_Sound_SetVolume(s_bgm, vol);
}
