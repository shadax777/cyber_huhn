/*
KITT: "michael, ich hab ein ungewöhnliches gefühl."
MK: "kumpel, du -hast- doch gar keine gefühle."
KITT: "aber michael!..."
*/

// management of game entities

#include <stdarg.h>

#include "g_local.h"



// max. no. of render-only entities, like particles and feathers;
// we need to limit it to leave enough space for true entities
#define MAX_RENTS       (MAX_ENTITIES * 2 / 3)


static int          s_numrenderonlies = 0; // no. of active render-only entities
static gentity_t    s_gentities[MAX_ENTITIES];
static int          s_numgentities = 0; // no. of *non-free* game entities;
                                // the fact that the engine can spawn an
                                // entity is not a sufficient critera that
                                // there are also some *free* entities on the
                                // game side

static float        s_spawngamma = 1.0f; // gamma for freshly-spawned entities


// HACK since the engine shall keep each entity cached
static int  fuck_numprecachedentities;



//----------------------------------------------------------------
// EM_Hack_EntityCachingStopsNow
//
// since the engine preferably keeps each entity
//----------------------------------------------------------------
void EM_Hack_EntityCachingStopsNow(void)
{
  fuck_numprecachedentities = EM_NumEntities();
} // EM_Hack_EntityCachingStopsNow


//----------------------------------------------------------------
// EM_Hack_FreeCachedEntities
//----------------------------------------------------------------
void EM_Hack_FreeCachedEntities(void)
{
  int   i;

  for(i = 0; i < fuck_numprecachedentities; i++)
    EM_Free(s_gentities + i);

  fuck_numprecachedentities = 0;
} // EM_Hack_FreeCachedEntities


//--------------------------------------------------------
// EM_RunFrame - free pending entities, do world clipping
//--------------------------------------------------------
void EM_RunFrame(void)
{
  vec3_t    worldcentre;
  float     worldradius;
  int       i;
  gentity_t *gent;

  vec3Copy(*G_GetWorldCentre(), worldcentre);
  worldradius = G_GetWorldRadius();

  i = fuck_numprecachedentities;
  for(gent = &s_gentities[i]; i < MAX_ENTITIES; i++, gent++)
  {
    if(gent->gstate == ES_PENDINGFREE)
    {
      // e.__pendfree() has already been called by EM_Free()
      gent->gstate = ES_FREE;
      s_numgentities--;
      assert(s_numgentities >= 0);

      // in case the entity wants to do something before getting finally
      // freed
      //
      // NOTE: we don't call it in EM_Free() because EM_Free() only
      //       marks the entity to get freed.
      if(gent->OnFree != NULL)
        gent->OnFree(gent);

      continue;
    }

    // skip some classes from world clipping
    switch(gent->mainclass)
    {
      case CLASS_PLAYER:
      case CLASS_CLIENT:
      case CLASS_ITEM:
      case CLASS_IMAGE:
      case CLASS_TEXT:
      case CLASS_SOUND:
        continue;

      default:  // don't filter
        ;
    }

    // world clipping
    if(gent->gstate == ES_ACTIVE)
    {
      if(!G_EntInsideSphere(gent, worldcentre, worldradius))
      {
        EM_Free(gent);
      }
    }
  }
} // EM_RunFrame


//-------------------------------------
// EM_SetSpawnGamma
//-------------------------------------
void EM_SetSpawnGamma(float gamma)
{
  if(gamma >= 0.0f && gamma <= 1.0f)
    s_spawngamma = gamma;
}


//-------------------------------------
// EM_GetSpawnGamma
//-------------------------------------
float EM_GetSpawnGamma(void)
{
  return s_spawngamma;
}


