// math3d_support.c - some math 3d support functions

#include <assert.h>

// game code header
#include "g_shared.h"   // G_Random() for vec3NormRandom()


// engine header
#include "math3d.h"



//---------------------------------
// vec3MA
//---------------------------------
void vec3MA(const vec3_t a, const vec3_t b, float scale, vec3_t out)
{
  int   i;

  for(i = 0; i < 3; i++)
    out[i] = a[i] + b[i] * scale;
} // vec3MA


//---------------------------------
// AxesCopy
//---------------------------------
void AxesCopy(const vec3_t in[3], vec3_t out[3])
{
  int   i;

  for(i = 0; i < 3; i++)
    vec3Copy(in[i], out[i]);
} // AxesCopy


//---------------------------------
// AngleAroundAxisNum
//---------------------------------
float AngleAroundAxisNum(const vec3_t v1, const vec3_t v2, int axisnum)
{
  vec3_t    v1_mapped;
  vec3_t    right_mapped;
  vec3_t    v2_mapped;

  assert(axisnum == 0
      || axisnum == 1
      || axisnum == 2);

  // FIXME: I have't yet tried with z-axis
  assert(axisnum != 2);

  // map "v1" onto plane
  vec3Copy(v1, v1_mapped);
  v1_mapped[axisnum] = 0.0f;
  vec3Normalize(v1_mapped);

  // build "right" on plane
  vec3CrossProduct(v1_mapped, AXIS[axisnum], right_mapped);

  vec3Copy(v2, v2_mapped);
  v2_mapped[axisnum] = 0.0f;
  vec3Normalize(v2_mapped);

  return RAD2DEG(acos(vec3Cos(v2_mapped, right_mapped))) - 90.0f;
} // AngleAroundAxisNum


//------------------------------------------
// vec3NormRandom
//------------------------------------------
void vec3NormRandom(vec3_t out)
{
  int   i;

  for(i = 0; i < 3; i++)
  {
    if(!(out[i] = G_Random() - 0.5f))
      out[i] = 0.0001f;     // to avoid zero-vec if this happens 3 times
  }
  vec3Normalize(out);
} // vec3NormRandom


//------------------------------------------------------------
// BuildAxisByDir
//
// - build "right" and "up" from a given direction vector
// - "right" will be on the x/z plane
// - IMPORTANT: the given "dir" must -not- be collinear to the default y-axis!
//   (otherwise, the resulting "right" and "up" will both have zero-length!)
//------------------------------------------------------------
void BuildAxisByDir(vec3_t right, vec3_t up, const vec3_t dir)
{
  vec3_t    temp;

  // map "dir" onto the x/z plane
  temp[0] = dir[0];
  temp[1] = 0;
  temp[2] = dir[2];
  vec3Normalize(temp);

  // build "right" (onto the x/z plane)
  vec3CrossProduct(temp, AXIS[1], right);

  // build "up"
  vec3CrossProduct(right, dir, up);

  // make sure, the resulting vectors are still normalized
  // FIXME: obsolete?
  vec3Normalize(right);
  vec3Normalize(up);
} // BuildAxisByDir


//------------------------------------------------------------
// BuildAxisByAngles
//------------------------------------------------------------
void BuildAxisByAngles(float pitch, float yaw, float roll,
                       vec3_t right, vec3_t up, vec3_t dir)
{
  vec3Copy(AXIS[2], dir);
  vec3Rotate(dir, -pitch, -yaw, -roll);
  BuildAxisByDir(right, up, dir);
} // BuildAxisByAngles


//------------------------------------------------------------
// VectorsToAngles - construct 3 angles of given vectors, assuming "right"
//                   is on the x/z plane
//------------------------------------------------------------
//
// FIXME: I'm surprised that this embarrassing crap even works...
//
void VectorsToAngles(const vec3_t right,
                     const vec3_t up,
                     const vec3_t dir,
                     vec3_t angles)
{
  // don't rotate around entity's "dir" vector
  angles[2] = 0;

  //
  // compute rotation around default y-axis by "right" and default z-axis
  //
/*
 bird's view for rotation around default y-axis:
 (here, the default y-axis goes out of the screen)

         |
    Q4   |
         |
         |Q3
  -------+-------> X+
    Q1   |
         |
         |Q2
         |
         v
         Z+
*/
  angles[1] = RAD2DEG(acos(vec3Cos(right, AXIS[2])));

  // for easy access of "right"'s components
  #define RX    right[0]
  #define RZ    right[2]

  // 2002-08-26: fixed computation for fixed vec3CrossProduct()
  // 2002-08-30: fixed computation again since AXIS[2][2] == "-1"
  // "right" in Q1 ?
  if(RX < 0 && RZ >= 0)
  {
    //e.conprintf("-- 1 --\n");
    angles[1] = angles[1] + 90;
  }
  // "right" in Q2 ?
  else if(RX >= 0 && RZ > 0)
  {
    //e.conprintf("-- 2 --\n");
    angles[1] = 90 - angles[1];
  }
  // "right" in Q3 ?
  else if(RX >= 0 && RZ <= 0)
  {
    //e.conprintf("-- 3 --\n");
    angles[1] = 90 - angles[1];
  }
  // "right" must be in Q4!
  else
  {
    //e.conprintf("-- 4 --\n");
    angles[1] = angles[1] + 90;
  }


  #undef RX
  #undef RZ

  //
  // compute rotation around "right" by "dir" and default y-axis
  //
  angles[0] = RAD2DEG(acos(vec3Cos(dir, AXIS[1])));

  #define DX    dir[0]
  #define DY    dir[1]

  // 2002-08-26: fixed computation for fixed vec3CrossProduct()
  // 2002-08-31: fixed last case
  if(DX > 0 && DY > 0)
  {
    //e.conprintf("-- 1 --\n");
    angles[0] = 270 + angles[0];
  }
  else if(DX <= 0 && DY > 0)
  {
    //e.conprintf("-- 2 --\n");
    angles[0] = angles[0] - 90;
  }
  else if(DX <= 0 && DY <= 0)
  {
    //e.conprintf("-- 3 --\n");
    angles[0] = angles[0] - 90;
  }
  else
  {
    //e.conprintf("-- 4 --\n");
    //OLD, BROKEN: angles[0] = 360 - (90 - angles[0]);
    angles[0] = angles[0] - 90;
  }

  #undef DX
  #undef DY

  // clamp angles
  while(angles[1] >= 360.0f)
    angles[1] =- 360.0f;
  while(angles[0] >= 360.0f)
    angles[0] =- 360.0f;

  while(angles[1] <= -360.0f)
    angles[1] =+ 360.0f;
  while(angles[0] <= -360.0f)
    angles[0] =+ 360.0f;

} // VectorsToAngles
