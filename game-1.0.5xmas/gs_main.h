// game states

#ifndef __GS_MAIN_H
#define __GS_MAIN_H


// engine header
#include "ms.h"     // msg_t


typedef enum
{
  GS_INVALID,
  GS_TITLESCREEN,
  GS_ACTION,
  GS_FADEIN,
  GS_FADEOUT,
  GS_INTERMISSION,
  GS_PAUSE,
  GS_GAMEOVER,
} gsid_t;


void    GS_Init(void);
gsid_t  GS_GetCurrentState(void);
gsid_t  GS_GetPendingState(void);
void    GS_SetPendingState(gsid_t id);
void    GS_ChangeState(gsid_t id);  // will not affect the pending state
void    GS_DebugPrintCurrentState(void);

struct level_s;

extern void (*GS_RunFrame)(struct level_s *level);
extern void (*GS_KeyEvent)(int key, int ispressed);
extern void (*GS_MouseEvent)(msg_t msg, int dx, int dy);


#endif  /* __GS_MAIN_H */
