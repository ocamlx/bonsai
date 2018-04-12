#ifndef bonsai_shader_cpp
#define bonsai_shader_cpp

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <string.h>

#define INVALID_SHADER_UNIFORM (-1)

char *
ReadEntireFileIntoString(const char *Filepath, memory_arena *Memory)
{
  FILE *File = fopen(Filepath, "r");
  char *FileContents = 0;

  if (File)
  {
    fseek(File, 0L, SEEK_END);
    u64 FileSize = ftell(File);

    rewind(File);
    FileContents = (char*)PushSize(Memory, FileSize + 1);
    fread(FileContents, FileSize, 1, File);

    // FIXME(Jesse): Why is either PushSize not returning 0'd memory or fread
    // reading more than FileSize bytes into our memory on EMCC ?!!
    //
    FileContents[FileSize] = 0;
  }
  else
  {
    Error("Opening %s", Filepath);
  }

  return FileContents;
}

s32
CompileShader(const char *Header, const char *Code, u32 Type)
{
  AssertNoGlErrors;
  u32 ShaderID = GL_Global->glCreateShader(Type);
  AssertNoGlErrors;

  const char *Sources[2] = {Header, Code};

  /* Debug("%s", Header); */
  /* Debug("%s", Code); */

  // Compile
  GL_Global->glShaderSource(ShaderID, 2, Sources, NULL);
  AssertNoGlErrors;
  GL_Global->glCompileShader(ShaderID);
  AssertNoGlErrors;

  // Check Status
  s32 Result = GL_FALSE;
  s32 InfoLogLength = 0;

  GL_Global->glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
  AssertNoGlErrors;
  GL_Global->glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, (s32*)&InfoLogLength);
  AssertNoGlErrors;

  // FIXME(Jesse): EMCC/webgl?? is misbehaving and setting this to 1 when there
  // is actually nothing to report
  if (InfoLogLength == 1)
    InfoLogLength = 0;

  if ( InfoLogLength > 0 )
  {
    char *VertexShaderErrorMessage = (char*)malloc(InfoLogLength+1);
    GL_Global->glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, VertexShaderErrorMessage);
    Error("Shader : %s", VertexShaderErrorMessage);
    ShaderID = INVALID_SHADER_UNIFORM;
  }

  AssertNoGlErrors;
  return ShaderID;
}

shader
LoadShaders(const char * VertShaderPath, const char * FragFilePath, memory_arena *Memory)
{
  Info("Creating shader : %s | %s", VertShaderPath, FragFilePath);

  // FIXME(Jesse): For gods sake don't use sprintf
  char ComputedVertPath[2048] = {};
  Snprintf(ComputedVertPath, 2048, "%s/%s", SHADER_PATH, VertShaderPath);

  char ComputedFragPath[2048] = {};
  Snprintf(ComputedFragPath, 2048, "%s/%s", SHADER_PATH, FragFilePath);

  char *HeaderCode       = ReadEntireFileIntoString(SHADER_PATH SHADER_HEADER, Memory);
  char *VertexShaderCode = ReadEntireFileIntoString(ComputedVertPath, Memory);
  char *FragShaderCode   = ReadEntireFileIntoString(ComputedFragPath, Memory);


  s32 Result = GL_FALSE;
  int InfoLogLength;


  AssertNoGlErrors;
  s32 VertexShaderID = CompileShader(HeaderCode, VertexShaderCode, GL_VERTEX_SHADER);
  AssertNoGlErrors;
  s32 FragmentShaderID = CompileShader(HeaderCode, FragShaderCode, GL_FRAGMENT_SHADER);
  AssertNoGlErrors;

  // Link the program
  u32 ProgramID = GL_Global->glCreateProgram();
  Assert(ProgramID);

  AssertNoGlErrors;

  GL_Global->glAttachShader(ProgramID, VertexShaderID);
  AssertNoGlErrors;
  GL_Global->glAttachShader(ProgramID, FragmentShaderID);
  AssertNoGlErrors;
  GL_Global->glLinkProgram(ProgramID);
  AssertNoGlErrors;

  // Check the program
  GL_Global->glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  AssertNoGlErrors;
  GL_Global->glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  AssertNoGlErrors;

  // FIXME(Jesse): EMCC/webgl?? is misbehaving and setting this to 1 when there
  // is actually nothing to report
  if (InfoLogLength == 1)
    InfoLogLength = 0;

  if ( InfoLogLength > 0 )
  {
    char *ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
    GL_Global->glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
    Error("%s", ProgramErrorMessage);
  }


  GL_Global->glDetachShader(ProgramID, VertexShaderID);
  AssertNoGlErrors;
  GL_Global->glDetachShader(ProgramID, FragmentShaderID);
  AssertNoGlErrors;

  GL_Global->glDeleteShader(VertexShaderID);
  AssertNoGlErrors;
  GL_Global->glDeleteShader(FragmentShaderID);
  AssertNoGlErrors;

  shader Shader = {};
  Shader.ID = ProgramID;

  return Shader;
}

#endif
