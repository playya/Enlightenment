#ifndef EON_CHECKER_H_
#define EON_CHECKER_H_
/*============================================================================*
 *                                 Events                                     *
 *============================================================================*/
#define EON_CHECKER_SW_CHANGED "swChanged"
#define EON_CHECKER_SH_CHANGED "shChanged"
#define EON_CHECKER_COLOR1_CHANGED "color1Changed"
#define EON_CHECKER_COLOR2_CHANGED "color2Changed"
/*============================================================================*
 *                               Properties                                   *
 *============================================================================*/
extern Ekeko_Property_Id EON_CHECKER_SW;
extern Ekeko_Property_Id EON_CHECKER_SH;
extern Ekeko_Property_Id EON_CHECKER_COLOR1;
extern Ekeko_Property_Id EON_CHECKER_COLOR2;
/*============================================================================*
 *                                 Class                                      *
 *============================================================================*/
typedef struct _Eon_Checker_Private Eon_Checker_Private;
struct _Eon_Checker
{
	Eon_Paint parent;
	Eon_Checker_Private *private;
};
/*============================================================================*
 *                                Functions                                   *
 *============================================================================*/
EAPI Ekeko_Type *eon_checker_type_get(void);
EAPI Eon_Checker * eon_checker_new(void);
EAPI Eon_Color eon_checker_color1_get(Eon_Checker *sq);
EAPI Eon_Color eon_checker_color2_get(Eon_Checker *sq);
EAPI void eon_checker_color1_set(Eon_Checker *sq, Eon_Color color);
EAPI void eon_checker_color2_set(Eon_Checker *sq, Eon_Color color);

#endif /* EON_CHECKER_H_ */
