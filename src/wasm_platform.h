#include <GL/gl.h>
#include <GL/glext.h>

NotImplemented;
typedef int PFNSWAPINTERVALPROC;

#define GAME_LIB "./bin/libGameLoadable.so"

#define THREAD_MAIN_RETURN void*

#define EXPORT extern "C" __attribute__((visibility("default")))

#define CompleteAllWrites  asm volatile("" ::: "memory"); _mm_sfence()

/*
 * glX Business
 */
#define bonsaiGlGetProcAddress(procName) glXGetProcAddress((GLubyte*)procName)
/* typedef PFNGLXSWAPINTERVALEXTPROC PFNSWAPINTERVALPROC; */

#define GlobalCwdBufferLength 2048

#define WindowEventMasks StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask

// FIXME(Jesse): Write own snprintf
#define Snprintf(...) snprintf(__VA_ARGS__)

// In Cygwin printing to the console with printf doesn't work, so we have a
// wrapper that does some additional crazyness on Win32
#define PrintConsole(Message) printf(Message)


NotImplemented;
typedef int thread_id;
typedef int semaphore;
typedef int shared_lib;
typedef int window;
typedef int display;
typedef int gl_context;
NotImplemented;

inline void
WakeThread( semaphore *Semaphore )
{
  NotImplemented;
}
