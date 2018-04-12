#include <emscripten.h>

extern "C" {
   #include "html5.h" // emscripten module
}

#include <GL/glcorearb.h>
#include <GL/glext.h>

#define GAME_LIB "./bin/libGameLoadable.so"

#define THREAD_MAIN_RETURN void*

#define EXPORT extern "C" __attribute__((visibility("default")))

#define CompleteAllWrites Warn("Implement CompleteAllWrites!!");

/*
 * glWasm Business
 */
#define bonsaiGlGetProcAddress(procName)
/* typedef PFNGLXSWAPINTERVALEXTPROC PFNSWAPINTERVALPROC; */

#define GlobalCwdBufferLength 2048

#define WindowEventMasks StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask

// FIXME(Jesse): Write own snprintf
#define Snprintf(...) snprintf(__VA_ARGS__)

// In Cygwin printing to the console with printf doesn't work, so we have a
// wrapper that does some additional crazyness on Win32
#define PrintConsole(Message) printf(Message)

// FIXME(Jesse): NotImplemented
typedef int thread_id;
typedef int semaphore;
typedef int shared_lib;
typedef int window;
typedef int display;
typedef int gl_context;
typedef int PFNSWAPINTERVALPROC;
// FIXME(Jesse): NotImplemented

inline void
WakeThread( semaphore *Semaphore )
{
  Info("Implement WakeThread!");
}
