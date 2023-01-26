// sc_io.c - read + write level data and script entities

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "script.h"

#ifdef SC_GAME
#include "g_local.h"
#endif

// "script0.txt\0\1\0\0\0\1\2\1"
//

#define XOR_CHAR    'a'

// for hack-safety
static void XORBuffer(char *out, const char *in, int len)
{
  for(; len > 0; out++, in++, len--)
    *out = *in ^ XOR_CHAR;
}




#ifdef SC_COMPILER

//-----------------------------------------
// Level_WriteFile
//-----------------------------------------
int Level_WriteFile(const level_t *level, const char filename[CRCLEN + 1], FILE *f)
{
  char  crcfilename[CRCLEN+1];

  assert(f);

  if(fwrite(&SCRIPTVERSION, sizeof SCRIPTVERSION, 1, f) != 1)
    return 0;

  XORBuffer(crcfilename, filename, CRCLEN);
  if(fwrite(&crcfilename, CRCLEN * sizeof(char), 1, f) != 1)
    return 0;

  if(fwrite(&level->title, sizeof level->title, 1, f) != 1)
    return 0;
  if(fwrite(&level->bgm, sizeof level->bgm, 1, f) != 1)
    return 0;
  if(fwrite(&level->duration, sizeof level->duration, 1, f) != 1)
    return 0;
  if(fwrite(&level->sky, sizeof level->sky, 1, f) != 1)
    return 0;

  return 1;
} // Level_WriteFile

#else   // #ifdef SC_COMPILER...

//-----------------------------------------
// Level_ReadFile
//-----------------------------------------
const char *Level_ReadFile(level_t *out, char filename[CRCLEN + 1], FILE *f)
{
  static char   errstring[256];
  scver_t       version;

  assert(f);

  if(fread(&version, sizeof version, 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }

  // make sure, the level has correct version number
  if(version != SCRIPTVERSION)
  {
  #ifdef SC_GAME
    e.conprintf("# Level_ReadFile: expected version %i, got %i\n", SCRIPTVERSION, version);
  #else
    printf("# Level_ReadFile: expected version %i, got %i\n", SCRIPTVERSION, version);
  #endif
    sprintf(errstring, "Falsche Dateiversion (%i)", version);
    return errstring;
  }

  if(fread(filename, CRCLEN * sizeof(char), 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }
  XORBuffer(filename, filename, CRCLEN);

  if(fread(&out->title, sizeof out->title, 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }

  if(fread(&out->bgm, sizeof out->bgm, 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }

  if(fread(&out->duration, sizeof out->duration, 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }

  if(fread(&out->sky, sizeof out->sky, 1, f) != 1)
  {
    sprintf("Unerwartetes Dateiende (%s)", strerror(errno));
    return errstring;
  }

  return NULL;
} // Level_ReadFile

#endif  // SC_COMPILER

//--------------------------
// Level_Print
//--------------------------
void Level_Print(const level_t *level)
{
#ifdef SC_GAME

  const scentity_t  *cur;

  e.conprintf("========= %s =========== \n", level->title);
  e.conprintf("bgm:      %s\n", level->bgm);
  e.conprintf("duration: %.2f\n", level->duration);
  for(cur = level->spawnring.next; cur != &level->spawnring; cur = cur->next)
    ScEnt_Print(cur);
#else
  printf("========= %s =========== \n", level->title);
  printf("bgm:      %s\n", level->bgm);
  printf("duration: %.2f\n", level->duration);
#endif
} // Level_Print


#ifdef SC_COMPILER

//-----------------------------------------
// ScEnt_WriteFile
//-----------------------------------------
int ScEnt_WriteFile(const scentity_t *ent, FILE *f)
{
  assert(f);

  if(fwrite(&ent->spawntime, sizeof ent->spawntime, 1, f) != 1)
    return 0;
  if(fwrite(&ent->respawnperiod, sizeof ent->respawnperiod, 1, f) != 1)
    return 0;
  if(fwrite(&ent->name, sizeof ent->name, 1, f) != 1)
    return 0;
  if(fwrite(&ent->pos, sizeof ent->pos, 1, f) != 1)
    return 0;
  if(fwrite(&ent->dir, sizeof ent->dir, 1, f) != 1)
    return 0;
  if(fwrite(&ent->speed, sizeof ent->speed, 1, f) != 1)
    return 0;

  return 1;
} // ScEnt_WriteFile

#else   // #ifdef SC_COMPILER

//----------------------------------------
// ScEnt_ReadFile - read an entity from given file into "out"
//                - returns 0/1 depending on success or failure
//----------------------------------------
int ScEnt_ReadFile(scentity_t *out, FILE *f)
{
  assert(f);

  if(fread(&out->spawntime, sizeof out->spawntime, 1, f) != 1)
    return 0;
  if(fread(&out->respawnperiod, sizeof out->respawnperiod, 1, f) != 1)
    return 0;
  if(fread(&out->name, sizeof out->name, 1, f) != 1)
    return 0;
  if(fread(&out->pos, sizeof out->pos, 1, f) != 1)
    return 0;
  if(fread(&out->dir, sizeof out->dir, 1, f) != 1)
    return 0;
  if(fread(&out->speed, sizeof out->speed, 1, f) != 1)
    return 0;

  return 1;
} // ScEnt_ReadFile

#endif  // SC_COMPILER


//-----------------------
// ScEnt_Print
//-----------------------
void ScEnt_Print(const scentity_t *ent)
{
#ifdef SC_GAME
  e.conprintf("spawntime:     %.2f\n", ent->spawntime);
  e.conprintf("respawnperiod: %.2f\n", ent->respawnperiod);
  e.conprintf("name:          %s\n", ent->name);
  e.conprintf("pos:           %.2f %.2f %.2f\n", ent->pos[0], ent->pos[1], ent->pos[2]);
  e.conprintf("dir:           %.2f %.2f %.2f\n", ent->dir[0], ent->dir[1], ent->dir[2]);
  e.conprintf("speed:         %.2f\n", ent->speed);
#else
  printf("spawntime:     %.2f\n", ent->spawntime);
  printf("respawnperiod: %.2f\n", ent->respawnperiod);
  printf("name:          %s\n", ent->name);
  printf("pos:           %.2f %.2f %.2f\n", ent->pos[0], ent->pos[1], ent->pos[2]);
  printf("dir:           %.2f %.2f %.2f\n", ent->dir[0], ent->dir[1], ent->dir[2]);
  printf("speed:         %.2f\n", ent->speed);
#endif
} // ScEnt_Print
