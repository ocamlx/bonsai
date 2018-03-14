#if NDEBUG // CMAKE defined
#define RELEASE 1
#else
#define DEBUG 1
#endif



#if BONSAI_INTERNAL
#define Assert(condition) if (!(condition)) { RuntimeBreak(); }
#else
#define Assert(...)
#endif

#if BONSAI_INTERNAL
#define NotImplemented() RuntimeBreak()
#else
#define NotImplemented() Implement Meeeeee!!!
#endif
