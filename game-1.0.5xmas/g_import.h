// also included by the engine!

#ifndef __G_IMPORT_H
#define __G_IMPORT_H


// engine headers
#include "types.h"
#include "entity.h"
#include "math3d.h"


struct sound_s;


// stuff, that the engine provides for the AI module
typedef struct
{
  const float   *gametime;      // elapsed time since level was started
                                // engine should reset this when a new level is
                                // starting
  const float   *lastgametime;  // elapsed time until last frame

  entity_t      *(*__spawnentity)(const char *classname, entitytype_t type);
  void          (*__pendremove)(entity_t *ent);

  void          (*conprintf)(const char *fmt, ...); // printf would be just fine
  void          (*shutdown)(void);  // just in case I coded shit and I need to exit now

  myhandle_t    (*sfx_cachefile)(const char *filename);
  void          (*sfx_play)(entity_t *ent, int looped);
  void          (*sfx_updatelistener3d)(const entity_t *listener);
  void          (*sfx_setsoundvolume)(struct sound_s *sound, float vol);
  void          (*sfx_setmastervolume)(float vol);
  void          (*sfx_pauseall)(void);
  void          (*sfx_resumeall)(void);

  void          (*gametime_pause)(void);
  void          (*gametime_resume)(void);

  const char	*(*lang_getlocalizedstr)(unsigned id);
} game_import_t;


// opcode for Game_Main()
enum
{
  GAME_INIT,
  // 2002-09-16: removed this:
  //GAME_SHUTDOWN,  // no longer used, since the -game- decides when to quit

  GAME_GET_CAMERA,
  GAME_KEY_EVENT,
  GAME_MOUSE_EVENT,
  GAME_FRAME,

  // 2002-09-16: added this:
  GAME_GET_VERSION,

  // 2002-09-21: added this:
  GAME_PARTICLE_PERCENTAGE,

  // 2002-10-06: added this:
  GAME_MOUSE_INVERT,

  // 2002-10-12 (FPS)
  GAME_FRAME_END
};


/*

 This is the official specification for opcodes called via
   const void *Game_Main(int opcode, int arg0, int arg1, int arg2) :

 opcode             arg0            arg1           arg2   return value
 ---------------------------------------------------------------------

 GAME_INIT          levelnum(0..5)                 -      NULL = OK; (void *)1 = error

// 2002-09-16: removed this:
// GAME_SHUTDOWN      -               -              -      NULL


 GAME_GET_CAMERA    -               -              -      (const entity_t *)
 // +-FPS
 GAME_KEY_EVENT     subtype(1)      key_t          -      NULL
 GAME_MOUSE_EVENT   subtype(2)      dx(3)          dx(4)  NULL
 GAME_FRAME         -               -              -      NULL

// 2002-09-16: added this:
 GAME_GET_VERSION   -               -              -      (const char *)

// 2002-09-21: added this:
 GAME_PARTICLE_PERCENTAGE  0..100   -              -      NULL

// 2002-10-06: added this:
 GAME_MOUSE_INVERT         0, 1     -              -      NULL

NOTES:

    - GAME_INIT: arg0 shall be the level number to be loaded. Level numbers
        range from 0..5. If a level could not be loaded, the game will
        exit() everything and give notice in the logfile ('chicken.log')
        (but only if the console is enabled, of course).

    - GAME_SHUTDOWN: causes the game code to free acquired resources.

    - GAME_GET_CAMERA: this opcode returns the camera entity, so the engine
        can render the scene according to this entity's coordinates.


    - GAME_KEY_EVENT:

       (1) MSG_CHAR (kann momentan ignoriert werden)

    - GAME_MOUSE_EVENT:

       (2) MSG_LBUTTONDOWN | MSG_LBUTTONUP | MSG_RBUTTONDOWN | MSG_RBUTTONUP |
           MSG_MOUSEWHEEL | MSG_MOUSEMOVE

       (3) fuer (2) == MSG_MOUSEWHEEL -> 100 fuer rad positiv drehen, -100 fuer negativ
             die diskrete schrittweite kann fuer zukuenftige maeuse variieren (zb 10/-10)
           fuer (2) == MSG_MOUSEMOVE -> deltax * 1000
           fuer alle anderen faelle -> undefiniert

       (4) fuer (2) == MSG_MOUSEMOVE -> deltay * 1000
           fuer alle anderen faelle -> undefiniert

    - GAME_FRAME: -always- call this opcode once in the main loop; this
        gives the game code the opportunity to do internal periodic stuff,
        like checking for level time-up, entity world culling, etc.

    - GAME_FRAME_END: -always- call this opcode once in the main loop, right
		after the current frame was completely done (handled and rendered); this
        gives the game code the opportunity to do internal stuff,
        like making screenshots and stuff like this.

    - GAME_PARTICLE_PERCENTAGE: arg0 holds the percentage of particles to
        emit upon explosion, hits, etc. Valid values range from 0..100; other
        values will be ignored and default to 100.

    - GAME_MOUSE_INVERT: this opcode can be called any time
        arg0: 0 == don't invert mouse; 1 == invert mouse;

*/


// fps wanted this
typedef void        *(*ai_import_f)(game_import_t);
typedef const void  *(*ai_main_f)(int, int, int, int);


#endif  /* __G_IMPORT_H */
