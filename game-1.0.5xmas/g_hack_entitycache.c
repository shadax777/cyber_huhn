
//---------------------------------------------------------------------------
// hack to keep the engine's entity cache always filled

#include <stdlib.h>

#include "g_local.h"
#include "g_shared.h"
#include "script.h"


static gentity_t    **s_entcache;
static int          s_entcachesize;


//
// g_entmanage.c
//
void EM_Hack_EntityCachingStopsNow(void);



static int _IsCached(const char *classname)
{
  int   i;

  for(i = 0; i < s_entcachesize; i++)
  {
    //if(!G_Stricmp(s_entcache[i]->sv->className, classname))
    if(!G_Stricmp(s_entcache[i]->classname, classname))
    {
      return 1;
    }
  }
  return 0;
}


int G_Hack_CacheClasses(const level_t *level)
{
  int               classnum;
  const classinfo_t *info;
  const scentity_t  *scent;

  s_entcache = NULL;
  s_entcachesize = 0;

  // cache all non-scriptable entities (like feather, shot, etc.)
  e.conprintf("* caching non-scriptable entity classes...\n");
  classnum = 0;
  while((info = G_Hack_GetClassInfoByIndex(classnum++)))
  {
    if(!info->is_scriptable)
    {
      gentity_t **newcache, *gent;

      if(!(newcache = realloc(s_entcache, (s_entcachesize + 1) * sizeof(gentity_t *))))
      {
        e.conprintf("_CacheClasses: out of mem at entity #%i\n", s_entcachesize + 1);
        return 0;
      }
      else
      {
        if((gent = EM_SpawnByClassInfo(info)))
        {
          gent->sv->state = passive_e;
          newcache[s_entcachesize++] = gent;
        }
        else
        {
          e.conprintf("# _CacheClasses: could not spawn '%s'\n", info->classname);
          return 0;
        }
      }
      s_entcache = newcache;
    }
  }

  // cache all entities in the given level
  e.conprintf("* caching level entity classes...\n");
  for(scent = level->spawnring.next; scent != &level->spawnring; scent = scent->next)
  {
    info = G_GetClassInfoByName(scent->name);
    assert(info);

  L_relational_entity_spawn:

    if(!_IsCached(info->classname))
    {
      gentity_t **newcache, *gent;

      if(!(newcache = realloc(s_entcache, (s_entcachesize + 1) * sizeof(gentity_t *))))
      {
        e.conprintf("_CacheClasses: out of mem at entity #%i\n", s_entcachesize + 1);
        return 0;
      }
      else
      {
        if((gent = EM_SpawnByClassInfo(info)))
        {
          gent->sv->state = passive_e;
          newcache[s_entcachesize++] = gent;
        }
        else
        {
          e.conprintf("# _CacheClasses: could not spawn '%s'\n", info->classname);
          return 0;
        }
      }
      s_entcache = newcache;
    }

    // some entities may spawn other entities which may not be scripted;
    // so, we must cache them as well
    switch(info->subclass)
    {
      case ENEMY_ENEMYSHIP:
        info = G_GetClassInfo(CLASS_ENEMY, ENEMY_BOMBCHICKEN);
        assert(info);
        goto L_relational_entity_spawn;
        break;

      case ENEMY_MOTHERCHICKEN:
        info = G_GetClassInfo(CLASS_ENEMY, ENEMY_EGG);
        assert(info);
        goto L_relational_entity_spawn;
        break;

      case ENEMY_EGG:
        info = G_GetClassInfo(CLASS_ENEMY, ENEMY_CHICKEN);
        assert(info);
        goto L_relational_entity_spawn;
        break;

      default:
        break;
    }
  }

  // tell the game code that no more entities will be cached for the engine
  EM_Hack_EntityCachingStopsNow();

  e.conprintf("OK\n");

  return 1;
}


void G_Hack_FreeCachedClasses(void)
{
  // explicitely removing the cached entities is not allowed because they are
  // persistent and G_MarkRemove_AllPersistents will do this

  free(s_entcache);
  s_entcache = NULL;
  s_entcachesize = 0;
}


// hack ends
//---------------------------------------------------------------------------
