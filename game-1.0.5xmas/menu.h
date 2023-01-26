#ifndef __MENU_H
#define __MENU_H

#include "g_local.h"


#define MAX_MENUITEMS   8   // should be sufficient for now


typedef struct menuitem_s   menuitem_t;
typedef struct menu_s       menu_t;


struct menuitem_s
{
  const char    *name;  // optional
  gentity_t *image;
  gentity_t *caption;
  float     pos[2];
  int       mins[2], maxs[2];   // bounding quad; default = image dimension
  void      (*OnTouch)(menuitem_t *self);
  void      (*OnClick)(menuitem_t *self);
  void      (*Idle)(menuitem_t *self);  // if mouse doesn't touch item
};


// private!!
typedef struct
{
  int       x, y;
  gentity_t *image;
} cursor_t;


struct menu_s
{
  const char    *name;  // optional
  float         pos[2];
  menuitem_t    *items[MAX_MENUITEMS];
  int           numitems;
  cursor_t      cursor;
};


int MenuItem_Create(menuitem_t *item, subclass_t imageclass, const char *caption);
void MenuItem_SetOnTouchHandler(menuitem_t *item, void (*OnTouch)(menuitem_t *self));
void MenuItem_SetOnClickHandler(menuitem_t *item, void (*OnClick)(menuitem_t *self));
void MenuItem_SetIdleHandler(menuitem_t *item, void (*Idle)(menuitem_t *self));
void MenuItem_SetGamma(menuitem_t *item, float gamma);
void MenuItem_ResizeBQuad(menuitem_t *item, int left, int right, int top, int bottom);

void Menu_Create(menu_t *menu, float absx, float absy);
void Menu_Destroy(menu_t *menu);
// note: "relx" and "rely" define relative position in a menu;
void Menu_AddItem(menu_t *menu, menuitem_t *item, int relx, int rely);
void Menu_RunFrame(menu_t *menu); // call touch- or idle-handler of each item
void Menu_HandleMouseMovement(menu_t *menu, int dx, int dy);
void Menu_HandleMouseClick(menu_t *menu);
void Menu_EnableRender(menu_t *menu, int enable);


#endif  // !__MENU_H
