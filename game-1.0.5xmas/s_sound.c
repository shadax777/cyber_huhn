#include "g_local.h"


static gentity_t *SoundBackend(const vec3_t pos,
                               const vec3_t dir,
                               float speed,
                               const char *filename,
                               int looped);

static void OnSvFree(entity_t *svself); // only for loose sounds


//---------------------------------
// Sound
//---------------------------------
void Sound(const vec3_t pos, const vec3_t dir, float speed, const char *filename)

{
  gentity_t *gsoundent;

  if((gsoundent = SoundBackend(pos, dir, speed, filename, 0)))
  {
    // loose sounds can't free themselves on game side
    gsoundent->sv->onSrvFree = OnSvFree;
  }
}


//---------------------------------
// SoundLoop - NOTE: we return the sound-entity, since the caller must
//                   free it again; otherwise the looped sound would be
//                   lost in space...
//---------------------------------
gentity_t *SoundLoop(const vec3_t pos, const vec3_t dir, float speed, const char *fn)
{
  // NOTE: the caller must care to free the sound entity on game side
  return SoundBackend(pos, dir, speed, fn, 1);
}


//---------------------------------------------------------------------------


static gentity_t *SoundBackend(const vec3_t pos,
                               const vec3_t dir,
                               float speed,
                               const char *filename,
                               int looped)
{
  gentity_t *soundent;

  if((soundent = EM_SpawnSound(filename)) != NULL)
  {
    vec3Copy(pos, soundent->sv->position);
    vec3Copy(dir, soundent->sv->direction);
    soundent->sv->speed = speed;
    E_Sound_Play(soundent, looped);
  }
  else
  {
    e.conprintf("# SoundBackend: could not spawn '%s'\n", filename);
  }
  return soundent;
}


// only for loose sound entities; loop sounds must be freed by caller
static void OnSvFree(entity_t *svself)
{
#if 0
  EM_Free(G_GEntity(svself));
#else
  gentity_t *gself = G_GEntity(svself);

  //if(G_EntIsActive(gself))
  if(gself->gstate == ES_ACTIVE)
    EM_Free(gself);
#endif
}
