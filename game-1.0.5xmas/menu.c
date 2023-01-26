// general purpose menu manager

#include "menu.h"


static void MenuItem_SetAbsPosition(menuitem_t *item, float absx, float absy);
static int MenuItem_Touched(const menuitem_t *item, int absx, int absy);
static void MenuItem_EnableRender(menuitem_t *item, int enable);

static int Cursor_Create(cursor_t *c, int x, int y);
static void Cursor_UpdatePos(cursor_t *c, float dx, float dy);
static void Cursor_EnableRender(cursor_t *c, int enable);


//-----------------------------------------------------------
// MenuItem_Create
//-----------------------------------------------------------
int MenuItem_Create(menuitem_t *item, subclass_t imageclass, const char *caption)
{
  // spawn image
  if(!(item->image = EM_SpawnImage(imageclass)))
  {
    e.conprintf("# MenuItem_Create: could not spawn imageclass %i (probably too many entities: %i)\n", imageclass, EM_NumEntities());
    return 0;
  }

  // spawn caption (tech font)
  if(!(item->caption = EM_SpawnText(TEXT_TECH)))
  {
    e.conprintf("# MenuItem_Create: could not spawn tech text (probably too many entities: %i)\n", EM_NumEntities());
    return 0;
  }

  // set (relative) position in menu
  // note: Menu_AddItem() will set the absolute position
  //MenuItem_SetAbsPosition(item, relx, rely);

  // set default bounding quad
  item->mins[0] = -item->image->sv->i.xsize / 2;
  item->maxs[0] =  item->image->sv->i.xsize / 2 - 1;
  item->mins[1] = -item->image->sv->i.ysize / 2;
  item->maxs[1] =  item->image->sv->i.ysize / 2 - 1;

  // set caption
  assert(caption);
  E_Text_Sprintf(item->caption, "%s", caption);
  E_Text_SetSize(item->caption, 25);

  return 1;
} // MenuItem_Create


//-----------------------------------------------------------
// MenuItem_SetOnTouchHandler
//-----------------------------------------------------------
void MenuItem_SetOnTouchHandler(menuitem_t *item, void (*OnTouch)(menuitem_t *self))
{
  item->OnTouch = OnTouch;
}


//-----------------------------------------------------------
// MenuItem_SetOnClickHandler
//-----------------------------------------------------------
void MenuItem_SetOnClickHandler(menuitem_t *item, void (*OnClick)(menuitem_t *self))
{
  item->OnClick = OnClick;
}


//-----------------------------------------------------------
// MenuItem_SetIdleHandler
//-----------------------------------------------------------
void MenuItem_SetIdleHandler(menuitem_t *item, void (*Idle)(menuitem_t *self))
{
  item->Idle = Idle;
}


//-----------------------------------------------------------
// MenuItem_SetGamma
//-----------------------------------------------------------
void MenuItem_SetGamma(menuitem_t *item, float gamma)
{
  assert(item);
  G_Assert(item->image, (item->name ? item->name : "(null)"));
  G_Assert(item->caption, (item->name ? item->name : "(null)"));

  if(gamma >= 0.0f && gamma <= 1.0f)
  {
    item->image->sv->gamma = gamma;
    item->caption->sv->gamma = gamma;
  }
} // MenuItem_SetGamma


//-----------------------------------------------------------
// MenuItem_ResizeBQuad
//-----------------------------------------------------------
void MenuItem_ResizeBQuad(menuitem_t *item, int left, int right, int top, int bottom)
{
  item->mins[0] += left;
  item->maxs[0] += right;
  item->maxs[1] += top;
  item->mins[1] += bottom;
} // MenuItem_ResizeBQuad


//---------------------------------------------------------------------------


//-----------------------------------------------------------
// MenuItem_Destroy
//-----------------------------------------------------------
static void MenuItem_Destroy(menuitem_t *item)
{
  assert(item);
  G_Assert(item->image, (item->name ? item->name : "(null)"));
  G_Assert(item->caption, (item->name ? item->name : "(null)"));

  EM_Free(item->image);
  EM_Free(item->caption);
  item->image = item->caption = NULL;
} // MenuItem_Destroy


//-----------------------------------------------------------
// MenuItem_SetAbsPosition
//-----------------------------------------------------------
static void MenuItem_SetAbsPosition(menuitem_t *item, float absx, float absy)
{
  assert(item);
  G_Assert(item->image, (item->name ? item->name : "(null)"));
  G_Assert(item->caption, (item->name ? item->name : "(null)"));

  item->pos[0] = absx;
  item->pos[1] = absy;
  item->image->sv->position[0] = absx;
  item->image->sv->position[1] = absy;
  item->caption->sv->position[0] = absx;
  item->caption->sv->position[1] = absy;
} // MenuItem_SetAbsPosition


//-----------------------------------------------------------
// MenuItem_Touched
//-----------------------------------------------------------
static int MenuItem_Touched(const menuitem_t *item, int absx, int absy)
{
  int   absmins[2], absmaxs[2];

  absmins[0] = item->pos[0] + item->mins[0];
  absmaxs[0] = item->pos[0] + item->maxs[0];
  absmins[1] = item->pos[1] + item->mins[1];
  absmaxs[1] = item->pos[1] + item->maxs[1];

  if(absx < absmins[0]) return 0;
  if(absx > absmaxs[0]) return 0;
  if(absy < absmins[1]) return 0;
  if(absy > absmaxs[1]) return 0;
  return 1;
} // MenuItem_Touched


