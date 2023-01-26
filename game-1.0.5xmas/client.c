#include "g_local.h"
#include "client.h"


#define DEBUG_INFO  0   // prints pos, dir, angles and no. of entities
//
// WARNING:
// if debug is endabled, G_SpawnPersistent() and other no-longer-existing
// functions will be called!! this must be fixed
//


#define TIMEUPWARNING_BEGIN         5.999999f
#define TIMEUPWARNING_INITIAL_SIZE  300.0f

// arrow position in cockpit: relative to player's position
#define ARROW_X     (0.0f)
#define ARROW_Y     (16.0f)
#define ARROW_Z     (30.0f)
#define ARROW_SCALE     (0.08f)
#define ARROW_OPACITY   (1.0f)


typedef struct
{
  gentity_t     *scoretext;
  gentity_t     *timetext;
  gentity_t     *timeup_warner; // prints remaining seconds if time is nearly up
  gentity_t     *wp_text;
  gentity_t     *wp_icons[NUM_WEAPONS];
  weapontype_t  curweapon;
  gentity_t     *bonusweapon_text;  // "Bonus-Waffe: 0:07"
} hud_t;


typedef struct
{
  gentity_t         *gent;  // self, for pos, dir, etc.
  const gentity_t   *hooked_gent;
  float             hooktime;
} camera_t;


typedef struct client_s
{
  camera_t  camera;
  gentity_t *player;
  gentity_t *cockpitmodel;
  gentity_t *weaponmodel;
  gentity_t *arrowmodel;    // -not- fixed, because it shall always point at
                            // a constant point somewhere in the distance. it
                            // will only change its position if player moves
  hud_t     hud;
  int       firepressed;
  float     lastshoottime;
} client_t;


static int Cam_Create(camera_t *cam);
static void Cam_Destroy(camera_t *cam);
static void Cam_HookEntity(camera_t *cam, const gentity_t *gent);
static void Cam_SetFreeStanding(camera_t *cam, const vec3_t pos, const vec3_t dir);
static void Cam_RunFrame(camera_t *cam);

// ChangeToWeapon - also changes icon in HUD
static void ChangeToWeapon(client_t *cl, weapontype_t weapon);
static void RequestShoot(client_t *cl);

static int HUD_Create(hud_t *hud);
static void HUD_Shutdown(hud_t *hud);
static void HUD_EnableRender(hud_t *hud, int enable);
static void HUD_UpdateItemTime(hud_t *hud, const gentity_t *item);
static void HUD_UpdateLevelTime(hud_t *hud, float timeleft);
static void HUD_UpdateScoreboard(hud_t *hud, int score);
static void HUD_SetWeaponIcon(hud_t *hud, weapontype_t weapon);
static void HUD_OnReceiveItem(hud_t *hud, const gentity_t *item);
static void HUD_OnItemExpires(hud_t *hud, const gentity_t *item);


static client_t s_client;
static int      s_mouseinverted = 0;

static const char   *s_strfmt_bonusweapon,
                    *s_strfmt_score,
                    *s_str_weapon,
                    *s_strfmt_time,
                    *s_str_timeup;


// only for debugging
#if DEBUG_INFO
static gentity_t    *s_dirtext, *s_anglestext, *s_postext, *s_numentstext,
                    *s_numrentstext;


