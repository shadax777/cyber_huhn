// g_items.s - allocate items + give to receivers

#include "g_local.h"


static void AnyItem_Expire(entity_t *svself);  // think()


//
// bonus weapon stuff
//
#define BONUSWEAPON_DURATION    16.0f



//-------------------------------------------------
// I_GiveItem
//-------------------------------------------------
void I_GiveItem(gentity_t *receiver, subclass_t itemclass)
{
  const classinfo_t *ci;
  gentity_t         *item;

  assert(receiver);

  // spawn the item
  ci = G_GetClassInfo(CLASS_ITEM, ITEM_BONUSWEAPON);
  assert(ci);
  item = EM_SpawnByClassInfo(ci);
  if(!item)   // probably out of entities
  {
    // blah
    return;
  }

  item->owner = receiver;
  if(receiver->OnReceiveItem)
  {
    receiver->OnReceiveItem(receiver, item);
  }
  receiver->item = item;

  // start the item
  switch(itemclass)
  {
    case ITEM_BONUSWEAPON:
      item->sv->think = AnyItem_Expire;
      item->sv->nextthink = *e.gametime + BONUSWEAPON_DURATION;
      break;

    default:    // I introduced a new item and forget to update it here
      G_Assert(0, G_Stringf("item class: %i\n", itemclass));
      break;
  }
} // I_GiveItem


//----------------------------------------------------------
// I_GetRemainingTime
//----------------------------------------------------------
float I_GetRemainingTime(const gentity_t *item)
{
  float age;

  assert(item);
  G_Assert(item->mainclass == CLASS_ITEM, E_GetString(item));

  age = (*e.gametime - item->timestamp);
  switch(item->subclass)
  {
    case ITEM_BONUSWEAPON:
      return BONUSWEAPON_DURATION - age;
      break;

    default:    // I introduced a new item and forget to update it here
      G_Assert(0, E_GetString(item));
      return 0.0f;
      break;
  }
} // I_GetRemainingTime


//----------------------------------------------------------
// AnyItem_Expire
//----------------------------------------------------------
static void AnyItem_Expire(entity_t *svself)
{
  gentity_t *gself;
  gentity_t *owner;

  gself = G_GEntity(svself);
  owner = gself->owner;
  assert(owner);

  // if owner is alive AND he's carrying this item make him
  // aware of removing the item and take it away
  if((owner->gstate == ES_ACTIVE)
  && (gself->timestamp >= owner->timestamp)
  && (owner->item == gself))
  {
    if(owner->OnItemExpires != NULL)
      owner->OnItemExpires(owner, gself);
    owner->item = NULL;
  }
  EM_Free(gself);
} // AnyItem_Expire
