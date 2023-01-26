// script.h - shared by the compiler + game code

#ifndef __SCRIPT_H
#define __SCRIPT_H

#include "g_shared.h"

#include "math3d.h"
#include "types.h"


typedef u32_t   scver_t;
typedef u32_t   sccrc_t;

#define CRCLEN  128


static const scver_t    SCRIPTVERSION = 5;  // stored in binary file;
                                            // may change in the future


#define SC_MAXSTRING    128

// info of what, when and where to spawn
typedef struct scentity_s
{
  float         spawntime;
  float         respawnperiod;
  char          name[SC_MAXSTRING + 1];
  vec3_t        pos;
  vec3_t        dir;
  vec3_t        _aim_;  // only for intermediate memory storage; not saved
  float         speed;

// additional info for the game code
#ifdef SC_GAME
  entitytype_t      sv_entitytype;
  objectsubtype_t   sv_subtype;
  mainclass_t       mainclass;
  subclass_t        subclass;
  struct scentity_s *prev, *next;
#endif

} scentity_t;


// info of a level
typedef struct level_s
{
  char      title[SC_MAXSTRING + 1];
  char      bgm[SC_MAXSTRING + 1];
  float     duration;
  char      sky[SC_MAXSTRING + 1];

// additional info for the game code
#ifdef SC_GAME
  scentity_t    spawnring;
  scentity_t    *trashlist; // holds spawned entities to remove after level
#endif

} level_t;


void Level_Print(const level_t *level);
void ScEnt_Print(const scentity_t *scent);

#ifdef SC_COMPILER
// Level_WriteFile - a filename (prefereably without path) is needed to
// compute a CRC and store it
int Level_WriteFile(const level_t *level, const char filename[CRCLEN+1], FILE *f);
int ScEnt_WriteFile(const scentity_t *scent, FILE *f);
void SC_CompileFile(const char *infilename, const char *outdir, int verbose);
#endif

#ifdef SC_GAME
// Level_ReadFile - will also un-CRC the stored filenamd and write it to
// "filename", so the caller can act according to mismatching filenames
// returns NULL if OK, otherwise error string
const char *Level_ReadFile(level_t *out, char filename[CRCLEN+1], FILE *f);
int ScEnt_ReadFile(scentity_t *out, FILE *f);
int SC_LoadLevel(level_t *l, const char *filename);
void SC_FreeLevel(level_t *l);
void SC_RunFrame(level_t *l);
#endif


#endif  /* !__SCRIPT_H */