//-------------------------
// debuginfo_init
//-------------------------
static void debuginfo_init(void)
{
  const classinfo_t *ci;

  ci = G_GetClassInfoByName(CLASSNAME_FONT_NORMAL);
  assert(ci);

  if(!s_dirtext && (s_dirtext = G_SpawnPersistent(ci->classname, ci->sv_etype, ci->classid)))
  {
    s_dirtext->sv->position[0] = VW - 140;
    s_dirtext->sv->position[1] = VH/3;
    s_dirtext->sv->t.size = 16;
    sprintf(s_dirtext->sv->t.message, "dir: %s", v3s(s_client.player->sv->direction));
  }

  if(!s_anglestext && (s_anglestext = G_SpawnPersistent(ci->classname, ci->sv_etype, ci->classid)))
  {
    s_anglestext->sv->position[0] = VW - 140;
    s_anglestext->sv->position[1] = VH/3 - 20;
    s_anglestext->sv->t.size = 16;
    sprintf(s_anglestext->sv->t.message, "ang: %s", v3s(s_client.player->sv->angles));
  }

  if(!s_postext && (s_postext = G_SpawnPersistent(ci->classname, ci->sv_etype, ci->classid)))
  {
    s_postext->sv->position[0] = VW - 140;
    s_postext->sv->position[1] = VH/3 - 60;
    s_postext->sv->t.size = 16;
    sprintf(s_postext->sv->t.message, "pos: %s", v3s(s_client.player->sv->position));
  }

  if(!s_numentstext && (s_numentstext = G_SpawnPersistent(ci->classname, ci->sv_etype, ci->classid)))
  {
    s_numentstext->sv->position[0] = VW - 110;
    s_numentstext->sv->position[1] = VH/3 - 80;
    s_numentstext->sv->t.size = 16;
    sprintf(s_numentstext->sv->t.message, "ents: %i", EM_NumEntities());
  }

  if(!s_numrentstext && (s_numrentstext = G_SpawnPersistent(ci->classname, ci->sv_etype, ci->classid)))
  {
    s_numrentstext->sv->position[0] = VW - 110;
    s_numrentstext->sv->position[1] = VH/3 - 100;
    s_numrentstext->sv->t.size = 16;
    sprintf(s_numrentstext->sv->t.message, "r ents: %i", EM_NumRenderEntities());
  }
} // debuginfo_init


//-------------------------
// debuginfo_update
//-------------------------
static void debuginfo_update(void)
{
  if(s_dirtext)
    sprintf(s_dirtext->sv->t.message, "dir: %s", v3s(s_client.player->sv->direction));

  if(s_anglestext)
    sprintf(s_anglestext->sv->t.message, "ang: %s", v3s(s_client.player->sv->angles));

  if(s_postext)
    sprintf(s_postext->sv->t.message, "pos: %s", v3s(s_client.player->sv->position));

  if(s_numentstext)
    sprintf(s_numentstext->sv->t.message, "ents: %i", EM_NumEntities());

  if(s_numrentstext)
    sprintf(s_numrentstext->sv->t.message, "r ents: %i", EM_NumRenderEntities());
} // debuginfo_update

#endif // DEBUG_INFO


//---------------------------------
// CL_SetMouseInversion
//---------------------------------
void CL_SetMouseInversion(int enable)
{
  s_mouseinverted = enable != 0;
} // CL_SetMouseInversion


// g_xmas_snowcloud.c
extern void SnowCloud_Start(const vec3_t clientpos, int numsnowflakes);