//--------------------------------------------------
// Spawn - helper for EM_Spawnxxx()
//--------------------------------------------------
static gentity_t *Spawn(const char      *medianame,
                        entitytype_t    sv_etype,
                        objectsubtype_t sv_objtype,
                        mainclass_t     mainclass,
                        subclass_t      subclass,
                        const char      *classname)
{
  entity_t  *svent;
  gentity_t *gent = NULL;


  // limit render-only entities to leave enough space for true entities
  if(mainclass == CLASS_RENDERONLY && s_numrenderonlies >= MAX_RENTS)
    return NULL;

  if(s_numgentities >= MAX_ENTITIES)
  {
    e.conprintf("- Spawn (%s) [time: %f]- already too many gentities (%i)\n", medianame, *e.gametime, s_numgentities);
    return NULL;
  }

  if((svent = e.__spawnentity(medianame, sv_etype)) != NULL)
  {
    int i;

    if(sv_etype == object_e)
    {
      assert(sv_objtype != _invalid_e);
      svent->m.subtype = sv_objtype;
    }
    else
    {
      // only 'object_e' entities may have a subtype
      assert(sv_objtype == _invalid_e);
    }

    // find a free entity slot
    i = 0;
    gent = s_gentities;
    while(i < MAX_ENTITIES && gent->gstate != ES_FREE)
    {
      i++;
      gent++;
    }

    // if this fails, the engine lied about the max. no. of entities
    assert(i < MAX_ENTITIES);

    memset(gent, 0, sizeof(gentity_t));
    gent->sv = svent;
    gent->sv->gentitynum = i;
    gent->sv->gamma = s_spawngamma;
    gent->gstate = ES_ACTIVE;
    gent->mainclass = mainclass;
    gent->subclass = subclass;
    gent->classname = classname;
    gent->medianame = medianame;
    gent->timestamp = *e.gametime;
    gent->health = 1;   // minimal health to avoid immediate death after spawn
                        // FIXME: obsolete due to startup-table ?
    gent->damage = 0;
    if(mainclass == CLASS_RENDERONLY)
    {
      s_numrenderonlies++;
    }
    s_numgentities++;
  }
  return gent;
} // Spawn


//--------------------------------------------------
// EM_SpawnByClassInfo
//--------------------------------------------------
gentity_t *EM_SpawnByClassInfo(const classinfo_t *ci)
{
  assert(ci);
  return Spawn(ci->medianame, ci->sv_etype, ci->sv_objtype, ci->mainclass, ci->subclass, ci->classname);
}


//--------------------------------------------------
// EM_SpawnModel
//--------------------------------------------------
gentity_t *EM_SpawnModel(mainclass_t mainclass, subclass_t subclass)
{
  const classinfo_t *ci;

  ci = G_GetClassInfo(mainclass, subclass);
  G_Assert(ci, G_Stringf("mainclass: %i, subclass: %i\n", mainclass, subclass));
  G_Assert(ci->sv_etype == object_e, G_Stringf("mainclass: %i, subclass: %i\n", mainclass, subclass));
  return EM_SpawnByClassInfo(ci);
} // EM_SpawnModel


//--------------------------------------------------
// EM_SpawnSound - NOTE: does NOT play the sound
//--------------------------------------------------
gentity_t *EM_SpawnSound(const char *filename)
{
  return Spawn(filename, sound_e, _invalid_e, CLASS_SOUND, SOUND_ANYTHING, "class_sound");
}


//--------------------------------------------------
// EM_SpawnImage
//--------------------------------------------------
gentity_t *EM_SpawnImage(subclass_t imageclass)
{
  const classinfo_t *ci;

  ci = G_GetClassInfo(CLASS_IMAGE, imageclass);
  G_Assert(ci, G_Stringf("subclass: %i\n", imageclass));
  G_Assert(ci->sv_etype == image_e, G_Stringf("subclass: %i\n", imageclass));
  return EM_SpawnByClassInfo(ci);
}


//--------------------------------------------------
// EM_SpawnText
//--------------------------------------------------
gentity_t *EM_SpawnText(subclass_t textclass)
{
  const classinfo_t *ci;

  ci = G_GetClassInfo(CLASS_TEXT, textclass);
  G_Assert(ci, G_Stringf("subclass: %i\n", textclass));
  G_Assert(ci->sv_etype == screenmsg_e, G_Stringf("subclass: %i\n", textclass));
  return EM_SpawnByClassInfo(ci);
} // EM_SpawnText


//--------------------------------------------------
// EM_SpawnParticle
//--------------------------------------------------
gentity_t *EM_SpawnParticle(void)
{
  return Spawn("(particle)", particle_e, _invalid_e, CLASS_RENDERONLY, RONLY_PARTICLE, "class_particle");
}


//--------------------------------------------------
// EM_Free - request server to free the server-part of the
//           entity and mark it to get free on game side on next frame
//--------------------------------------------------
void EM_Free(gentity_t *gent)
{
  assert(gent);

  // if already de-activated, do nothing
  if(gent->gstate != ES_ACTIVE)
    return;

  G_Assert(gent->sv, G_Stringf("classname: '%s', mainclass: %i, subclass: %i\n", gent->classname, gent->mainclass, gent->subclass));

  e.__pendremove(gent->sv);

  if(gent->mainclass == CLASS_RENDERONLY)
  {
    assert(s_numrenderonlies > 0);
    s_numrenderonlies--;
  }
  gent->gstate = ES_PENDINGFREE;
} // EM_Free


//--------------------------------------------------
// EM_FreeAll - except the hackly cached ones
//--------------------------------------------------
void EM_FreeAll(void)
{
  int   i;

  for(i = fuck_numprecachedentities; i < MAX_ENTITIES; i++)
  {
    EM_Free(s_gentities + i);
  }
} // EM_FreeAll


