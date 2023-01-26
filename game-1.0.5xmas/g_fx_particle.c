// the particle size can be scaled before each call to Part_EmitRadially(),
// but will be reset to its initial size afterwards.
// same counts for the minimal emission radius.


#include "g_local.h"


//-------------------------------------------------------------------
// CheckFadeAndFall - if particle's fade time has come fade it away;
//                  - returns 0 if totally faded away; otherwise 1
//-------------------------------------------------------------------
static int CheckFadeAndFall(gentity_t *gself);


// think()'ers for different shaders
static void ShaderNone_Proceed(entity_t *svself);
static void ShaderFlicker_Proceed(entity_t *svself);
static void ShaderSnowflake_Proceed(entity_t *svself);


// percentage of particles emitted by Part_EmitRadially()
static float    s_particlemultiplier = 1.0f;

#define DEFAULT_PARTSIZE    2.0f
#define DEFAULT_MINRADIUS   0.0f
#define DEFAULT_MINSPEED    20.0f
#define DEFAULT_MAXSPEED    30.0f
#define DEFAULT_FADEFACTOR  1.0f
#define DEFAULT_GRAVITY     0.0f

#define DEFAULT_SNOWFLAKE_MAXTRAVELDISTANCE 0.0f


// parameters for the next particle emission; these can be manipulated
// before an emission occures, but will be reset afterwards
static partshader_t s_curshader = PARTSHADER_NONE;
static void         (*s_OnFree)(gentity_t *gself) = NULL;
static float        s_curparticlesize = DEFAULT_PARTSIZE;
static float        s_curminradius = DEFAULT_MINRADIUS;
static vec3_t       s_curemissionaxes[3];
static int          s_onlypositives[3] = { 0, 0, 0 };
static float        s_curminspeed = DEFAULT_MINSPEED,
                    s_curmaxspeed = DEFAULT_MAXSPEED;
static float        s_curfadefactor = DEFAULT_FADEFACTOR;
static float        s_curgravity = DEFAULT_GRAVITY;
static float        s_curfadetime = 0.0f;

static float        s_snowflake_maxtraveldistance = DEFAULT_SNOWFLAKE_MAXTRAVELDISTANCE;


static int  s_emissionaxes_initialized = 0;


//-------------------------------------------------------------------
// Part_SetPercentage - will stay persistent
//-------------------------------------------------------------------
void Part_SetPercentage(int percent)
{
  if(percent >= 0 && percent <= 100)
    s_particlemultiplier = percent / 100.0f;
}


//-------------------------------------------------------------------
// Part_SetShader
//-------------------------------------------------------------------
void Part_SetShader(partshader_t shader)
{
  if(shader >= 0 && shader < NUM_PARTSHADERS)
    s_curshader = shader;
}


//-------------------------------------------------------------------
// Part_SetOnFree - callback function if the particle gets freed
//-------------------------------------------------------------------
void Part_SetOnFree(void (*OnFree)(gentity_t *gself))
{
  s_OnFree = OnFree;
}


//-------------------------------------------------------------------
// Part_ScaleSize
//-------------------------------------------------------------------
void Part_ScaleSize(float scale)
{
  if(scale >= 0)
    s_curparticlesize *= scale;
}


//-------------------------------------------------------------------
// Part_SetMinRadius
//-------------------------------------------------------------------
void Part_SetMinRadius(float minrad)
{
  if(minrad >= 0)
    s_curminradius = minrad;
}


//-------------------------------------------------------------------
// Part_ClampMoveSpeed - only non-negative values allowed
//-------------------------------------------------------------------
void Part_ClampMoveSpeed(float min, float max)
{
  if((s_curminspeed = min) < 0.0f)
    s_curminspeed = 0.0f;
  if((s_curmaxspeed = max) < s_curminspeed)
    s_curmaxspeed = s_curminspeed;
}


//-------------------------------------------------------------------
// Part_SetEmissionAxes
//-------------------------------------------------------------------
void Part_SetEmissionAxes(/*const*/ vec3_t axes[3])
{
  // FIXME: nested const array types cause (legal) compiler warnings
  AxesCopy(axes, s_curemissionaxes);
  s_emissionaxes_initialized = 1;
}


//-------------------------------------------------------------------
// Part_RestrictToPositiveDirections
// - define if any of the 3 emission axis may emit only along their
//   positive directions
// - default: allow both positive and negative directions
//-------------------------------------------------------------------
void Part_RestrictToPositiveDirections(int along_right, int along_up,
            int along_dir)
{
  s_onlypositives[0] = along_right;
  s_onlypositives[1] = along_up;
  s_onlypositives[2] = along_dir;
}


//-------------------------------------------------------------------
// Part_SetFadeFactor
//-------------------------------------------------------------------
void Part_SetFadeFactor(float factor)
{
  if(factor >= 0.0f)    // 0 allowed to force particle never fade away
    s_curfadefactor = factor;
}


