#include "g_local.h"


//---------------------------------------------------------------
// D_CheckForDamage - check if "other" is a foe of "self"; if so, inflict
//                    damage on "self"
//                    INFO: "OnDamage" and "OnDie" are optional
//---------------------------------------------------------------

int D_CheckForDamage(gentity_t *self,
                     gentity_t *other,
                     void (*OnDamage)(gentity_t *self, float damage),
                     void (*OnDie)(gentity_t *self))
{
  assert(self);
  assert(other);

  if(other->damage <= 0)
    return DAMAGE_NONE;

  if((self->health -= other->damage) <= 0)
  {
    // if colliding entity is missile and it has an owner, consider him as
    // attacker
    if(other->mainclass == CLASS_MISSILE)
    {
      if(other->owner != NULL && other->owner->timestamp <= other->timestamp)
        other = other->owner;
    }

    EC_OnHavingKilled(other, self);

    if(OnDie)
    {
      (*OnDie)(self);
    }
    return DAMAGE_KILL;
  }
  else
  {
    if(other->mainclass == CLASS_MISSILE)
    {
      EC_CheckDamageParticles(self, other->sv->position, other->damage);
    }
    if(OnDamage)
    {
      (*OnDamage)(self, other->damage);
    }
    return DAMAGE_PAIN;
  }
} // D_CheckForDamage