//---------------------------------
// CL_Init
//---------------------------------
int CL_Init(void)
{
  const classinfo_t *ci;
  vec3_t            up;

  // get localized strings from engine
  // note: it's guaranted that the engine never returns NULL
  s_strfmt_bonusweapon = e.lang_getlocalizedstr(STRID_BONUSWEAPON);
  s_strfmt_score       = e.lang_getlocalizedstr(STRID_SCORE);
  s_str_weapon         = e.lang_getlocalizedstr(STRID_WEAPON);
  s_strfmt_time        = e.lang_getlocalizedstr(STRID_TIME);
  s_str_timeup         = e.lang_getlocalizedstr(STRID_TIMEUP);


  ci = G_GetClassInfo(CLASS_PLAYER, PLAYER_CLIENT);
  assert(ci);
  if(!(s_client.player = EM_SpawnByClassInfo(ci)))
  {
    e.conprintf("CL_Init: could not spawn player entity\n");
    return 0;
  }

  ci = G_GetClassInfo(CLASS_CLIENT, CLIENT_COCKPITMODEL);
  assert(ci);
  if(!(s_client.cockpitmodel = EM_SpawnByClassInfo(ci)))
  {
    e.conprintf("CL_Init: could not spawn cockpit model '%s'\n", ci->classname);
    return 0;
  }

  ci = G_GetClassInfo(CLASS_CLIENT, CLIENT_WEAPONMODEL);
  assert(ci);
  if(!(s_client.weaponmodel = EM_SpawnByClassInfo(ci)))
  {
    e.conprintf("CL_Init: could not spawn weapon model '%s'\n", ci->classname);
    return 0;
  }

  ci = G_GetClassInfo(CLASS_CLIENT, CLIENT_ARROWMODEL);
  assert(ci);
  if(!(s_client.arrowmodel = EM_SpawnByClassInfo(ci)))
  {
    e.conprintf("CL_Init: could not spawn arrow model '%s'\n", ci->classname);
    return 0;
  }

  if(!Cam_Create(&s_client.camera))
  {
    return 0;
  }

  if(!HUD_Create(&s_client.hud))
  {
    return 0;
  }

  s_client.firepressed = 0;
  s_client.lastshoottime = 0.0f;
  s_client.player->n.player.score = 0;
  ChangeToWeapon(&s_client, WP_PLASMAGUN);
  s_client.player->n.player.client = &s_client;

  // scale + position cockpit model
  E_Model_SetScale(s_client.cockpitmodel, 0.01f);
  s_client.cockpitmodel->sv->position[2] = -1.0f;

  // scale + position weapon model
  E_Model_SetScale(s_client.weaponmodel, 0.01f);
  s_client.weaponmodel->sv->position[2] = -1.6f;

  // arrow: point straight forward
  vec3Copy(AXIS[2], s_client.arrowmodel->sv->direction);
  vec3Normalize(s_client.arrowmodel->sv->direction);
  BuildAxisByDir(s_client.arrowmodel->sv->right, up, s_client.arrowmodel->sv->direction);
  VectorsToAngles(s_client.arrowmodel->sv->right, up, s_client.arrowmodel->sv->direction, s_client.arrowmodel->sv->angles);

  // scale arrow model + set opacity
  E_Model_SetScale(s_client.arrowmodel, ARROW_SCALE);
  s_client.arrowmodel->sv->color[3] = ARROW_OPACITY;

  // set player position
  vec3Copy(ZEROVEC, s_client.player->sv->position);

  // set player direction into screen
  vec3Copy(AXIS[2], s_client.player->sv->direction);
  vec3Normalize(s_client.player->sv->direction);

  // create player angles
  VectorsToAngles(s_client.player->sv->right, AXIS[1],
                  s_client.player->sv->direction, s_client.player->sv->angles);
  CL_Reset();
  Cam_HookEntity(&s_client.camera, s_client.player);
  Player_Start(s_client.player);

  // if running an x-mas level start the snow cloud
  if(g_xmasrunning)
  {
    SnowCloud_Start(s_client.player->sv->position, 170);
  }

#if DEBUG_INFO
  debuginfo_init();
#endif

  return 1;
} // CL_Init


//---------------------------------
// CL_Shutdown
//---------------------------------
void CL_Shutdown(void)
{
  Cam_Destroy(&s_client.camera);

  if(s_client.player)
    EM_Free(s_client.player);
  s_client.player = NULL;

  if(s_client.cockpitmodel)
    EM_Free(s_client.cockpitmodel);
  s_client.cockpitmodel = NULL;

  if(s_client.weaponmodel)
    EM_Free(s_client.weaponmodel);
  s_client.weaponmodel = NULL;

  if(s_client.arrowmodel)
    EM_Free(s_client.arrowmodel);
  s_client.arrowmodel = NULL;

  HUD_Shutdown(&s_client.hud);
} // CL_Shutdown


//---------------------------------
// CL_GetPlayerEntity
//---------------------------------
gentity_t *CL_GetPlayerEntity(void)
{
  assert(s_client.player);
  return s_client.player;
}


//---------------------------------
// CL_GetCameraEntity
//---------------------------------
const gentity_t *CL_GetCameraEntity(void)
{
  assert(s_client.camera.gent);
  return s_client.camera.gent;
}


//----------------------------------
// CL_KeyEvent
//----------------------------------
void CL_KeyEvent(int key, int ispressed)
{
  // no keyboard keys used at the moment

  // TEST: provoke bonus weapon banner
/*
  switch(key)
  {
    case K_A:
    case K_a:
      CL_OnGivePlayerBonusWeapon(&s_client);
      break;
  }
*/
} // CL_KeyEvent


