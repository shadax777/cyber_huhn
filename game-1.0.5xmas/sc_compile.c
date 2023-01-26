/* sc_compile.c - script parser + compiler

- single entities are directly saved in memory
- groups of entities are resolved and their entities are then singly saved in
  memory

** NOTE: all components of a group must come BEFORE its entities, otherwise,
         the group cannot inherit all of its components and parse errors
         may occur.

- a group can't be defined inside another group or inside an entity block

*/

#ifdef SC_COMPILER  // as opposed to SC_GAME

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

#include "script.h"
#include "sc_tok.h"
#include "g_shared.h"
#include "math3d.h"


#define NEXT_EQ(out, test)  ((out = Tok_NextToken()) && !strcmp(out, test))

#define NO_RESPAWNPERIOD    (-1.0f)


// name/value pair
typedef struct
{
  const char    *name, *value;
} namval_t;


// in-memory representation of an entity group
typedef struct
{
  float     spawntime;
  float     respawnperiod;
  vec3_t    pos;
  vec3_t    dir;
  vec3_t    _aim_;  // only intermediate memory storage; not saved
  float     speed;
} group_t;


// value types of a block-item (needed by ParseValue())
enum
{
  VAL_INVALID,
  VAL_VEC3,
  VAL_FLOAT,
  VAL_STRING
};


// item modifiers
enum
{
  MOD_REQUIRED,
  MOD_OPTIONAL,
  MOD_XOR
};


// item name in block
typedef struct
{
  const char    *name;
  int           valuetype;
  int           ofs;    // offset in generic structure
  int           mod;    // modifier: required / optional / xor
  const char    *xor_itemname; // if mod == MOD_XOR this is the according xor item
                            // note:this must not be NULL if mod == MOD_XOR
} item_t;


// parse state of a block instance: offsets of level_t/scentity_t/group_t
// members hold 0/1 values, depending if the member has been parsed.
typedef byte_t  level_state_t[sizeof(level_t)];
typedef byte_t  scent_state_t[sizeof(scentity_t)];
typedef byte_t  group_state_t[sizeof(group_t)];


//---------------------------------------------------------------

static scentity_t   *s_scentities = NULL;   // array of entities in memory
static int          s_numscentities = 0;

static const item_t s_levelitems[] =
{
  { "$title",       VAL_STRING, offsetof(level_t, title),       MOD_OPTIONAL },
  { "$bgm",         VAL_STRING, offsetof(level_t, bgm),         MOD_OPTIONAL },
  { "$duration",    VAL_FLOAT,  offsetof(level_t, duration),    MOD_REQUIRED },
  { "$sky",         VAL_STRING, offsetof(level_t, sky),         MOD_REQUIRED },
  { NULL }  // sentinel
};

static const item_t s_scentitems[] =
{
  { "$spawntime",     VAL_FLOAT,  offsetof(scentity_t, spawntime), MOD_REQUIRED },
  { "$respawnperiod", VAL_FLOAT,  offsetof(scentity_t, respawnperiod), MOD_OPTIONAL },
  { "$name",          VAL_STRING, offsetof(scentity_t, name),      MOD_REQUIRED },
  { "$pos",           VAL_VEC3,   offsetof(scentity_t, pos),       MOD_REQUIRED },
  { "$dir",           VAL_VEC3,   offsetof(scentity_t, dir),       MOD_XOR, "$aim" },
  { "$aim",           VAL_VEC3,   offsetof(scentity_t, _aim_),     MOD_XOR, "$dir" },
  { "$speed",         VAL_FLOAT,  offsetof(scentity_t, speed),     MOD_OPTIONAL },
  { NULL }  // sentinel
};

