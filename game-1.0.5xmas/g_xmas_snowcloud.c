// (invisible) cloud residing above client and emitting snow flakes

#include "g_local.h"


static vec3_t   s_clientpos;
static vec3_t   s_emissionaxes[3];


static void EmitSnowFlake(int initialcall);
static void OnFree(gentity_t *gself);   // emit one new snowflake


//------------------------------------------------------
// SnowCloud_Start
//------------------------------------------------------
void SnowCloud_Start(const vec3_t clientpos, int numsnowflakes)
{
  int   i;

  vec3Copy(clientpos, s_clientpos);

  // create a emission axes for all upoming snowflake emissions;
  // fall straight down with these emission axes
  AxesCopy(AXIS, s_emissionaxes);
  vec3Clear(s_emissionaxes[0]);
  vec3Mul(s_emissionaxes[1], -1);
  vec3Clear(s_emissionaxes[2]);

  for(i = 0; i < numsnowflakes; i++)
  {
    EmitSnowFlake(1);
  }
} // SnowCloud_Start


//--------------------------------------------------
// EmitSnowFlake - if initial call force broader random values for
//                 snowflake's max. travel distance to spread
//                 time when next snowflake will be emitted
//--------------------------------------------------
static void EmitSnowFlake(int initialcall)
{
  vec3_t    partorg;
  float     maxtraveldist;

  Part_SetEmissionAxes(s_emissionaxes);
  Part_RestrictToPositiveDirections(0, 1, 0);
  Part_SetShader(PARTSHADER_SNOWFLAKE);
  Part_ScaleSize(0.7f + G_Random());
  Part_ClampMoveSpeed(10.0f, 30.0f);

  if(initialcall)
    maxtraveldist = G_Random() * 100.0f;
  else
    maxtraveldist = 150.0f + G_Random() * 150.0f;

  Part_SnowFlake_SetMaxTravelDistance(maxtraveldist);

  Part_SetOnFree(OnFree);

  partorg[0] = s_clientpos[0] + (G_Random() * 300 - 150);
  partorg[1] = s_clientpos[1] + 80 + G_Random()*100;
  partorg[2] = s_clientpos[2] + (G_Random() * 300 - 150);

  Part_EmitRadially(partorg, 1, 15.0f, g_color_white);
} // EmitSnowFlake


//--------------------------------------------------
// OnFree - emit one new snowflake
//--------------------------------------------------
static void OnFree(gentity_t *gself)
{
  //e.conprintf("g_xmas_snowcloud.c::OnFree()\n");
  EmitSnowFlake(0);
} // OnSrvFree