//----------------------------------
// CL_MouseEvent
//----------------------------------
void CL_MouseEvent(msg_t msg, int dx, int dy)
{
  int   is_shaking;

  assert(s_client.player);
  G_Assert(s_client.player->mainclass == CLASS_PLAYER, G_Stringf("classname: '%s'", s_client.player->classname));

  // see if shaking, because we will ignore some events then
  is_shaking = Player_IsShaking(s_client.player);

  switch(msg)
  {
    case MSG_LBUTTONDOWN:
      s_client.firepressed = 1;
      break;

    case MSG_LBUTTONUP:
      s_client.firepressed = 0;
      break;

    // cycle through weapons if not being shaked
    case MSG_RBUTTONDOWN:
      if(!is_shaking)
      {
        weapontype_t    wp = (s_client.player->curweapon + 1) % NUM_WEAPONS;

        // skip homing gun if not available
        if(!E_HasItem(s_client.player, ITEM_BONUSWEAPON) && wp == WP_HOMINGGUN)
        {
          wp = (wp + 1) % NUM_WEAPONS;
        }

        // if we're running an x-mas level skip glibber-gun, otherwise
        // skip snowball-gun
        if(g_xmasrunning)
        {
          if(wp == WP_GLIBBERGUN)
            wp = (wp + 1) % NUM_WEAPONS;
        }
        else
        {
          if(wp == WP_SNOWBALLGUN)
            wp = (wp + 1) % NUM_WEAPONS;
        }

        // skip homing gun a second time
        // FIXME: this is -really- bad style, but time pressure....
        if(!E_HasItem(s_client.player, ITEM_BONUSWEAPON) && wp == WP_HOMINGGUN)
        {
          wp = (wp + 1) % NUM_WEAPONS;
        }

        ChangeToWeapon(&s_client, wp);
      }
      break;

    case MSG_MOUSEMOVE:
      if(!is_shaking)
      {
        // dx and dy are already proportional to machine speed, so, no additional
        // mess with elapsed time is necessary
        if(dx)
        {
          Player_Yaw(s_client.player, dx * 0.0005f);
        }
        if(dy)
        {
          if(s_mouseinverted)
            dy = -dy;
          Player_Pitch(s_client.player, -dy * 0.0005f);
        }
      }
      break;

    default:
      break;
  }
} // CL_MouseEvent


//-------------------------------------------
// CL_Finish - finish what player is currently doing
//-------------------------------------------
void CL_Finish(void)
{
  Player_Finish(s_client.player);
  s_client.firepressed = 0;
} // CL_Finish


//-------------------------------------------
// CL_Reset
//-------------------------------------------
void CL_Reset(void)
{
  CL_Finish();
  Player_Reset(s_client.player);
} // CL_Reset


//----------------------------------
// CL_EnableRender
//----------------------------------
void CL_EnableRender(int enable)
{
  HUD_EnableRender(&s_client.hud, enable);
  s_client.player->sv->state = enable ? active_e : passive_e;   // obsolete?
  s_client.cockpitmodel->sv->state = enable ? active_e : passive_e;
  s_client.weaponmodel->sv->state = enable ? active_e : passive_e;
  s_client.arrowmodel->sv->state = enable ? active_e : passive_e;
} // CL_EnableRender


//----------------------------------------------------------
// CL_Pause
//----------------------------------------------------------
void CL_Pause(void)
{
} // CL_Pause


//----------------------------------------------------------
// CL_Resume
//----------------------------------------------------------
void CL_Resume(void)
{
  s_client.firepressed = 0;
} // CL_Resume


//----------------------------------------------------------
// CL_RunFrame
//----------------------------------------------------------
void CL_RunFrame(float timeleft)
{
  vec3_t    dummyright, up; // of player
  int       i;

  // check if shoot requested
  if(s_client.firepressed)
    RequestShoot(&s_client);

  // update arrow in cockpit
  BuildAxisByDir(dummyright, up, s_client.player->sv->direction);
  vec3Copy(s_client.player->sv->position, s_client.arrowmodel->sv->position);
  for(i = 0; i < 3; i++)
  {
    s_client.arrowmodel->sv->position[i] += s_client.player->sv->right[i] * ARROW_X;
    s_client.arrowmodel->sv->position[i] += up[i] * ARROW_Y;
    s_client.arrowmodel->sv->position[i] += s_client.player->sv->direction[i] * ARROW_Z;
  }

// TEST - cycle through all active entities with camera
#if 0
  // if 1 sec. elapsed hook camera to next enemy or stick to player
  if(*e.gametime - s_client.camera.hooktime > 1.0f)
  {
    const gentity_t *next;

    // stop at an enemy
    next = s_client.camera.hooked_gent;
    while((next = EM_NextActiveEntity(next)) != NULL)
    {
      if(next->mainclass == CLASS_ENEMY)
        break;
    }
    if(!next)
      next = s_client.player;
    Cam_HookEntity(&s_client.camera, next);
  }
#endif

  // update camera
  Cam_RunFrame(&s_client.camera);

  // update item countdown in HUD
  if(s_client.player->item != NULL)
  {
    HUD_UpdateItemTime(&s_client.hud, s_client.player->item);
  }

  HUD_UpdateLevelTime(&s_client.hud, timeleft);
} // CL_RunFrame