//-------------------------------------------------------------------
// Part_SetGravity
//-------------------------------------------------------------------
void Part_SetGravity(float gravity)
{
  s_curgravity = gravity;
}


//-------------------------------------------------------------------
// Part_SetFadeTime
//-------------------------------------------------------------------
void Part_SetFadeTime(float starttime)
{
  if(starttime >= 0.0f)
    s_curfadetime = starttime;
}


//-------------------------------------------------------------------
// Part_SnowFlake_SetMaxTravelDistance - can ONLY be applied if
//                                       current shader is snowflake!!!
//-------------------------------------------------------------------
void Part_SnowFlake_SetMaxTravelDistance(float dist)
{
  G_Assert(s_curshader == PARTSHADER_SNOWFLAKE, G_Stringf("%i", s_curshader));

  if(dist >= 0.0f)
    s_snowflake_maxtraveldistance = dist;
}


//-------------------------------------------------------------------
// Part_EmitRadially
//-------------------------------------------------------------------
void Part_EmitRadially(const vec3_t org, int n, float maxrad, const vec4_t col)
{
  if(!s_emissionaxes_initialized)
  {
    AxesCopy(AXIS, s_curemissionaxes);
    s_emissionaxes_initialized = 1;
  }

  n = G_Round(n * s_particlemultiplier);
  while(n-- > 0)
  {
    gentity_t   *part;
    int         i;

    if((part = EM_SpawnParticle()))
    {
      float     space;  // variable space between min. + max. radius
      vec3_t    tmpdir;

      // first, set random direction, ignoring emission axis
      for(i = 0; i < 3; i++)
      {
        if(!(tmpdir[i] = G_Random() - 0.5 * (s_onlypositives[i]==0)))
          tmpdir[i] = 0.0001f;
      }

      // now, set true direction along current emission axis
      for(i = 0; i < 3; i++)
      {
        part->sv->direction[i] = s_curemissionaxes[0][i] * tmpdir[0]
                               + s_curemissionaxes[1][i] * tmpdir[1]
                               + s_curemissionaxes[2][i] * tmpdir[2];
      }
      vec3Normalize(part->sv->direction);
      vec4Copy(col, part->sv->color);

      // get space between min + max radius
      if(maxrad < s_curminradius)
        maxrad = s_curminradius;
      space = maxrad - s_curminradius;

      // set position in clamped min/max radius
      space *= G_Random();
      vec3MA(org, part->sv->direction, s_curminradius + space, part->sv->position);
      vec3Copy(part->sv->position, part->n.particle.spawnpos);

      part->sv->p.size = s_curparticlesize;
      part->sv->speed = s_curminspeed + G_Random() * (s_curmaxspeed - s_curminspeed);
      part->OnFree = s_OnFree;
      part->n.particle.shader = s_curshader;
      part->n.particle.fadefactor = s_curfadefactor;
      part->n.particle.gravity = s_curgravity;
      part->n.particle.fadetime = s_curfadetime;

      // set particle's think function according to shader
      switch(s_curshader)
      {
        case PARTSHADER_NONE:
          part->sv->think = ShaderNone_Proceed;
          break;

        case PARTSHADER_FLICKER:
          part->sv->think = ShaderFlicker_Proceed;
          break;

        case PARTSHADER_SNOWFLAKE:
          part->sv->think = ShaderSnowflake_Proceed;
          part->n.particle.n.snowflake.maxtraveldistance = s_snowflake_maxtraveldistance;
          break;

        default:
          G_Assert(0, G_Stringf("Shadax missed a new shader: %i", s_curshader));
          break;
      }
    }
  }

  // restore default particle parameters
  s_curshader = PARTSHADER_NONE;
  s_OnFree = NULL;
  s_curparticlesize = DEFAULT_PARTSIZE;
  s_curminradius = DEFAULT_MINRADIUS;
  s_curminspeed = DEFAULT_MINSPEED;
  s_curmaxspeed = DEFAULT_MAXSPEED;
  AxesCopy(AXIS, s_curemissionaxes);
  s_onlypositives[0] = s_onlypositives[1] = s_onlypositives[2] = 0;
  s_curfadefactor = DEFAULT_FADEFACTOR;
  s_curgravity = DEFAULT_GRAVITY;
  s_curfadetime = 0.0f;

  s_snowflake_maxtraveldistance = DEFAULT_SNOWFLAKE_MAXTRAVELDISTANCE;
} // Part_EmitRadially


