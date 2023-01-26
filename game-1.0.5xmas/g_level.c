// level routines

#include "g_local.h"
#include "script.h"


#define MAX_LEVELS  8

static level_t  s_levels[MAX_LEVELS];
static int      s_numlevels;
static int      s_curlevelnum;


//-----------------------------------------------------
// G_LoadLevel - attempt to load a level an append it to others in
//               memory
//             - returns a pointer to the level or NULL
//-----------------------------------------------------
const level_t *G_LoadLevel(const char *filename)
{
  assert(s_numlevels < MAX_LEVELS);

  if(!SC_LoadLevel(&s_levels[s_numlevels], filename))
  {
    return NULL;
  }
  else
  {
    return &s_levels[s_numlevels++];
  }
} // G_LoadLevel


//---------------------------------------
// G_NumLoadedLevels
//---------------------------------------
int G_NumLoadedLevels(void)
{
  return s_numlevels;
}


//--------------------------------------
// G_FreeLevels - free all loaded levels
//--------------------------------------
void G_FreeLevels(void)
{
  while(s_numlevels > 0)
  {
    SC_FreeLevel(&s_levels[--s_numlevels]);
  }
} // G_FreeLevels


//---------------------------------------
// G_GetCurLevel
//---------------------------------------
level_t *G_GetCurLevel(void)
{
  return &s_levels[s_curlevelnum];
}


//--------------------------------------------------
// G_AdvanceLevel - attempt to advance to next level in memory
//                - returns pointer to next level if successful; NULL if
//                  no more levels available
//--------------------------------------------------
struct level_s *G_AdvanceLevel(void)
{
  if(s_curlevelnum >= s_numlevels - 1)
    return NULL;
  else
    return &s_levels[++s_curlevelnum];
} // G_AdvanceLevel
