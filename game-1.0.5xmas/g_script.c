// g_script.c - executable module; used by game code to load + execute
//              compiled script code

#ifdef SC_GAME  // as opposed to SC_COMPILER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include <windows.h>    // MessageBox() for failed CRC computation


#include "g_local.h"
#include "script.h"

#include "math3d.h"



static void InitLists(level_t *l);  // reset a list's spawnring and trashlist

static void         Insert(scentity_t *ring, scentity_t *ent);
static scentity_t   *Extract(scentity_t *ent);

// think(): fade in; if fully bright call EC_StartEntity()
static void FadeIn(entity_t *svself);


// log malloc()- and free()-calls for diagnostical reason in test environment
static int  s_nmalloc = 0, s_nfree = 0;
static void *_shuttle_;
#define MALLOC(n)       (s_nmalloc+=((_shuttle_=malloc(n))!=NULL), _shuttle_)
#define REALLOC(p, n)   (s_nmalloc+=((_shuttle_=(p))==NULL), realloc(_shuttle_, n))
#define FREE(p)         (s_nfree+=((_shuttle_=(p))!=NULL), free(_shuttle_))


//---------------------------------------------------------
// SC_LoadLevel - load level info + all entities; build spawn list
//---------------------------------------------------------
int SC_LoadLevel(level_t *l, const char *inpath)
{
  FILE          *f;
  int           c;  // test if EOF
  int           nents = 0;  // no. of loaded entities (just for info)
  char          infilename[CRCLEN + 1], crcfilename[CRCLEN + 1];
  const char    *cc;
  char          msg[256];   // general purpose

  InitLists(l);

  if(!(f = fopen(inpath, "rb")))
  {
    e.conprintf("SC_LoadLevel: could not open file '%s' for read: %s\n",
                 inpath, strerror(errno));
    sprintf(msg, "Kann Datei %s nicht oeffnen: %s", inpath, strerror(errno));
    MessageBox(NULL, msg, "Fehler", MB_OK | MB_ICONSTOP);
    return 0;
  }

  // read until EOF
  if((c = fgetc(f)) != EOF)
  {
    const char  *errstring;

    ungetc(c, f);
    if((errstring = Level_ReadFile(l, crcfilename, f)))
    {
      fclose(f);
      e.conprintf("SC_LoadLevel: read error in file '%s' at level block: %s\n",
                  inpath, errstring);
      sprintf(msg, "Fehler beim Lesen von %s: %s", inpath, errstring);
      MessageBox(NULL, msg, "Fehler", MB_OK | MB_ICONSTOP);
      return 0;
    }

    //
    // grab filename fron input path
    //

    // walk from back to front until first path separator is encountered or
    // we reached beginning of input path and grab the filename
    for(cc = &inpath[strlen(inpath)-1]; cc>inpath && *cc!='/' && *cc!='\\'; cc--)
      ;
    if(*cc == '/' || *cc == '\\')
      cc++;
    strcpy(infilename, cc);

    //
    // check CRC
    //
    if(strcmp(crcfilename, infilename) != 0)
    {
      e.conprintf("\nCRC ERROR!\n");
      e.conprintf("resolved filename: '%s'\n", crcfilename);
      e.conprintf("expected filename: '%s'\n", infilename);
      sprintf(msg, "CRC Fehler (%s)", infilename);
      MessageBox(NULL, msg, "Fehler", MB_OK | MB_ICONSTOP);
      return 0;
    }

    //
    // read entity blocks
    //
    while((c = fgetc (f)) != EOF)
    {
      scentity_t        *scent;
      const classinfo_t *ci;

      ungetc(c, f);
      if(!(scent = (scentity_t *)MALLOC(sizeof(scentity_t))))
      {
        fclose(f);
        e.conprintf("SC_LoadLevel: out of mem at entity #%i\n", nents);
        exit(EXIT_FAILURE);
      }
      memset(scent, 0, sizeof(scentity_t));
      if(!ScEnt_ReadFile(scent, f))
      {
        fclose(f);
        e.conprintf("SC_LoadLevel: read error in file '%s' at entity #%i: %s\n",
                    inpath, nents, strerror(errno));
        return 0;
      }
      ci = G_GetClassInfoByName(scent->name);

      // cannot happen if properly compiled
      // (but may happen if old binary script is loaded by new code)
      G_Assert(ci, G_Stringf("%s", scent->name));
      // same check again for developers who have outdated script-compilers
      if(!ci)
      {
        e.conprintf("*** ERROR: entity '%s' is NOT recognized! (is your script-compiler current?)\n", scent->name);
        Game_Shutdown(SHUTDOWN_FAILURE);
      }

      scent->sv_entitytype = ci->sv_etype;
      if(scent->sv_entitytype == object_e)
        scent->sv_subtype = ci->sv_objtype;
      scent->mainclass = ci->mainclass;
      scent->subclass = ci->subclass;

      Insert(&l->spawnring, scent);
      nents++;
    }
  }
  fclose(f);
  return 1;
} // SC_LoadLevel