//-------------------------------------------------------------------
// CheckFadeAndFall - if particle's fade time has come fade it away;
//                  - returns 0 if totally faded away; otherwise 1
//-------------------------------------------------------------------
static int CheckFadeAndFall(gentity_t *gself)
{
  float timediff = *e.gametime - *e.lastgametime;

  assert(gself);

  // FADING
  // if particle is snowflake we may only consider fading away if
  // travelled far enough
  if(gself->n.particle.shader == PARTSHADER_SNOWFLAKE)
  {
    vec3_t  traveldist;
    float   maxtraveldist = gself->n.particle.n.snowflake.maxtraveldistance;

    // if travelled far enough allow fading away
    vec3Sub(gself->sv->position, gself->n.particle.spawnpos, traveldist);
    if(vec3LenSquared(traveldist) >= maxtraveldist * maxtraveldist)
    {
      // see if time to fade and do so
      if(gself->n.particle.fadetime <= *e.gametime)
      {
        if((gself->sv->color[3] -= 1.5 * timediff * gself->n.particle.fadefactor) < 0)
        {
          gself->sv->color[3] = 0.0f;
          return 0;
        }
      }
    }
  }
  else  // see if fade time for others than snowflake
  {     // FIXME: equal code twice!
    // see if time to fade and do so
    if(gself->n.particle.fadetime <= *e.gametime)
    {
      if((gself->sv->color[3] -= 1.5 * timediff * gself->n.particle.fadefactor) < 0)
      {
        gself->sv->color[3] = 0.0f;
        return 0;
      }
    }
  }

  // applay gravity
  // FIXME: if "direction" is pointing straight up it should be manipulated a bit
  //        otherwise it will always point straight up
  gself->sv->direction[1] -= 1.5 * timediff * gself->n.particle.gravity;
  vec3Normalize(gself->sv->direction);

  return 1;
} // CheckFadeAndFall


//---------------------------------------------------------------------------


//-------------------------------------------------------------------
// ShaderNone_Proceed
//-------------------------------------------------------------------
static void ShaderNone_Proceed(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);

  if(!CheckFadeAndFall(gself))
  {
    EM_Free(gself);
    return;
  }
} // ShaderNone_Proceed


//-------------------------------------------------------------------
// ShaderFlicker_Proceed
//-------------------------------------------------------------------
static void ShaderFlicker_Proceed(entity_t *svself)
{
  gentity_t *gself = G_GEntity(svself);

  if(!CheckFadeAndFall(gself))
  {
    EM_Free(gself);
    return;
  }

  // check if time to flicker
  if(gself->n.particle.n.flicker.nexttime <= *e.gametime)
  {
    gself->sv->state = (gself->sv->state == active_e) ? passive_e : active_e;
    gself->n.particle.n.flicker.nexttime = *e.gametime + G_Random()/20.0f;
  }
} // ShaderFlicker_Proceed


//-------------------------------------------------------------------
// ShaderSnowflake_Proceed
//-------------------------------------------------------------------
static void ShaderSnowflake_Proceed(entity_t *svself)
{
  float     timediff = *e.gametime - *e.lastgametime;
  gentity_t *gself = G_GEntity(svself);
  vec3_t    wavedir;

  if(!CheckFadeAndFall(gself))
  {
    EM_Free(gself);
    return;
  }

  // don't wave as long as not flying down at an angle of at least 90°
  if(vec3DotProduct(AXIS[1], gself->sv->direction) > -0.5f)
    return;


  // if first call find a good wave angle to start with
  if(!gself->n.particle.n.snowflake.initialized)
  {
    // if flight "dir" points straight down get random wave, otherwise
    // find the best angle to start with to prevent occasional direction change
    if(gself->sv->direction[1] == -1.0f)
    {
      gself->n.particle.n.snowflake.wave = G_Random() * 179.0f;
    }
    else
    {
      float initangle;

      // FIXME: this is not 100% correct, angle is messed
      initangle = AngleAroundAxisNum(AXIS[0], gself->sv->direction, 1);
      gself->n.particle.n.snowflake.wave = initangle;
      //e.conprintf("wave: %f (self->dir: %s)\n", gself->n.particle.n.snowflake.wave, v3s(gself->sv->direction));
    }
    gself->n.particle.n.snowflake.isclockwise = (G_Random() > 0.5f) ? 1 : 0;
    gself->n.particle.n.snowflake.initialized = 1;
  }

  // wave around
  // FIXME: nearly equal code twice
  if(gself->n.particle.n.snowflake.isclockwise)
  {
    if((gself->n.particle.n.snowflake.wave -= timediff * 5 * G_Random()) < 0.0f)
      gself->n.particle.n.snowflake.wave += 180.0f;
  }
  else
  {
    if((gself->n.particle.n.snowflake.wave += timediff * 5 * G_Random()) >= 180.0f)
      gself->n.particle.n.snowflake.wave -= 180.0f;
  }

  // alter "direction" according to current wave
  wavedir[0] = RAD2DEG(sin(gself->n.particle.n.snowflake.wave));
  wavedir[1] = 0.0f;
  wavedir[2] = RAD2DEG(cos(gself->n.particle.n.snowflake.wave));
  vec3Normalize(wavedir);
  vec3MA(wavedir, AXIS[1], -1.0f, gself->sv->direction);
  vec3Normalize(gself->sv->direction);  // just to be on the safe side
} // ShaderSnowflake_Proceed
