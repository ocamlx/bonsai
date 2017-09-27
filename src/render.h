#ifndef RENDER_H
#define RENDER_H

#include <shader.hpp>

DEBUG_GLOBAL float g_quad_vertex_buffer_data[] =
{
  -1.0f, -1.0f, 1.0f,
   1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
   1.0f, -1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
};

typedef u32 framebuffer;

struct RenderBasis
{
  m4 ModelMatrix;
  m4 ViewMatrix;
  m4 ProjectionMatrix;
};

struct texture
{
  u32 ID;
  v2i Dim;
};

struct ao_render_group
{
  framebuffer Framebuffer;
  texture AoTexture;
  shader AoShader;
};

struct g_buffer_render_group
{
  u32 FBO;

  texture *SsaoNoiseTexture;

  v3 SsaoKernel[SSAO_KERNEL_SIZE];

  u32 colorbuffer;
  u32 vertexbuffer;
  u32 normalbuffer;

  u32 quad_vertexbuffer;

  shader GBufferShader;
  u32 MVPID;
  u32 ModelMatrixID;
  /* u32 LightPID; */


  u32 GlobalLightPositionID;;

  // Lighting Shader
  shader LightingShader;
  u32 ColorTextureUniform;
  u32 NormalTextureUniform;
  u32 PositionTextureUniform;
  u32 DepthTextureUniform;

  u32 ShadowMapTextureUniform;
  u32 DepthBiasMVPID;
  u32 ViewMatrixUniform;
  u32 ProjectionMatrixUniform;
  u32 CameraPosUniform;
  u32 SsaoKernelUniform;
  //

  shader DebugColorTextureShader;
  shader DebugNormalTextureShader;
  shader DebugPositionTextureShader;

  RenderBasis Basis;
};

struct ShadowRenderGroup
{
  u32 MVP_ID;

  shader DebugTextureShader;

  shader DepthShader;
  u32 FramebufferName;
};

struct debug_text_render_group
{
  u32 Text2DTextureID;
  u32 Text2DVertexBufferID;
  u32 Text2DUVBufferID;

  shader Text2DShader;
  u32 Text2DUniformID;
};

inline void
SetViewport(v2 Dim)
{
  glViewport(0, 0, Dim.x, Dim.y);
  return;
}

inline m4
Orthographic( r32 X, r32 Y, r32 Zmin, r32 Zmax, v3 Translate )
{

#if 1
  m4 Result = GLM4(glm::ortho<r32>(-X+Translate.x, X+Translate.x,
                                   -Y+Translate.y, Y+Translate.y,
                                   Zmin +Translate.z, Zmax +Translate.z));
#else
  m4 Result = IdentityMatrix;
  Assert(False);
#endif

  return Result;
}

inline m4
Perspective(radians FOV, r32 AspectRatio, r32 NearClip, r32 FarClip)
{

#if 1
  glm::mat4 Projection = glm::perspective(FOV, AspectRatio, NearClip, FarClip);
  m4 Result = GLM4(Projection);
#else
  // Scale
  r32 S = (1/(tan((FOV/2) * (PIf/180))));

  // Remap z to 0-1
  r32 Z = (-FarClip)/(FarClip-NearClip);
  r32 ZZ = -FarClip * NearClip / (FarClip - NearClip);

  m4 Result = {
    V4(S, 0, 0, 0),
    V4(0, S, 0, 0),
    V4(0, 0, Z, -1),
    V4(0, 0, ZZ, 0),
  };
#endif



  return Result;
}

inline radians
Rads(degrees Degrees)
{
  radians Result = (Degrees/180);
  return Result;
}

inline m4
GetProjectionMatrix(camera *Camera, int WindowWidth, int WindowHeight)
{
  m4 Projection = Perspective(
      Rads(Camera->Frust.FOV),
      (float)WindowWidth/(float)WindowHeight,
      Camera->Frust.nearClip,
      Camera->Frust.farClip);

  return Projection;
}

inline v3
GetRenderP( chunk_dimension WorldChunkDim, canonical_position P, camera *Camera)
{
  v3 CameraOffset = Camera->Target.Offset + (Camera->Target.WorldP * WorldChunkDim);
  v3 Result = P.Offset + (P.WorldP * WorldChunkDim) - CameraOffset;
  return Result;
}

inline v3
GetRenderP( chunk_dimension WorldChunkDim, v3 Offset, camera *Camera)
{
  v3 Result = GetRenderP(WorldChunkDim, Canonical_Position(Offset, World_Position(0)), Camera);
  return Result;
}

inline v3
GetRenderP( chunk_dimension WorldChunkDim, world_position WorldP, camera *Camera)
{
  v3 Result = GetRenderP(WorldChunkDim, Canonical_Position(V3(0,0,0), WorldP), Camera);
  return Result;
}

inline v3
GetRenderP( chunk_dimension WorldChunkDim, entity *entity, camera *Camera)
{
  v3 Result = GetRenderP(WorldChunkDim, entity->P, Camera);
  return Result;
}

inline aabb
GetRenderSpaceAABB(chunk_dimension WorldChunkDim, entity *Entity, camera *Camera)
{
  v3 Radius = Entity->CollisionVolumeRadius;
  v3 Center = GetRenderP(WorldChunkDim, Entity->P, Camera) + Radius;

  aabb Result(Center, Radius);
  return Result;
}

inline m4
LookAt(v3 P, v3 Target, v3 Up)
{
  glm::mat4 M = glm::lookAt( glm::vec3(P.x, P.y, P.z),
                             glm::vec3(Target.x, Target.y, Target.z),
                             glm::vec3(Up.x, Up.y, Up.z) );

  m4 Result = GLM4(M);

  return Result;
}

inline m4
GetViewMatrix(chunk_dimension WorldChunkDim, camera *Camera)
{
  v3 up = V3(0, 1, 0);
  v3 CameraRight = Normalize( Cross(up, Camera->Front) );
  v3 CameraUp = Normalize( Cross( Camera->Front, CameraRight) );

  m4 Result = LookAt(
    GetRenderP(WorldChunkDim, Camera->P, Camera),
    GetRenderP(WorldChunkDim, Camera->Target, Camera),
    CameraUp
  );

  return Result;
}

#endif