//----------------------------------------------------------
// CL_OnPlayerHasKilled - spawn killing score in world + update scoreboard in HUD
//----------------------------------------------------------
void CL_OnPlayerHasKilled(struct client_s *cl, const gentity_t *corpse)
{
  const scoreinfo_t *si;

  if((si = EC_GetKillScoreInfo(corpse->mainclass, corpse->subclass)))
  {
    vec3_t  scoredir, scorepos;

    // spawn score entity; set a bit in player's direction to be better
    // visible
    vec3Sub(cl->player->sv->position, corpse->sv->position, scoredir);
    vec3Normalize(scoredir);
    vec3MA(corpse->sv->position, scoredir, 20.0f, scorepos);
    Score(scorepos, si->value, si->size, si->color);

    // update scoreboard in HUD
    HUD_UpdateScoreboard(&cl->hud, cl->player->n.player.score);
  }
} // CL_OnPlayerHasKilled


//----------------------------------------------------------
// CL_OnPlayerReceivesItem
//----------------------------------------------------------
void CL_OnPlayerReceivesItem(struct client_s *cl, const gentity_t *item)
{
  assert(item);

  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      ChangeToWeapon(cl, WP_HOMINGGUN);
      break;

    default:    // forgot to update for freshly introduced item
      G_Assert(0, E_GetString(item));
  }
  HUD_OnReceiveItem(&cl->hud, item);
} // CL_OnPlayerReceivesItem


//----------------------------------------------------------
// CL_OnPlayerItemExpires
//----------------------------------------------------------
void CL_OnPlayerItemExpires(struct client_s *cl, const gentity_t *item)
{
  assert(item);

  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      ChangeToWeapon(cl, WP_PLASMAGUN);
      break;

    default:    // forgot to update for freshly introduced item
      G_Assert(0, E_GetString(item));
  }
  HUD_OnItemExpires(&cl->hud, item);
} // CL_OnPlayerItemExpires


//---------------------------------------------------------------------------


//----------------------------------------------------------
// Cam_Create
//----------------------------------------------------------
static int Cam_Create(camera_t *cam)
{
  const classinfo_t *ci;

  ci = G_GetClassInfo(CLASS_CLIENT, CLIENT_CAMERA);
  assert(ci);
  if(!(s_client.camera.gent = EM_SpawnByClassInfo(ci)))
  {
    e.conprintf("# Cam_Create: could not spawn camera entity\n");
    return 0;
  }
  cam->hooked_gent = NULL;
  return 1;
} // Cam_Create


//----------------------------------------------------------
// Cam_Destroy
//----------------------------------------------------------
static void Cam_Destroy(camera_t *cam)
{
  if(cam->gent != NULL)
  {
    EM_Free(cam->gent);
    cam->gent = NULL;
  }
  cam->hooked_gent = NULL;
}// Cam_Destroy


//----------------------------------------------------------
// Cam_HookEntity
//----------------------------------------------------------
static void Cam_HookEntity(camera_t *cam, const gentity_t *gent)
{
  cam->hooked_gent = gent;
  cam->hooktime = *e.gametime;
} // Cam_HookEntity


//----------------------------------------------------------
// Cam_SetFreeStanding
//----------------------------------------------------------
static void Cam_SetFreeStanding(camera_t *cam, const vec3_t pos, const vec3_t dir)
{
  vec3_t    up;

  assert(cam->gent);

  cam->hooked_gent = NULL;

  vec3Copy(pos, cam->gent->sv->position);
  vec3Copy(dir, cam->gent->sv->direction);
  BuildAxisByDir(cam->gent->sv->right, up, cam->gent->sv->direction);
  VectorsToAngles(cam->gent->sv->right, up, cam->gent->sv->direction, cam->gent->sv->angles);
} // Cam_SetFreeStanding


//----------------------------------------------------------
// Cam_RunFrame
//----------------------------------------------------------
static void Cam_RunFrame(camera_t *cam)
{
  // if camera is hooked to an entity...
  if(cam->hooked_gent != NULL)
  {
    // if hooked entity is alive move camera to it; otherwise release camera
    if(cam->hooked_gent->timestamp <= cam->hooktime)
    {
      vec3Copy(cam->hooked_gent->sv->position,  cam->gent->sv->position);
      vec3Copy(cam->hooked_gent->sv->direction, cam->gent->sv->direction);
      vec3Copy(cam->hooked_gent->sv->right,     cam->gent->sv->right);
      vec3Copy(cam->hooked_gent->sv->angles,    cam->gent->sv->angles);
    }
    else
    {
      cam->hooked_gent = NULL;
    }
  }
} // Cam_RunFrame


