#ifndef EFLPP_EDJEPART_H
#define EFLPP_EDJEPART_H

/* STL */
#include <string>

/* EFL++ */
#include <eflxx/Common.h>
#include <eflxx/CountedPtr.h>

#include <evasxx/Object.h>

using std::string;

namespace Edjexx {

class Object;

class Part
{
  friend class Object;

private:
  Part( Object* parent, const std::string &partname );

public:
  ~Part();

  Eflxx::Rect getGeometry() const;

  void setText( const std::string &text );
  const std::string getText() const;

  void swallow( Evasxx::Object* );
  void unswallow( Evasxx::Object* );

  Eflxx::CountedPtr <Evasxx::Object> swallow();

  //const Evasxx::Object* getObject ( const char* name );

private:
  Object* _parent;
  const std::string &_partname;

  /* State?
  EAPI const char  *edje_object_part_state_get      (Evas_Object *obj, const char *part, double *val_ret);
  */

  /* Directions?
  EAPI int          edje_object_part_drag_dir_get   (Evas_Object *obj, const char *part);
  */

  /* Drag?
  EAPI void         edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);
  EAPI void         edje_object_part_drag_value_get (Evas_Object *obj, const char *part, double *dx, double *dy);
  EAPI void         edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);
  EAPI void         edje_object_part_drag_size_get  (Evas_Object *obj, const char *part, double *dw, double *dh);
  EAPI void         edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);
  EAPI void         edje_object_part_drag_step_get  (Evas_Object *obj, const char *part, double *dx, double *dy);
  EAPI void         edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);
  EAPI void         edje_object_part_drag_page_get  (Evas_Object *obj, const char *part, double *dx, double *dy);
  EAPI void         edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);
  EAPI void         edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);
  */
private:
  Part();
  Part( const Part& );
  bool operator=( const Part& );
  bool operator==( const Part& );
};

} // end namespace Edjexx

#endif // EFLPP_EDJEPART_H
