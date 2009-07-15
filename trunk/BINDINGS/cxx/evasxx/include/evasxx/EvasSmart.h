#ifndef EVASSMART_H
#define EVASSMART_H

/* STD */
#include <map>

/* EFLxx */
#include <eflxx/CountedPtr.h>
#include "EvasCanvas.h"

/**
 * C++ Wrapper for the Enlightenment Smart Object Library (ESMART)
 *
 */

namespace efl {
  
struct CustomEventWrap
{
  EvasSmart *es;
  sigc::signal <void, void*, Evas_Object*, void*> *customSignal; // TODO: C++ style!
  string event;
};

typedef std::map <const std::string, CustomEventWrap*> CustomEventWrapMapType;

class EvasSmart : public EvasObject
{
public:
  EvasSmart (EvasCanvas &canvas);
  virtual ~EvasSmart ();
  
  sigc::signal <void> signalAdd; /// called when smart object is added
  sigc::signal <void> signalDel; /// called when smart object is deleted
  sigc::signal <void, const Point&> signalMove; /// callback called when smart object is moved
  sigc::signal <void, const Size&> signalResize; /// callback called when smart object is resized
  sigc::signal <void> signalShow; /// callback called when smart object is shown
  sigc::signal <void> signalHide; // callback called when smart object is hidden
  sigc::signal <void, const Color&> signalSetColor;  /// callback called when smart object has its color set
  sigc::signal <void, const EvasObject&> signalSetClip; /// callback called when smart object has its clip set
  sigc::signal <void> signalUnsetClip; /// callback called when smart object has its clip unset
  sigc::signal <void, const EvasObject&> signalAddMember; /// FIXME: docu
  sigc::signal <void, const EvasObject&> signalDelMember; /// FIXME: docu
    
  /*!
   * Set an EvasObject as a member of a smart object.
   *
   * @param obj The member object
   *
   * Members will automatically be stacked and layered with the smart object. 
   * The various stacking function will operate on members relative to the
   * other members instead of the entire canvas.
   *
   * Non-member objects can not interleave a smart object's members.
   *
   */
  void addMember (const EvasObject &obj);
  
  /*!
   * Removes a member object from a smart object.
   *
   * @param obj the member object
   *
   * This removes a member object from a smart object. The object will still
   * be on the canvas, but no longer associated with whichever smart object
   * it was associated with.
   *
   */
  static void delMember (const EvasObject &obj);
  
  /*!
   * Gets the list of the member objects.
   *
   * @param obj the Evas_Object you want to get the list of member objects
   * @return Returns the list of the member objects
   * The returned list should be freed with eina_list_free() when you no longer need it
   * TODO: wrap Eina_List
   */
  Eina_List *getMembers ();
  
  /*!
   * Retrieve user data stored on a smart object.
   *
   * @return A pointer to data stored using setData(), or 
   *         NULL if none has been set.
   */
  void *getData ();
  
  /*!
   * Store a pointer to user data for a smart object.
   *
   * @param data A pointer to user data
   */
  void setData (void *data);
  
  void addEventSignal (const std::string &event);
  
  void delEventSignal (const std::string &event);
  
  sigc::signal <void, void*, Evas_Object*, void*> *getEventSignal (const std::string &event);
  
  void callEventSignal (const std::string &event, void *event_info);
  
  /*!
   * Mark smart object as changed, dirty.
   *
   * This will inform the scene that it changed and needs to be redraw, also
   * setting need_recalculate flag on the given object.
   *
   * @see setNeedRecalculate().
   */
  void hasChanged ();
  
  /*!
   * Set the need_recalculate flag of given smart object.
   *
   * If this flag is set then calculate() callback (method) of the given
   * smart object will be called, if one is provided, during render phase
   * usually evas_render(). After this step, this flag will be automatically
   * unset.
   *
   * If no calculate() is provided, this flag will be left unchanged.
   *
   * @note just setting this flag will not make scene dirty and evas_render()
   *       will have no effect. To do that, use hasChanged(),
   *       that will automatically call this function with 1 as parameter.
   *
   * @param value if one want to set or unset the need_recalculate flag.
   */
  void setNeedRecalculate (bool value);
  
  /*!
   * Get the current value of need_recalculate flag.
   *
   * @note this flag will be unset during the render phase, after calculate()
   *       is called if one is provided.  If no calculate() is provided, then
   *       the flag will be left unchanged after render phase.
   *
   * @return if flag is set or not.
   */
  bool setNeedRecalculate ();
  
  /*!
   * Call user provided calculate() and unset need_calculate.
   *
   * @return if flag is set or not.
   */
  void calculate ();
  
protected:
  /*! 
   * a wrapper for an implemented C Esmart object
   * allow only for child classes
   */
  EvasSmart ();

private:
  Evas_Smart *getEsmart( const std::string &name );
  Evas_Object *newEsmart( EvasCanvas &canvas, const std::string &name );
  
  static void wrap_add(Evas_Object *o);
  static void wrap_del(Evas_Object *o);
  static void wrap_move(Evas_Object *o, Evas_Coord x, Evas_Coord y);
  static void wrap_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h);
  static void wrap_show(Evas_Object *o);
  static void wrap_hide( Evas_Object *o);
  static void wrap_color_set(Evas_Object *o, int r, int g, int b, int a);
  static void wrap_clip_set(Evas_Object *o, Evas_Object *clip); // TODO: Evas_Object -> EvasObject?
  static void wrap_clip_unset(Evas_Object *o);
  static void wrap_member_add(Evas_Object *o, Evas_Object *clip); // TODO: Evas_Object -> EvasObject?
  static void wrap_member_del(Evas_Object *o, Evas_Object *clip); // TODO: Evas_Object -> EvasObject?
  
  Evas_Smart_Class sc;
  CustomEventWrapMapType mCustomSignalMap;
};

} // end namespace efl

#endif // EVASSMART_H
