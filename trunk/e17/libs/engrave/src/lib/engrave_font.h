#ifndef ENGRAVE_FONT_H
#define ENGRAVE_FONT_H

/**
 * @file engrave_font.h Engrave_Font block 
 * @brief Contains all of the functions to maniuplate Engrave_Font blocks
 */

/**
 * @defgroup Engrave_Font Engrave_Font: Functions to work with engrave font objects
 *
 * @{
 */

/**
 * The Engrave_Font typedef
 */
typedef struct _Engrave_Font Engrave_Font;

/**
 * Stores the needed font information.
 */
struct _Engrave_Font
{
  char *name; /**< The font alias */
  char *path; /**< The font path */
};

Engrave_Font *engrave_font_new(char *path, char *name);

/**
 * @}
 */

#endif

