#ifndef __EDVI_ENUM_H__
#define __EDVI_ENUM_H__


#define EDVI_DEFAULT_DPI      300
#define EDVI_DEFAULT_SHRINK   5
#define EDVI_DEFAULT_AA       4
#define EDVI_DEFAULT_OFFSET_X 1.00  /*inch*/
#define EDVI_DEFAULT_OFFSET_Y 1.00  /*inch*/

typedef enum
{
  EDVI_PAGE_ORIENTATION_PORTRAIT,
  EDVI_PAGE_ORIENTATION_LANDSCAPE
}Edvi_Page_Orientation;

typedef enum
{
  EDVI_PAGE_SIZE_A1,
  EDVI_PAGE_SIZE_A2,
  EDVI_PAGE_SIZE_A3,
  EDVI_PAGE_SIZE_A4,
  EDVI_PAGE_SIZE_A5,
  EDVI_PAGE_SIZE_A6,
  EDVI_PAGE_SIZE_A7,
  EDVI_PAGE_SIZE_B1,
  EDVI_PAGE_SIZE_B2,
  EDVI_PAGE_SIZE_B3,
  EDVI_PAGE_SIZE_B4,
  EDVI_PAGE_SIZE_B5,
  EDVI_PAGE_SIZE_B6,
  EDVI_PAGE_SIZE_B7,
  EDVI_PAGE_SIZE_LETTER,
  EDVI_PAGE_SIZE_US,
  EDVI_PAGE_SIZE_LEGAL
}Edvi_Page_Size;


#endif /* __EDVI_ENUM_H__ */
