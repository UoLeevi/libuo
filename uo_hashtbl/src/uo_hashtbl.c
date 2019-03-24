#include "uo_hashtbl.h"
#include "uo_util.h"

#include <assert.h>
#include <string.h>

uo_impl_hashtbl(uo_str, const char *, uo_strhash_djb2, uo_streq);
uo_impl_hashtbl(uo_, const void *, (uintptr_t), uo_eq);
uo_impl_hashtbl(uo_int, int, (uint64_t), uo_eq);
