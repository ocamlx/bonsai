#define CACHE_LINE_SIZE (64)
#define bonsai_function static

#ifdef _WIN32
#define BONSAI_WIN32 (1)
#include <win32_platform.h>
#else
#define BONSAI_LINUX (1)
#include <unix_platform.h>
#endif

#define TriggeredRuntimeBreak() do { if (GetDebugState) { if (GetDebugState()->TriggerRuntimeBreak) { RuntimeBreak(); } } } while (false)

struct os
{
  window Window;
  display Display;
  gl_context GlContext;

  b32 ContinueRunning = True;
};