//---------------------------------------------------------------------------


//----------------------------------------------------------
// ChangeToWeapon
//----------------------------------------------------------
static void ChangeToWeapon(client_t *cl, weapontype_t weapon)
{
  assert(weapon >= 0 && weapon < NUM_WEAPONS);

  cl->player->curweapon = weapon;
  cl->player->curmuzzlenum = 0;
  HUD_SetWeaponIcon(&cl->hud, weapon);
} // ChangeToWeapon


//----------------------------------------------------------
// RequestShoot - shoot if client's weapon is ready
//----------------------------------------------------------
static void RequestShoot(client_t *cl)
{
  float shotperiod;     // of current weapon

  assert(cl);
  assert(cl->player);
  assert(cl->player->curweapon >= 0);
  assert(cl->player->curweapon < NUM_WEAPONS);

  // if weapon is ready shoot
  shotperiod = Weapon_GetShotPeriod(cl->player->curweapon);
  if(*e.gametime >= cl->lastshoottime + shotperiod)
  {
    Weapon_Fire(cl->player, ZEROVEC, cl->player->sv->direction);
    cl->lastshoottime = *e.gametime;
    cl->player->curmuzzlenum = (cl->player->curmuzzlenum + 1) % NUM_MUZZLES;

    // recoil weapon model
    Weaponmodel_Recoil(cl->weaponmodel, 0.3f, 5.0f);
  }
} // RequestShoot


//---------------------------------------------------------------------------


#define HUD_TEXTCLASS   TEXT_TECH
#define HUD_TEXTSIZE    20



//---------------------------------
// HUD_Create
//---------------------------------
static int HUD_Create(hud_t *hud)
{
  int   i;

  // attempt to spawn score text
  if((hud->scoretext = EM_SpawnText(HUD_TEXTCLASS)))
  {
    hud->scoretext->sv->position[0] = VW / 2;
    hud->scoretext->sv->position[1] = 96;
    E_Text_SetSize(hud->scoretext, HUD_TEXTSIZE);
    E_Text_Sprintf(hud->scoretext, s_strfmt_score, s_client.player->n.player.score);
  }
  else
  {
    e.conprintf("# HUD_Create: could not spawn score text\n");
    return 0;
  }

  // attempt to spawn timer text
  if((hud->timetext = EM_SpawnText(HUD_TEXTCLASS)))
  {
    hud->timetext->sv->position[0] = VW / 2;
    hud->timetext->sv->position[1] = 36;
    E_Text_SetSize(hud->timetext, HUD_TEXTSIZE);
    E_Text_Sprintf(hud->timetext, "\0");
  }
  else
  {
    e.conprintf("# HUD_Create: could not spawn timer text\n");
    return 0;
  }

  // attempt to spawn time-up warner
  if((hud->timeup_warner = EM_SpawnText(HUD_TEXTCLASS)))
  {
    // centre of screen
    hud->timeup_warner->sv->position[0] = VW / 2;
    hud->timeup_warner->sv->position[1] = VH / 2;
    E_Text_SetSize     (hud->timeup_warner, TIMEUPWARNING_INITIAL_SIZE);
    E_Text_EnableShadow(hud->timeup_warner, 1);
    E_Text_Sprintf     (hud->timeup_warner, "\0");

    if(g_xmasrunning)
      vec4Copy(g_color_white, hud->timeup_warner->sv->color) // GNAAA: vec4Copy() is a macro surrounded by { }, so no semicolon may be here!
    else
      vec4Copy(g_color_red,   hud->timeup_warner->sv->color);
  }
  else
  {
    e.conprintf("# HUD_Create: could not spawn time-up warner text\n");
    return 0;
  }

  // attempt to spawn weapon text
  if((hud->wp_text = EM_SpawnText(HUD_TEXTCLASS)))
  {
    // centre of screen
    hud->wp_text->sv->position[0] = VW / 2 - 16;
    hud->wp_text->sv->position[1] = 66;
    E_Text_SetSize     (hud->wp_text, HUD_TEXTSIZE);
    E_Text_EnableShadow(hud->wp_text, 1);
    E_Text_Sprintf     (hud->wp_text, "%s", s_str_weapon);
  }
  else
  {
    e.conprintf("# HUD_Create: could not spawn weapon text\n");
    return 0;
  }

  // attempt to spawn weapon icons
  for(i = 0; i < NUM_WEAPONS; i++)
  {
    if((hud->wp_icons[i] = EM_SpawnImage(Weapon_GetIconClass(i))))
    {
      hud->wp_icons[i]->sv->position[0] = VW / 2 + 42;
      hud->wp_icons[i]->sv->position[1] = 66;
      E_Image_SetDimension(hud->wp_icons[i], 40, 40);
      // suppress simultan drawing
      hud->wp_icons[i]->sv->state = passive_e;
    }
    else
    {
      e.conprintf("# HUD_Create: could not spawn weapon icon imageclass #%i\n", i);
      return 0;
    }
  }

  // attempt to spawn bonusweapon text
  if((hud->bonusweapon_text = EM_SpawnText(HUD_TEXTCLASS)))
  {
    // centre of screen
    hud->bonusweapon_text->sv->position[0] = VW / 2;

    // message will move up and re-size if bonus weapon just received, so,
    // nothing needs to be done here
    //hud->bonusweapon_text->sv->position[1] = VH - 32;
    //hud->bonusweapon_text->sv->t.size = 30;

    E_Text_EnableShadow(hud->bonusweapon_text, 1);
    E_Text_Sprintf     (hud->bonusweapon_text, "\0");
    vec4Copy(g_color_dkorange, hud->bonusweapon_text->sv->color);
  }
  else
  {
    e.conprintf("# HUD_Create: could not spawn bonusweapon text\n");
    return 0;
  }

  return 1;
} // HUD_Create