//------------------------------------
// G_GEntity - return the game entity referenced by given server entity
//------------------------------------
gentity_t *G_GEntity(entity_t *svent)
{
  assert(svent);

  return s_gentities + svent->gentitynum;
} // G_GEntity


//----------------------------------
// EM_NextActiveEntity
//----------------------------------
gentity_t *EM_NextActiveEntity(gentity_t *last)
{
  int       entnum;
  gentity_t *cur;

  if(!last)
  {
  #if 0
    entnum = 0;
  #else
    // HACK - the cached entities don't count!!
    entnum = fuck_numprecachedentities;
  #endif
  }
  else
  {
    entnum = last - s_gentities + 1;   // successor of given entity
  }

  for(cur = &s_gentities[entnum]; entnum < MAX_ENTITIES; entnum++, cur++)
  {
    if(cur->gstate == ES_ACTIVE)
      return cur;
  }
  return NULL;
} // EM_NextActiveEntity


//-------------------------------------
// EM_NumEntities
//-------------------------------------
int EM_NumEntities(void)
{
  return s_numgentities;
} // EM_NumEntities


//-------------------------------------
// EM_NumRenderEntities
//-------------------------------------
int EM_NumRenderEntities(void)
{
  int               i, n = 0;
  const gentity_t   *gent;

  for(i = fuck_numprecachedentities, gent = s_gentities; i < MAX_ENTITIES; i++, gent++)
  {
    if(gent->mainclass == CLASS_RENDERONLY && gent->gstate != ES_FREE)
      n++;
  }
  return n;
} // EM_NumRenderEntities


void E_Model_SetScale(gentity_t *gent, float scale)
{
  assert(gent);
  G_Assert(gent->sv->type == object_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->m.scale = scale;
}


void E_Model_SetAnimSpeed(gentity_t *gent, float animspeed)
{
  assert(gent);
  G_Assert(gent->sv->type == object_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->m.animSpeed = animspeed;
}


void E_Sound_Play(gentity_t *gent, int looped)
{
  assert(gent);
  G_Assert(gent->sv->type == sound_e, G_Stringf("classname: %s", gent->classname));
  e.sfx_play(gent->sv, looped);
}


void E_Sound_SetVolume(gentity_t *gent, float vol)
{
  assert(gent);
  G_Assert(gent->sv->type == sound_e, G_Stringf("classname: %s", gent->classname));
  e.sfx_setsoundvolume(gent->sv->s.sound, vol);
}


void E_Image_SetDimension(gentity_t *gent, float width, float height)
{
  assert(gent);
  G_Assert(gent->sv->type == image_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->i.xsize = width;
  gent->sv->i.ysize = height;
}


void E_Text_Sprintf(gentity_t *gent, const char *fmt, ...)
{
  va_list   ap;

  assert(gent);
  G_Assert(gent->sv->type == screenmsg_e, G_Stringf("classname: %s", gent->classname));
  va_start(ap, fmt);
  vsprintf(gent->sv->t.message, fmt, ap);   // FIXME: buffer might overflow
  va_end(ap);
}


void E_Text_SetSize(gentity_t *gent, float size)
{
  assert(gent);
  G_Assert(gent->sv->type == screenmsg_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->t.size = size;
}


void E_Text_EnableShadow(gentity_t *gent, int enable)
{
  assert(gent);
  G_Assert(gent->sv->type == screenmsg_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->t.shadow = enable ? true : false;
}


void E_Text_Enable3D(gentity_t *gent, int enable)
{
  assert(gent);
  G_Assert(gent->sv->type == screenmsg_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->t.threeD = enable ? true : false;
}


void E_Particle_SetSize(gentity_t *gent, float size)
{
  assert(gent);
  G_Assert(gent->sv->type == particle_e, G_Stringf("classname: %s", gent->classname));
  gent->sv->p.size = size;
}


//---------------------------------------------------------------------------
// debugging stuff


//------------------------------------------------
// EM_ReportEntities - for debugging
//------------------------------------------------
void EM_ReportEntities(void)
{
  int   i, count = 0;

  for(i = 0; i < MAX_ENTITIES; i++)
  {
    const gentity_t *gent = s_gentities + i;

    if(gent->gstate == ES_ACTIVE)
    {
      count++;
      if(i < fuck_numprecachedentities)
        e.conprintf("[precached] %s: (%p)\n", gent->classname, gent);
      else
        e.conprintf("%s: (%p)\n", gent->classname, gent);
    }
  }
  e.conprintf("------\n %i entities\n", count);
} // EM_ReportEntities
