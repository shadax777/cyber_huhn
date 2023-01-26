#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "math3d.h"
#include "types.h"

#define MAX_ENTITIES        1024
#define SCREENMSG_MAXLEN    128

typedef enum
{
    invalid_e,          // ungueltiger entity-typ (darf nicht vorkommen)
    object_e,           // ein dreidimensionales objekt
    screenmsg_e,        // eine textnachricht auf dem bildschirm
    sound_e,            // eine soundquelle
    image_e,            // ein normales bild
    particle_e,         // ein partikel bzw pointsprite
    general_e           // werden von der engine nicht behandelt
} entitytype_t;

typedef enum
{
    active_e,               // ist aktiv und wird verschoben, gedreht, gerendert, etc..
    passive_e,              // wie active_e, wird aber weder gerendert, noch kann es kollidieren
    pending_remove_e        // als "zu loeschen" markiert, wird vorm naechsten frame freigegeben
} entitystate_t;

typedef enum
{
    _invalid_e,         // ungueltiger entity-subtyp (darf nicht vorkommen)
    normal_e,           // normales objekt wie zb ein huhn
    projectile_e,       // ein geschoss
    fixed_e             // fixe sachen wie zb die kanzel oder waffen
} objectsubtype_t;

typedef struct entity_s
{
    int                 gentitynum;     // fuer die ai
    char                className[128]; // nur zu debugzwecken fuern shaddy
    entitytype_t        type;           // art des entity
    entitystate_t       state;          // status des entitys
    float               timeSpawn;      // spawn-zeitpunkt
    float               nextthink;      // naechster zeitpunkt fuer onTimer
    vec3_t              position;       // position, an der sich das entity befindet
    vec3_t              oldPosition;    // position, an der sich das entity im frame vorher befand
    vec3_t              direction;      // bewegungsrichtung (immer auf 1.0f normiert!)

    vec3_t              right;          // CW

    float               speed;          // geschwindigkeit bei der bewegung in "meter"/sec
    vec3_t              angles;         // der aktuelle betrag der drehung. [0]=um x-achse, [1]=um y, ..
    vec3_t              rotSpeed;       // drehgeschw. in grad/sec. [0]=um x, .. (neg = dreht rueckwaerts)
    vec4_t              color;          // r(0),g(1),b(2) und alpha(3) (per default alles = 1.0f)
    float               gamma;          // gammawert fuer dieses entity (1.0f==fullbright, 0.0f=black)
    union
    {
        struct              // fuer model-entities
        {
            objectsubtype_t subtype;        // der genaue typ des models
            unsigned        startFrame;     // erstes frame der animation
            unsigned        endFrame;       // letztes frame der animation
            struct model_s* model;          // das zu zeichnende model (engineinterna!)
            float           timeKeyframe;   // aktueller zeitpunkt innerhalb der animation
            float           animSpeed;      // faktor, 1.0f per default
            float           scale;          // faktor fuer die modelgroesse, 1.0f per default
            float           colRadius;      // radius der kollisionssphere
            bool_t          zbuff;          // rendern modifiziert den zbuffer, true per default
        } m;
        struct              // fuer screenmsg-entities
        {
            char            message[SCREENMSG_MAXLEN+1];    // text der nachricht
            struct font_s*  font;           // der font_t (engineinterna!)
            float           size;           // normal font size
            bool_t          shadow;         // mit schatten versehen?
            bool_t          threeD;         // raeumlich positionieren?
        } t;
        struct              // fuer sound-entities
        {
            struct sound_s* sound;
        } s;
        struct              // fuer image-entities
        {
            float   xsize;                  // ausdehnung in xrichtung
            float   ysize;                  // ausdehnung in yrichtung
            struct  img_s*  img;            // handle fuer texturdaten (engineinterna!)
        } i;
        struct
        {
            float   size;                   // groesse des pointsprites
        } p;
    };
    void    (*think)(struct entity_s *self);
    void    (*onCollision)(struct entity_s *self, struct entity_s *other);
    void    (*onSrvFree)(struct entity_s *self);
} entity_t;

entity_t*   entCreate(const char *classname, entitytype_t type);    // generiert ein entity
void        entMarkPending(entity_t *entity);   // markiert ein entity zum entfernen
void        entFreePendings(void);              // entfernt alle markieren entities
unsigned    entGetCount(void);                  // liefert die anzahl der entities
entity_t*   entGetFirst(void);                  // liefert das erste benutze entity (oder NULL)
entity_t*   entGetNext(void);                   // liefert nachfolgende entities (oder NULL);
void        entSaveIterator(void);              // sicher den mom. iteratorzustand (nicht kaskadierbar!)
void        entRestoreIterator(void);

#endif