//---------------------------------
// HUD_Shutdown
//---------------------------------
static void HUD_Shutdown(hud_t *hud)
{
  int   i;

  assert(hud);

  hud->timetext = hud->scoretext = hud->timeup_warner = hud->wp_text = NULL;
  for(i = 0; i < NUM_WEAPONS; i++)
    hud->wp_icons[i] = NULL;
  hud->bonusweapon_text = NULL;
} // HUD_Shutdown


//---------------------------------
// HUD_EnableRender
//---------------------------------
static void HUD_EnableRender(hud_t *hud, int enable)
{
  int   i;

  assert(hud);
  assert(hud->scoretext);
  assert(hud->timetext);
  assert(hud->timeup_warner);
  assert(hud->wp_text);
  for(i = 0; i < NUM_WEAPONS; i++)
    G_Assert(hud->wp_icons[i], G_Stringf("wp_icon #%i is NULL", i));
  assert(hud->bonusweapon_text);

  hud->scoretext->sv->state = enable ? active_e : passive_e;
  hud->timetext->sv->state = enable ? active_e : passive_e;
  hud->timeup_warner->sv->state = enable ? active_e : passive_e;
  hud->wp_text->sv->state = enable ? active_e : passive_e;
  hud->wp_icons[hud->curweapon]->sv->state = enable ? active_e : passive_e;
  hud->bonusweapon_text->sv->state = enable ? active_e : passive_e;
} // HUD_EnableRender


//-----------------------------------------------
// HUD_UpdateItemTime
//-----------------------------------------------
static void HUD_UpdateItemTime(hud_t *hud, const gentity_t *item)
{
  int   itimeleft;

  assert(item);

  itimeleft = (int)floor(I_GetRemainingTime(item));
  if(itimeleft < 0) // stay non-negative for display
    itimeleft = 0;
  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      E_Text_Sprintf(hud->bonusweapon_text, s_strfmt_bonusweapon, itimeleft/60, itimeleft%60);
      break;

    default:    // I forget to update freshly introduced item
      G_Assert(0, E_GetString(item));
      break;
  }
} // HUD_UpdateItemTime