//---------------------------------------------------------
// SC_FreeLevel - free all script entities of given level
//---------------------------------------------------------
void SC_FreeLevel(level_t *l)
{
  scentity_t    *nex;

  while(l->spawnring.next != &l->spawnring)
  {
    FREE(Extract(l->spawnring.next));
  }

  while(l->trashlist)
  {
    nex = l->trashlist->next;
    FREE(l->trashlist);
    l->trashlist = nex;
  }
} // SC_FreeLevel


//---------------------------------------------------------
// SC_RunFrame - spawn entities if it's time to
//---------------------------------------------------------
void SC_RunFrame(level_t *l)
{
  scentity_t    *cur, *nex;

  assert(l);

  // 1 time around the ring...
  for(cur = l->spawnring.next; cur != &l->spawnring; cur = nex)
  {
    nex = cur->next;
    if(cur->spawntime <= *e.gametime)
    {
      gentity_t *newgent;

      if((newgent = EM_SpawnModel(cur->mainclass, cur->subclass)) != NULL)
      {
        vec3_t  up;

        vec3Copy(cur->pos, newgent->sv->position);
        vec3Copy(cur->dir, newgent->sv->direction);
        BuildAxisByDir(newgent->sv->right, up, newgent->sv->direction);
        VectorsToAngles(newgent->sv->right, up, newgent->sv->direction, newgent->sv->angles);
        newgent->sv->speed = cur->speed;

        // check for spawn outside world
        if(!G_EntInsideSphere(newgent, *G_GetWorldCentre(), G_GetWorldRadius()))
        {
          e.conprintf("- WARNING: spawned '%s' outside world: %s\n",
                       newgent->classname, v3s(newgent->sv->position));
          // entity will be automatically freed by world clipper
        }

        EC_StartEntity(newgent);

        // make new entity fade in, except planets, which would look bad
        if((newgent->mainclass != CLASS_NEUTRAL)
        || (newgent->subclass < NEUTRAL_PLANET01 || newgent->subclass > NEUTRAL_PLANET08))
        {

          // HACK: special santa must keep its think() because it needs to race around
          if(newgent->mainclass == CLASS_FRIEND && newgent->subclass == FRIEND_XMAS_SANTA_SPECIAL)
            goto L_special_santa_jump_here;

          newgent->sv->color[0] = newgent->sv->color[1] = newgent->sv->color[2] = 0.0f;

          // HACK: backup server callbacks for restorage when fade-in
          //       is finished
          newgent->sv_callbacks.nextthinktime = newgent->sv->nextthink;
          newgent->sv_callbacks.Think = newgent->sv->think;
          newgent->sv_callbacks.OnCollision = newgent->sv->onCollision;
          newgent->sv_callbacks.OnSrvFree = newgent->sv->onSrvFree;

          // HACK
          newgent->sv->nextthink = 0.0f;
          newgent->sv->think = FadeIn;
          newgent->sv->onCollision = NULL;
          newgent->sv->onSrvFree = NULL;

          L_special_santa_jump_here:
           ;    // to supress compiler warning
        }
      }
      else
      {
        e.conprintf("# SC_RunFrame: could not spawn '%s'\n", cur->name);
      }
      Extract(cur);

      // check if respawn desired
      if(cur->respawnperiod > 0.0f)
      {
        cur->spawntime += cur->respawnperiod;
        Insert(&l->spawnring, cur);
      }
      else
      {
        // put script entity into recycle bin
        cur->next = l->trashlist;
        l->trashlist = cur;
      }
    }
  }
} // SC_RunFrame


