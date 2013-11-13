#ifdef __GNUC__
  #include <features.h>
  #if !__GNUC_PREREQ(4,6)
    #define nullptr 0
  #endif
#endif
