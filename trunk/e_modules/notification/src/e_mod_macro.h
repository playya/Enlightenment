#ifndef E_MOD_MACRO_H
#define E_MOD_MACRO_H

#define D_(str) dgettext(PACKAGE, str)

#define MOD_CFG_FILE_EPOCH 0x0002
#define MOD_CFG_FILE_GENERATION 0x0003
#define MOD_CFG_FILE_VERSION \
   ((MOD_CFG_FILE_EPOCH << 16) | MOD_CFG_FILE_GENERATION)

#undef  __UNUSED__
#define __UNUSED__ __attribute__((unused))

#undef  MIN_LEN
#define MIN_LEN(str1, str2) strlen(str1) < strlen(str2) ? strlen(str1) : strlen(str2)

#undef  MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#endif