static const item_t s_groupitems[] =
{
  { "$spawntime",     VAL_FLOAT, offsetof(group_t, spawntime), MOD_REQUIRED },
  { "$respawnperiod", VAL_FLOAT, offsetof(group_t, respawnperiod), MOD_OPTIONAL },
  { "$pos",           VAL_VEC3,  offsetof(group_t, pos),       MOD_REQUIRED },
  { "$dir",           VAL_VEC3,  offsetof(group_t, dir),       MOD_XOR, "$aim" },
  { "$aim",           VAL_VEC3,  offsetof(group_t, _aim_),     MOD_XOR, "$dir" },
  { "$speed",         VAL_FLOAT, offsetof(group_t, speed),     MOD_OPTIONAL },
  { NULL }  // sentinel
};


//-----------------------------------------------------------
// Item_FindByName - search for "name" in "items" and return the item or NULL
//                 - "items[]" is expected to hold a NULL'ed sentinel
//-----------------------------------------------------------
static const item_t *Item_FindByName(const char *name, const item_t items[])
{
  int   i;

  for(i = 0; items[i].name; i++)
  {
    if(!strcmp(items[i].name, name))
      return items + i;
  }
  return NULL;
} // Item_Find


//---------------------------------
// ParseError - print parser error and exit()
//---------------------------------
static void ParseError(const char *fmt, ...)
{
  va_list   ap;
  char      text[1024];

  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  fprintf(stderr, "*** PARSE ERROR: %s [compilation aborted]\n", text);
  exit(EXIT_FAILURE);
} // ParseError


//--------------------------
// Warning
//--------------------------
static void Warning(const char *fmt, ...)
{
  va_list   ap;
  char      text[1024];

  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  fprintf(stderr, "WARNING: %s\n", text);
} // Warning


//-----------------------------------------------------
// ParseValue - parse 'value' and write its value according to the given type
//              into 'out'
//            - Note: the 'value' param is assumed to hold citation marks
//-----------------------------------------------------
static void ParseValue(void *out, int valuetype, const char *value)
{
  vec3_t    *vec3val;
  float     *floatval;
  char      *strval, *c;
  int       n;

  switch(valuetype)
  {
    case VAL_VEC3:
      vec3val = (vec3_t *)out;
      n = sscanf(value, "\"%f%f%f\"", &(*vec3val)[0], &(*vec3val)[1], &(*vec3val)[2]);
      if(n != 3)
        ParseError("line %i: incomplete vector", Tok_CurLineNum());
      break;

    case VAL_FLOAT:
      floatval = (float *)out;
      n = sscanf(value, "\"%f\"", floatval);
      if(n != 1)
        ParseError("line %i: could not read float value", Tok_CurLineNum());
      break;

    case VAL_STRING:
      strval = (char *)out;
      strncpy(strval, value + 1, SC_MAXSTRING); // +1 to skip leading '"'
      strval[SC_MAXSTRING] = '\0';
      // remove the possibly trailing '"'
      if(*(c = &strval[strlen(strval) - 1]) == '"' && c >= strval)
        *c = '\0';
      break;

    default:
      assert(0);    // cannot happen
  }
} // ParseValue


//===========================================================================


//---------------------------------
// Level_ParseNamval
//---------------------------------
static void Level_ParseNamval(level_t           *level,
                              const namval_t    *nv,
                              level_state_t     lstate)
{
  const item_t  *item;

  for(item = s_levelitems; item->name; item++)
  {
    if(!strcmp(nv->name, item->name))
    {
      ParseValue((byte_t *)level + item->ofs, item->valuetype, nv->value);
      lstate[item->ofs] = 1;
      return;
    }
  }
  ParseError("line %i: unknown level item '%s'\n", Tok_CurLineNum(), nv->name);
} // Level_ParseNamval