//-----------------------------------------------------------
// MenuItem_EnableRender
//-----------------------------------------------------------
static void MenuItem_EnableRender(menuitem_t *item, int enable)
{
  assert(item);
  G_Assert(item->image, (item->name ? item->name : "(null)"));
  G_Assert(item->caption, (item->name ? item->name : "(null)"));

  item->image->sv->state   = enable ? active_e : passive_e;
  item->caption->sv->state = enable ? active_e : passive_e;
} // MenuItem_EnableRender


//===========================================================================


//-----------------------------------------------------------
// Cursor_Create
//-----------------------------------------------------------
static int Cursor_Create(cursor_t *c, int x, int y)
{
  if(!(c->image = EM_SpawnImage(IMAGE_MOUSECURSOR)))
  {
    e.conprintf("# Cursor_Create: could not spawn cursor image (probably too many entities: %i)\n", EM_NumEntities());
    return 0;
  }
  c->x = x;
  c->y = y;
  Cursor_UpdatePos(c, 0, 0);
  return 1;
} // Cursor_Create


//-----------------------------------------------------------
// Cursor_Destroy
//-----------------------------------------------------------
static void Cursor_Destroy(cursor_t *c)
{
  assert(c);
  assert(c->image);

  EM_Free(c->image);
  c->image = NULL;
} // Cursor_Destroy


//-----------------------------------------------------------
// Cursor_UpdatePos
//-----------------------------------------------------------
static void Cursor_UpdatePos(cursor_t *c, float dx, float dy)
{
  assert(c);
  assert(c->image);

  c->x = G_Round((float)c->x + dx);
  c->y = G_Round((float)c->y + dy);

  // clamp cursor on screen area
  if(c->x < 0)  c->x = 0;
  if(c->x > VW) c->x = VW;
  if(c->y < 0)  c->y = 0;
  if(c->y > VH) c->y = VH;

  // cursor must always be nearly in the centre of the image
  c->image->sv->position[0] = c->x + 4;
  c->image->sv->position[1] = c->y - 4;
} // Cursor_UpdatePos


//-----------------------------------------------------------
// Cursor_EnableRender
//-----------------------------------------------------------
static void Cursor_EnableRender(cursor_t *c, int enable)
{
  assert(c);
  assert(c->image);

  c->image->sv->state = enable ? active_e : passive_e;
}


//===========================================================================


//-----------------------------------------------------------
// Menu_Create
//-----------------------------------------------------------
void Menu_Create(menu_t *menu, float absx, float absy)
{
  memset(menu->items, 0, NELEMS(menu->items) * sizeof(menu->items[0]));
  menu->numitems = 0;
  menu->pos[0] = absx;
  menu->pos[1] = absy;
  Cursor_Create(&menu->cursor, (int)absx, (int)absy);
} // Menu_Create


//-----------------------------------------------------------
// Menu_Destroy
//-----------------------------------------------------------
void Menu_Destroy(menu_t *menu)
{
  while(menu->numitems > 0)
  {
    MenuItem_Destroy(menu->items[--menu->numitems]);
  }
  Cursor_Destroy(&menu->cursor);
} // Menu_Destroy


//-----------------------------------------------------------
// Menu_AddItem - add item and update its position relative to menu
//-----------------------------------------------------------
void Menu_AddItem(menu_t *menu, menuitem_t *item, int relx, int rely)
{
  assert(menu);
  assert(item);

  // here, the item's pos is still relative to menu

  if(menu->numitems < MAX_MENUITEMS)
  {
    float   absx, absy;

    // set item to absolute position
    absx = menu->pos[0] + relx;
    absy = menu->pos[1] + rely;
    MenuItem_SetAbsPosition(item, absx, absy);
    menu->items[menu->numitems++] = item;
  }
} // Menu_AddItem


//-----------------------------------------------------------
// Menu_RunFrame - call touch- or idle-handler of each item
//-----------------------------------------------------------
void Menu_RunFrame(menu_t *menu)
{
  int   i;

  assert(menu);
  assert(menu->numitems <= MAX_MENUITEMS);

  for(i = 0; i < menu->numitems; i++)
  {
    menuitem_t  *item = menu->items[i];

    assert(item);
    if(MenuItem_Touched(item, menu->cursor.x, menu->cursor.y))
    {
      if(item->OnTouch)
        item->OnTouch(item);
    }
    else
    {
      if(item->Idle)
        item->Idle(item);
    }
  }
} // Menu_RunFrame


//-----------------------------------------------------------
// Menu_HandleMouseMovement
//-----------------------------------------------------------
void Menu_HandleMouseMovement(menu_t *menu, int dx, int dy)
{
  assert(menu);

  Cursor_UpdatePos(&menu->cursor, dx * 0.005f, dy * 0.005f);
} // Menu_HandleMouseMovement


//-----------------------------------------------------------
// Menu_HandleMouseClick
//-----------------------------------------------------------
void Menu_HandleMouseClick(menu_t *menu)
{
  int   i;

  assert(menu);
  assert(menu->numitems <= MAX_MENUITEMS);

  for(i = 0; i < menu->numitems; i++)
  {
    menuitem_t  *item = menu->items[i];

    assert(item);
    if(MenuItem_Touched(item, menu->cursor.x, menu->cursor.y))
    {
      if(item->OnClick)
        item->OnClick(item);
    }
  }
} // Menu_HandleMouseClick


//-----------------------------------------------------------
// Menu_EnableRender
//-----------------------------------------------------------
void Menu_EnableRender(menu_t *menu, int enable)
{
  int   i;

  assert(menu);
  assert(menu->numitems <= MAX_MENUITEMS);

  Cursor_EnableRender(&menu->cursor, enable);

  for(i = 0; i < menu->numitems; i++)
  {
    menuitem_t  *item = menu->items[i];

    assert(item);
    MenuItem_EnableRender(item, enable);
  }
} // Menu_EnableRender