//---------------------------------------------------------------------------


//-------------------------------------------------------------
// InitLists
//-------------------------------------------------------------
static void InitLists(level_t *l)
{
  l->spawnring.next = &l->spawnring;
  l->spawnring.prev = &l->spawnring;
  l->trashlist = NULL;
} // InitLists


//-------------------------------------------------------------
// Insert - link an entity according to its spawntime into given ring
//-------------------------------------------------------------
static void Insert(scentity_t *ring, scentity_t *scent)
{
  scentity_t    *cur;

  // find correct position; stop after insert position
  for(cur = ring->next; cur != ring; cur = cur->next)
  {
    if(cur->spawntime > scent->spawntime)
      break;
  }
  // insert before "cur"
  scent->next = cur;
  scent->prev = cur->prev;
  cur->prev->next = scent;
  cur->prev = scent;
} // Insert


//-------------------------------------------------------------
// Extract - extract given entity from its ring and return it
//-------------------------------------------------------------
static scentity_t *Extract(scentity_t *scent)
{
  scent->prev->next = scent->next;
  scent->next->prev = scent->prev;
  return scent;
} // Extract


//---------------------------------
// FadeIn - fade in; if fully bright start the entity
//---------------------------------
static void FadeIn(entity_t *svself)
{
  float brightness;
  int   stop_fading = 0, i;

  brightness = svself->color[0];
  if((brightness += (*e.gametime - *e.lastgametime)) >= 1.0f)
  {
    brightness = 1.0f;
    stop_fading = 1;
  }

  for(i = 0; i < 3; i++)
    svself->color[i] = brightness;

  if(stop_fading)
  {
    gentity_t   *gself = G_GEntity(svself);

    // HACK: restore original server callbacks
    gself->sv->nextthink = gself->sv_callbacks.nextthinktime;
    gself->sv->think = gself->sv_callbacks.Think;
    gself->sv->onCollision = gself->sv_callbacks.OnCollision;
    gself->sv->onSrvFree = gself->sv_callbacks.OnSrvFree;
  }
} // FadeIn


#if 0
//========================================================================
//
// test environment
//
//========================================================================

//-----------------------
// print_ent
//-----------------------
static void print_ent(const scentity_t *ent)
{
  printf("spawntime:     %.2f\n", ent->spawntime);
//  printf("respawnperiod: %.2f\n", ent->respawnperiod);
  printf("name:          %s\n", ent->name);
//  printf("pos:           %.2f %.2f %.2f\n", ent->pos[0], ent->pos[1], ent->pos[2]);
//  printf("dir:           %.2f %.2f %.2f\n", ent->dir[0], ent->dir[1], ent->dir[2]);
//  printf("speed:         %.2f\n", ent->speed);
} // EntBlock_Print

static void printlist(const level_t *l)
{
  scentity_t    *cur;

  printf("===========================\n");
  for(cur = l->spawnring.next; cur != &l->spawnring; cur = cur->next)
  {
    print_ent(cur);
    printf("\n");
  }
}

int main(int argc, char *argv[])
{
  float         gametime;
  level_t       level;
  int           levelnum;

  //if(argc != 2)
  //  Exit("need a compiled level file\n");

  // simulate 3 levels
  for(levelnum = 0; levelnum < 3; levelnum++)
  {
    char    filename[128];

    sprintf(filename, "script%i.level_bin", levelnum + 1);
    SC_LoadLevel(&level, filename);

    printf("===== level %i =====\n", levelnum + 1);
    printf("%s\n", level.title);

    gametime = 0.0;
    while(1)
    {
      SC_Main(&level, gametime);
      if(fgetc(stdin) == 'q')
        break;
      gametime += 0.5;
    }
    SC_FreeLevel(&level);
  }
  printf("# malloc: %i\n# free: %i\n", s_nmalloc, s_nfree);

  return 0;
}
#endif // #if 0 - test environment


#endif  /* SC_GAME */
