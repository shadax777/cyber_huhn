#ifndef __CLIENT_H
#define __CLIENT_H
// client <-> player interaction

#include "g_local.h"


void CL_SetMouseInversion(int enable);
int CL_Init(void);
void CL_Shutdown(void);
gentity_t *CL_GetPlayerEntity(void);
const gentity_t *CL_GetCameraEntity(void);
void CL_KeyEvent(int key, int ispressed);
void CL_MouseEvent(msg_t msg, int dx, int dy);
void CL_Finish(void);
void CL_Reset(void);
void CL_EnableRender(int enable);
void CL_Pause(void);
void CL_Resume(void);
void CL_RunFrame(float timeleft);

struct client_s;

void CL_OnPlayerHasKilled(struct client_s *cl, const gentity_t *corpse);
void CL_OnPlayerReceivesItem(struct client_s *cl, const gentity_t *item);
void CL_OnPlayerItemExpires(struct client_s *cl, const gentity_t *item);


#endif  /* __CLIENT_H */
