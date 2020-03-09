
#define meta(...)
#define break_here

#define ITERATE_OVER(type, value_ptr)                \
  for (type##_iterator Iter = Iterator((value_ptr)); \
      IsValid(&Iter);                                \
      Advance(&Iter))

#define GET_ELEMENT(I) (&(I).At->Element)

