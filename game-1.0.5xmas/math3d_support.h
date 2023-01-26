#ifndef __MATH3D_SUPPORT_H
#define __MATH3D_SUPPORT_H

// some more math 3d functions

#include "math3d.h"

#define vec3Clear(v)    { v[0] = v[1] = v[2] = 0.0f; }

// out := a + b*scale
void vec3MA(const vec3_t a, const vec3_t b, float scale, vec3_t out);
void AxesCopy(const vec3_t in[3], vec3_t out[3]);
#define vec3LenSquared(v)   ((v)[0] * (v)[0] + (v)[1] * (v)[1] + (v)[2] * (v)[2])
float AngleAroundAxisNum(const vec3_t v1, const vec3_t v2, int axisnum);
void vec3NormRandom(vec3_t out);

void BuildAxisByDir(vec3_t right, vec3_t up, const vec3_t dir);
void BuildAxisByAngles(float pitch, float yaw, float roll,
                       vec3_t right, vec3_t up, vec3_t dir);

void VectorsToAngles(const vec3_t right, const vec3_t up, const vec3_t dir, vec3_t angles);


#endif  /* __MATH3D_SUPPORT_H */
