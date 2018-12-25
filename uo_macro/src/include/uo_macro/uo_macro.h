#ifndef UO_MACRO_H
#define UO_MACRO_H

#ifdef __cplusplus
extern "C" {
#endif

#define UO_CAT(x, y) x ## y
#define UO_EVALCAT(x, y) UO_CAT(x, y)
#define UO_VAR(ident) UO_EVALCAT(ident, __LINE__)

#define UO_STRLEN(s) (sizeof(s)/sizeof(s[0]) - 1)

#ifdef __cplusplus
}
#endif

#endif