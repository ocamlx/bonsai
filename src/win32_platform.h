// Disable warnings about insecure CRT functions
#pragma warning(disable : 4996)

#include <Windows.h>
#include <WindowsX.h> // Macros to retrieve mouse coordinates
#include <WinBase.h>
#include <Wingdi.h>

#include <sys/stat.h>

#include <stdint.h>

#include <direct.h> // Chdir

#define BONSAI_FUNCTION_NAME __FUNCTION__

#define RED_TERMINAL ""
#define BLUE_TERMINAL ""
#define GREEN_TERMINAL ""
#define YELLOW_TERMINAL ""
#define WHITE_TERMINAL ""

#include <stdio.h>

void
VariadicOutputDebugString(const char* FmtString, ...)
{
  va_list Args;
  va_start(Args, FmtString);
  char OutputBuffer[1024] = {};
  vsnprintf(OutputBuffer, 1023, FmtString, Args);
  va_end(Args);
  return;
}

#define GlDebugMessage(...)  PrintConsole(" * Gl Debug Message - ");             \
                             VariadicOutputDebugString(__VA_ARGS__); \
                             PrintConsole("\n")

#define Info(...) do { PrintConsole("   Info - ");             \
                       VariadicOutputDebugString(__VA_ARGS__); \
                       PrintConsole("\n") } while (false)

#define Debug(...) do { VariadicOutputDebugString(__VA_ARGS__); \
                        PrintConsole("\n") } while (false)

#define Error(...) do { PrintConsole(" ! Error - ");            \
                         VariadicOutputDebugString(__VA_ARGS__);   \
                         PrintConsole("\n") } while (false)

#define Warn(...) do {PrintConsole(" * Warn - ");            \
                      VariadicOutputDebugString(__VA_ARGS__); \
                      PrintConsole("\n") } while (false)

#define RuntimeBreak() __debugbreak()


#define GAME_LIB_PATH "bin/Debug/GameLoadable"

#define Newline "\r\n"

struct native_file
{
  FILE* Handle;
  counted_string Path;
};

global_variable HANDLE StdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

#define PrintConsole(Message)                        \
  OutputDebugString(Message);                        \
  WriteFile(StdoutHandle, Message, strlen(Message), 0, 0);

#define PLATFORM_OFFSET (sizeof(void*))

#define GAME_LIB (GAME_LIB_PATH".dll")

#define exported_function extern "C" __declspec( dllexport )

#define THREAD_MAIN_RETURN DWORD WINAPI
#define GAME_MAIN_PROC FARPROC GameMain

#define sleep(seconds) Sleep(seconds * 1000)

#define SWAP_BUFFERS SwapBuffers(hDC)

#define bonsaiGlGetProcAddress(procName) wglGetProcAddress(procName)
typedef bool (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef PFNWGLSWAPINTERVALEXTPROC PFNSWAPINTERVALPROC;

// #define Log(str) OutputDebugString(str)


typedef HANDLE thread_id;
typedef HANDLE semaphore;


// ???
typedef HMODULE shared_lib;
typedef HWND window;
typedef HGLRC gl_context;
typedef HDC display;


// TODO(Jesse): Implement these
typedef int native_mutex;

#define FullBarrier (0)

u64
GetCycleCount()
{
  u64 Result = __rdtsc();
  return Result;
}

bonsai_function void
PlatformMemprotect(void* Page, umm PageSize)
{
  NotImplemented;
}