//---------------------------------
// HUD_UpdateLevelTime
//---------------------------------
static void HUD_UpdateLevelTime(hud_t *hud, float timeleft)
{
  int   itimeleft;

  // print remaining level time
  itimeleft = (int)floor(timeleft);
  E_Text_Sprintf(hud->timetext, s_strfmt_time, itimeleft/60, itimeleft%60);

  // update time-up warner if it's running
  if(timeleft <= TIMEUPWARNING_BEGIN)
  {
    // if some seconds remaining, print them, otherwise print "time up"
    if(timeleft > 0.0f)
    {
      float frac = timeleft - (int)timeleft;

      // fraction defines transparency value + size
      hud->timeup_warner->sv->color[3] = frac;
      E_Text_SetSize(hud->timeup_warner, frac * TIMEUPWARNING_INITIAL_SIZE);
      E_Text_Sprintf(hud->timeup_warner, "%i", itimeleft%60);
    }
    else
    {
      hud->timeup_warner->sv->color[3] = 1.0f;
      E_Text_SetSize(hud->timeup_warner, TIMEUPWARNING_INITIAL_SIZE / 3);
      E_Text_Sprintf(hud->timeup_warner, s_str_timeup);
    }
  }
} // HUD_UpdateLevelTime


//---------------------------------
// HUD_UpdateScoreboard
//---------------------------------
static void HUD_UpdateScoreboard(hud_t *hud, int score)
{
  E_Text_Sprintf(hud->scoretext, s_strfmt_score, score);
}


//---------------------------------
// HUD_SetWeaponIcon
//---------------------------------
static void HUD_SetWeaponIcon(hud_t *hud, weapontype_t weapon)
{
  int   i;

  // first deactivate all icons, then re-activate the desired one
  for(i = 0; i < NUM_WEAPONS; i++)
  {
    hud->wp_icons[i]->sv->state = passive_e;
  }
  assert(weapon >= 0 && weapon < NUM_WEAPONS);
  hud->wp_icons[weapon]->sv->state = active_e;
  hud->curweapon = weapon;
} // HUD_SetWeaponIcon


static void BonusWeaponText_Start(gentity_t *gself);
static void BonusWeaponText_Fade(entity_t *svself);

//-------------------------------------------------------
// HUD_OnReceiveItem
//-------------------------------------------------------
static void HUD_OnReceiveItem(hud_t *hud, const gentity_t *item)
{
  assert(item);

  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      BonusWeaponText_Start(hud->bonusweapon_text);
      break;

    default:    // forget to update for freshly introduced item
      G_Assert(0, E_GetString(item));
      break;
  }
} // HUD_OnReceiveItem


//-------------------------------------------------------
// HUD_OnItemExpires
//-------------------------------------------------------
static void HUD_OnItemExpires(hud_t *hud, const gentity_t *item)
{
  assert(item);

  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      hud->bonusweapon_text->sv->think = BonusWeaponText_Fade;
      break;

    default:    // forget to update for freshly introduced item
      G_Assert(0, E_GetString(item));
      break;
  }
} // HUD_OnItemExpires


//---------------------------------------------------------------------------

// NOTE: the text of the bonusweapon text entity is updated by the
//       bonusweapon itself; it also controls when the text entity must fade;
//       after the text entity is totally faded away itself sets its text
//       to an zero-length string.

static void BonusWeaponText_Start(gentity_t *gself);
static void BonusWeaponText_ScrollupAndShrink(entity_t *svself);
static void BonusWeaponText_Fade(entity_t *svself);


// put message in screen centre + set big size; it will move up and
// sizedown
static void BonusWeaponText_Start(gentity_t *gself)
{
  gself->sv->position[1] = VH / 2;
  gself->sv->color[3] = 1.0f;
  gself->sv->t.size = 150;
  gself->sv->think = BonusWeaponText_ScrollupAndShrink;
}


// think()
static void BonusWeaponText_ScrollupAndShrink(entity_t *svself)
{
  float timediff = *e.gametime - *e.lastgametime;

  // check to scroll up
  if(s_client.hud.bonusweapon_text->sv->position[1] < VH - 32)
  {
    if((s_client.hud.bonusweapon_text->sv->position[1] += timediff*180) > VH - 32)
      s_client.hud.bonusweapon_text->sv->position[1] = VH - 32;
  }

  // check to shrink
  if(s_client.hud.bonusweapon_text->sv->t.size > 30)
  {
    if((s_client.hud.bonusweapon_text->sv->t.size -= timediff * 80) < 30)
      s_client.hud.bonusweapon_text->sv->t.size = 30;
  }
}


// used when bonusweapon has expired to make the timer text fade away
static void BonusWeaponText_Fade(entity_t *svself)   // think
{
  float timediff = *e.gametime - *e.lastgametime;

  // fade away
  if((svself->color[3] -= timediff * 0.7f) <= 0)
  {
    svself->t.message[0] = 0;
    svself->think = NULL;
  }
}
