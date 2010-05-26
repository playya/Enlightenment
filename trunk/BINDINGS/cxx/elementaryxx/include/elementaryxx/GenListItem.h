#ifndef ELMXX_GEN_LIST_ITEM_H
#define ELMXX_GEN_LIST_ITEM_H

/* EFL */
#include <Elementary.h>

namespace Elmxx {

/* forward declarations */
class GenList;
  
class GenListItem
{
public:
  friend class GenList;
  
private:
  Elm_Genlist_Item *mItem;
};

} // end namespace Elmxx

#endif // ELMXX_GEN_LIST_ITEM_H