//--------------------------------------
// Level_VerifyParseState - make sure all required fields of a level block
//                          are present; otherwise abort the program
//--------------------------------------
static void Level_VerifyParseState(level_state_t lstate)
{
  const item_t  *item;

  for(item = s_levelitems; item->name; item++)
  {
    if(!lstate[item->ofs])
    {
      if(item->mod == MOD_REQUIRED)
      {
        ParseError("near line %i: level '%s' missing!", Tok_CurLineNum(), item->name);
      }
      else if(item->mod == MOD_OPTIONAL)
      {
        Warning("near line %i: optional '%s' of level missing", Tok_CurLineNum(), item->name);
      }
      else  // MOD_XOR
      {
        // find the according xor-item and make sure it is set

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_levelitems);
        assert(xor_item);
        if(!lstate[xor_item->ofs])
        {
          ParseError("near line %i: level: neither '%s' nor '%s' present!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
    else    // if(!lstate[item->ofs]) ...
    {
      if(item->mod == MOD_XOR)
      {
        // make sure only one of the xor items is defined

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_levelitems);
        assert(xor_item);
        if(lstate[xor_item->ofs])
        {
          ParseError("near line %i: level: only '%s' OR '%s' can be set, but not both!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
  }
} // Level_VerifyParseState


//---------------------------------------------------------------------------


//---------------------------------
// ScEnt_ParseNamval
//---------------------------------
static void ScEnt_ParseNamval(scentity_t *outscent, const namval_t *nv,
                              scent_state_t estate)
{
  const item_t  *item;

  for(item = s_scentitems; item->name; item++)
  {
    if(!strcmp(nv->name, item->name))
    {
      ParseValue((byte_t *)outscent + item->ofs, item->valuetype, nv->value);
      estate[item->ofs] = 1;
      return;
    }
  }
  ParseError("line %i: unknown entity item '%s'\n", Tok_CurLineNum(), nv->name);
} // ScEnt_ParseNamval



//--------------------------------------
// ScEnt_InheritFromGroup
//--------------------------------------
static void ScEnt_InheritFromGroup(scentity_t *scent, const group_t *g,
                    scent_state_t estate, const group_state_t gstate)
{
  vec3_t        right, up, localpos;
  const item_t  *gitem, *entitem;
  int           i;

  assert(gstate);

  scent->spawntime += g->spawntime;
  estate[offsetof(scentity_t, spawntime)] = 1;

  if(scent->respawnperiod <= 0.0)
  {
    scent->respawnperiod = g->respawnperiod;
    estate[offsetof(scentity_t, respawnperiod)] = 1;
  }

  // build local coordinate system of group;
  // (it's guaranteed that "dir" is set - at least through "aim")
  BuildAxisByDir(right, up, g->dir);

  //
  // set entity position in group
  //
  vec3Copy(scent->pos, localpos);
  vec3Copy(g->pos, scent->pos);
  for(i = 0; i < 3; i++)
  {
    scent->pos[i] += right[i] * localpos[0];
    scent->pos[i] += up[i] * localpos[1];
    scent->pos[i] += g->dir[i] * localpos[2];
  }
  estate[offsetof(scentity_t, pos)] = 1;


  // check if group's "dir" or "aim" is provided
  gitem = Item_FindByName("$dir", s_groupitems);
  assert(gitem);
  if(gstate[gitem->ofs])
  {
    /* inheritance of group's "dir"

    we consider only 1 case: is entity's "dir" set or not.
    - if it's not set, copy group's "dir"
    - if it is set, set it relative to group's "dir"

    There are other possibilities concerning "dir", but they don't need
    consideration here:
    - entity's "aim" is set => ParseEntity() will set "dir"
    - "dir" and "aim" are set => ScEnt_VerifyParseState() will catch the error
    */

    entitem = Item_FindByName("$dir", s_scentitems);
    assert(entitem);
    if(!estate[entitem->ofs])
    {
      vec3Copy(g->dir, scent->dir);
    }
    else
    {
      vec3_t  localdir, right, up;

      // set entity's "dir" relative to group's "dir"
      vec3Copy(scent->dir, localdir);
      vec3Clear(scent->dir);
      BuildAxisByDir(right, up, g->dir);
      for(i = 0; i < 3; i++)
      {
        scent->dir[i] += right[i] * localdir[0];
        scent->dir[i] += up[i] * localdir[1];
        scent->dir[i] += g->dir[i] * localdir[2];
      }
    }
  }
  else  // if(gstate[gitem->ofs]) // "$dir"
  {
    // group's "aim" is set; "dir" has been derived from it

    entitem = Item_FindByName("$dir", s_scentitems);
    assert(entitem);

    // if entity's "dir" is not set, derive it from group's "aim"
    if(!estate[entitem->ofs])
    {
      vec3Sub(g->_aim_, scent->pos, scent->dir);
      vec3Normalize(scent->dir);
    }
    else
    {
      // adjust entity's "dir" according to group's "aim"

      vec3_t    localdir, intermdir, right, up;

      // get an intermediate "dir" of entity before we set it relative
      // to group's "dir"
      vec3Sub(g->_aim_, scent->pos, intermdir);
      vec3Normalize(intermdir);
      BuildAxisByDir(right, up, intermdir);
      vec3Copy(scent->dir, localdir);
      vec3Clear(scent->dir);
      for(i = 0; i < 3; i++)
      {
        scent->dir[i] += right[i] * localdir[0];
        scent->dir[i] += up[i] * localdir[1];
        scent->dir[i] += intermdir[i] * localdir[2];
      }
    }
  }
  estate[offsetof(scentity_t, dir)] = 1;

  scent->speed += g->speed;
  estate[offsetof(scentity_t, speed)] = 1;
} // ScEnt_InheritFromGroup


//--------------------------------------
// ScEnt_VerifyParseState - make sure all required fields of an entity block
//                          are present; otherwise abort the program
//--------------------------------------
static void ScEnt_VerifyParseState(scent_state_t estate)
{
  const item_t  *item;

  for(item = s_scentitems; item->name; item++)
  {
    if(!estate[item->ofs])
    {
      if(item->mod == MOD_REQUIRED)
      {
        ParseError("near line %i: entity '%s' missing!", Tok_CurLineNum(), item->name);
      }
      else if(item->mod == MOD_OPTIONAL)
      {
        Warning("near line %i: optional '%s' of entity missing", Tok_CurLineNum(), item->name);
      }
      else  // MOD_XOR
      {
        // find the according xor-item and make sure it is set

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_scentitems);
        assert(xor_item);
        if(!estate[xor_item->ofs])
        {
          ParseError("near line %i: entity: neither '%s' nor '%s' present!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
    else    // state is set
    {
      if(item->mod == MOD_XOR)
      {
        // make sure only one of the xor items is defined

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_scentitems);
        assert(xor_item);
        if(estate[xor_item->ofs])
        {
          ParseError("near line %i: entity: only '%s' OR '%s' can be set, but not both!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
  }
} // ScEnt_VerifyParseState


//---------------------------------------
// ScEnt_CopyToMem - push given entity block to others in memory
//---------------------------------------
static void ScEnt_CopyToMem(const scentity_t *scent)
{
  scentity_t    *newscentities;

  newscentities = (scentity_t *)realloc(s_scentities, (s_numscentities+1)*sizeof(scentity_t));
  if(!(newscentities))
  {
    fprintf(stderr, "ScEnt_CopyToMem: out of mem\n");
    free(s_scentities);
    exit(EXIT_FAILURE);
  }
  s_scentities = newscentities;
  memcpy(&s_scentities[s_numscentities++], scent, sizeof(scentity_t));
} // ScEnt_CopyToMem


//---------------------------------------------------------------------------


//--------------------------------------
// G_ParseNamval
//--------------------------------------
static void G_ParseNamval(group_t           *g,
                          const namval_t    *nv,
                          group_state_t     gstate)
{
  const item_t  *item;

  for(item = s_groupitems; item->name; item++)
  {
    if(!strcmp(nv->name, item->name))
    {
      ParseValue((byte_t *)g + item->ofs, item->valuetype, nv->value);
      gstate[item->ofs] = 1;
      return;
    }
  }
  ParseError("line %i: unknown group item '%s'", Tok_CurLineNum(), nv->name);
} // G_ParseNamval


//--------------------------------------
// G_VerifyParseState - make sure all required fields of a group
//                      are present; otherwise abort the program
//--------------------------------------
static void G_VerifyParseState(group_state_t gstate)
{
  const item_t  *item;

  // print warning for items which were expected to be parsed
  for(item = s_groupitems; item->name; item++)
  {
    if(!gstate[item->ofs])
    {
      if(item->mod == MOD_REQUIRED)
      {
        ParseError("near line %i: group '%s' missing!", Tok_CurLineNum(), item->name);
      }
      else if(item->mod == MOD_OPTIONAL)
      {
        Warning("near line %i: optional '%s' of group missing", Tok_CurLineNum(), item->name);
      }
      else  // MOD_XOR
      {
        // find the according xor-item and make sure it is set

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_groupitems);
        assert(xor_item);
        if(!gstate[xor_item->ofs])
        {
          ParseError("near line %i: group: neither '%s' nor '%s' present!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
    else    // state is set
    {
      if(item->mod == MOD_XOR)
      {
        // make sure only one of the xor items is defined

        const item_t    *xor_item;

        assert(item->xor_itemname);
        xor_item = Item_FindByName(item->xor_itemname, s_groupitems);
        assert(xor_item);
        if(gstate[xor_item->ofs])
        {
          ParseError("near line %i: group: only '%s' OR '%s' can be set, but not both!", Tok_CurLineNum(), item->name, item->xor_itemname);
        }
      }
    }
  }
} // G_VerifyParseState


//---------------------------------------------------------------------------


//----------------------------------
// ParseLevel
//----------------------------------
static void ParseLevel(level_t *outlevel)
{
  const char    *tok;
  namval_t      nv;
  level_state_t lstate;

  memset(outlevel, 0, sizeof(level_t));
  memset(lstate, 0, sizeof lstate);

  // expect block begin
  if(!NEXT_EQ(tok, "{"))
  {
    if(!tok) ParseError("near line %i: expected '{', but reached EOF", Tok_CurLineNum());
    else     ParseError("line %i: expected '{', got '%s'", Tok_CurLineNum(), tok);
  }

  // read + parse level components until block end
  while(1)
  {
    // check unexpected EOF
    if(!(tok = Tok_NextToken()))
      ParseError("reached EOF without closing brace");

    // check block end
    if(!strcmp(tok, "}"))
    {
      // make sure, all required fields were supplied
      Level_VerifyParseState(lstate);
      break;
    }

    // item name
    nv.name = tok;

    // expect assignment
    if(!NEXT_EQ(tok, "="))
    {
      if(!tok) ParseError("near line %i: expected '=', but reached EOF", Tok_CurLineNum());
      else     ParseError("line %i: expected '=', got '%s'", Tok_CurLineNum(), tok);
    }

    // get item value
    if(!(nv.value = Tok_NextToken()))
      ParseError("line %i: expected item value, but reached EOF", Tok_CurLineNum());
    Level_ParseNamval(outlevel, &nv, lstate);
  }
} // ParseLevel


//----------------------------------
// ParseEntity - read entity components; if a super group is
//               given, first inherit the group's components
//----------------------------------
static void ParseEntity(const group_t *super, const group_state_t gstate)
{
  const char    *tok;   // current token
  namval_t      nv;
  scentity_t    scent;
  scent_state_t estate;
  const item_t  *item;

  memset(&scent, 0, sizeof scent);
  scent.respawnperiod = NO_RESPAWNPERIOD;
  memset(estate, 0, sizeof estate);

  // expect block begin
  if(!NEXT_EQ(tok, "{"))
  {
    if(!tok) ParseError("unexpected EOF");
    else     ParseError("line %i: expected '{', got '%s'", Tok_CurLineNum(), tok);
  }

  // read + parse entity components until block end
  while(1)
  {
    // check unexpected EOF
    if(!(tok = Tok_NextToken()))
      ParseError("reached EOF without closing brace");

    // check block end
    if(!strcmp(tok, "}"))
    {
      const classinfo_t *ci;

      // if the entity is part of a group...
      if(super)
      {
        //CW 2002-08-26
        vec3Normalize(scent.dir);
        ScEnt_InheritFromGroup(&scent, super, estate, gstate);
      }

      // make sure, all required fields were supplied
      ScEnt_VerifyParseState(estate);

      // make sure spawntime is non-negative
      if(scent.spawntime < 0.0f)
      {
        ParseError("entity near line %i: spawntime must not be negative (%f)", Tok_CurLineNum(), scent.spawntime);
      }

      // check if entity is known at all
      if((ci = G_GetClassInfoByName(scent.name)) == NULL)
      {
        ParseError("unrecognized entity near line %i: '%s'\n", Tok_CurLineNum(), scent.name);
      }

      // check if entity is scriptable
      if(!ci->is_scriptable)
      {
        ParseError("entity near line %i is not scriptable: '%s'\n", Tok_CurLineNum(), scent.name);
      }

      // if "dir" is not set, derive it from "aim"
      item = Item_FindByName("$dir", s_scentitems);
      assert(item);
      if(!estate[item->ofs])
      {
        vec3Sub(scent._aim_, scent.pos, scent.dir);
      }
      vec3Normalize(scent.dir);
      if(!vec3Len(scent.dir))
        ParseError("near line %i: entity's direction vector is zero", Tok_CurLineNum());

      // 2002-09-10: make sure the entity's "speed" is non-negative
      if(scent.speed < 0.0f)
      {
        ParseError("near line %i: entity's speed must not be negative", Tok_CurLineNum());
      }

      ScEnt_CopyToMem(&scent);
      break;
    }

    // item name
    nv.name = tok;

    // expect assignment
    if(!NEXT_EQ(tok, "="))
    {
      if(!tok) ParseError("unexpected EOF");
      else     ParseError("line %i: expected '=', got '%s'", Tok_CurLineNum(), tok);
    }

    // get item value
    if(!(nv.value = Tok_NextToken()))
      ParseError("unexpected EOF");

    ScEnt_ParseNamval(&scent, &nv, estate);
  }
} // ParseEntity


//------------------------------------------------------------------
// ParseGroup - read components of group; inherit the red
//              components to each of the group's entitis
//------------------------------------------------------------------
static void ParseGroup(void)
{
  group_t       g;
  const char    *tok;
  namval_t      nv;
  group_state_t gstate;
  const item_t  *item;
  int           veryfied = 0;   // to not get multiple warnings in entity blocks

  memset(&g, 0, sizeof(group_t));
  g.respawnperiod = NO_RESPAWNPERIOD;
  memset(gstate, 0, sizeof gstate);

  // expect block begin
  if(!NEXT_EQ(tok, "{"))
  {
    if(!tok) ParseError("near line %i: expected '{', but reached EOF", Tok_CurLineNum());
    else     ParseError("line %i: expected '{', got '%s'", Tok_CurLineNum(), tok);
  }

  // read + parse the group's components and look for entity blocks
  while(1)
  {
    // check for block end
    if(NEXT_EQ(tok, "}"))
    {
      break;
    }
    else if(!tok)
      ParseError("unexpected EOF");

    // check for start of entity block
    if(!strcmp(tok, "$entity"))
    {
      if(!veryfied)
      {
        G_VerifyParseState(gstate);
        veryfied = 1;
      }

      // make sure spawntime is non-negative
      if(g.spawntime < 0.0f)
      {
        ParseError("group near line %i: spawntime must not be negative (%f)", Tok_CurLineNum(), g.spawntime);
      }

      // if "dir" is not set, derive it from "aim"
      item = Item_FindByName("$dir", s_groupitems);
      assert(item);
      if(!gstate[item->ofs])
      {
        vec3Sub(g._aim_, g.pos, g.dir);
      }
      vec3Normalize(g.dir);
      if(!vec3Len(g.dir))
        ParseError("near line %i: group's direction vector is zero", Tok_CurLineNum());

      ParseEntity(&g, gstate);
      continue;
    }

    // expect identifier
    if(tok[0] != '$')
      ParseError("line %i: expected identifier, got '%s'", Tok_CurLineNum(), tok);

    // item name
    nv.name = tok;

    // expect assignment
    if(!NEXT_EQ(tok, "="))
    {
      if(!tok) ParseError("near line %i: expected '=', but reached EOF", Tok_CurLineNum());
      else     ParseError("line %i: expected '=', got '%s'", Tok_CurLineNum(), tok);
    }

    // item value
    if(!(nv.value = Tok_NextToken()))
      ParseError("unexpected EOF");

    G_ParseNamval(&g, &nv, gstate);
  }
} // ParseGroup


//===========================================================================


//-----------------------------------
// SC_CompileFile - main function to parse + compile a source file and
//                  output a binary file
//-----------------------------------
void SC_CompileFile(const char *inpath, const char *outdir, int verbose)
{
  const char    *tok;
  char          outfilename[64], outpath[256], *c;
  const char    *cc;
  FILE          *f;
  int           i;
  level_t       level;
  int           level_parsed = 0;

  if(!Tok_LoadFile(inpath))
  {
    // Tok_LoadFile() already notified of the error
    return;
  }

  while((tok = Tok_NextToken()))
  {
    if(!strcmp(tok, "$level"))
    {
      if(level_parsed)
        ParseError("line %i: encountered more than 1 level block", Tok_CurLineNum());
      ParseLevel(&level);
      level_parsed = 1;
    }
    else if(!strcmp(tok, "$group"))
    {
      ParseGroup();
    }
    else if(!strcmp(tok, "$entity"))
    {
      ParseEntity(NULL, NULL);    // no super group, no group_state_t
    }
    else
    {
      ParseError("line %i: unknown block: '%s'", Tok_CurLineNum(), tok);
    }
  }
  // free the whole token parse schmodder
  Tok_Free();

  if(!level_parsed)
    ParseError("no $level block defined!");

  // create output filename with leading path (if one is given)
  if(outdir != NULL && strlen(outdir) > 0)
  {
    char    *c;

    strcpy(outpath, outdir);
    c = &outpath[strlen(outpath) - 1];
    if(*c != '\\' && *c != '/')
      strcat(c + 1, "/");
  }
  else
  {
    outpath[0] = 0;
  }

#if 0 // bug if given input filename contains path
  strcat(outpath, inpath);
  if((c = strrchr(outpath, '.')))
    *c = '\0';
  strcat(outpath, ".bin");
#else
  //
  // grab filename from input path and build output path
  //
  /*

  ../scripts/script0.txt

  ./script.txt

  script0.txt

  /script0.txt

  */

  // walk from back to front until first path separator is encountered or
  // we reached beginning of input path and grab the filename
  for(cc = &inpath[strlen(inpath)-1]; cc>inpath && *cc!='/' && *cc!='\\'; cc--)
    ;
  if(*cc == '/' || *cc == '\\')
    cc++;
  strcpy(outfilename, cc);

  // build output filename with ".bin"
  if((c = strrchr(outfilename, '.')))
    *c = '\0';
  strcat(outfilename, ".bin");

  // build final output path
  strcat(outpath, outfilename);
#endif

  // write level block and all entity blocks to the binary file
  if((f = fopen(outpath, "wb")))
  {
    if(verbose)
    {
      printf("======= level =======\n");
      Level_Print(&level);
      printf("\n");
    }

    // output filename is required to build CRC
    if(!Level_WriteFile(&level, outfilename, f))
    {
      fprintf(stderr, "could not write level data: %s\n", strerror(errno));
      fclose(f);
      exit(EXIT_FAILURE);
    }

    for(i = 0; i < s_numscentities; i++)
    {
      if(verbose)
      {
        printf("***** entity **************\n");
        ScEnt_Print(&s_scentities[i]);
      }

      if(!ScEnt_WriteFile(&s_scentities[i], f))
      {
        fprintf(stderr, "could not write entity data: %s\n", strerror(errno));
        break;
      }
    }
    fclose(f);
  }
  else
  {
    fprintf(stderr, "could not write file '%s': %s\n", outpath,
            strerror(errno));
  }
} // SC_CompileFile


#endif  /* SC_COMPILER */
