#ifndef BONSAI_MESH_CPP
#define BONSAI_MESH_CPP

#include <colors.h>

void
RenderGBuffer(mesh_buffer_target *Target, g_buffer_render_group *gBuffer, shadow_render_group *SG, camera *Camera);

inline void
BufferVertsDirect(
    mesh_buffer_target *Dest,

    s32 NumVerts,

    v3 *VertsPositions,
    v3 *Normals,
    const v3 *VertColors,
    v3 Offset = V3(0),
    v3 Scale = V3(1)
  )
{
  TIMED_FUNCTION();

  // This path assumes we've already checked there's enough memroy remaining
  if ( Dest->VertsFilled + NumVerts > Dest->VertsAllocated )
  {
    Warn("Ran out of memory pushing %d Verts onto Mesh with %d/%d used", NumVerts, Dest->VertsFilled, Dest->VertsAllocated);
    return;
  }

#if 1
  __m128 mmScale = _mm_set_ps(0, Scale.z, Scale.y, Scale.x);
  __m128 mmOffset = _mm_set_ps(0, Offset.z, Offset.y, Offset.x);

  s32 FaceVerts = 6;
  Assert(NumVerts % FaceVerts == 0);

  for ( s32 VertIndex = 0;
        VertIndex < NumVerts;
        VertIndex += FaceVerts )
  {
    v3 VertSrc0 = VertsPositions[VertIndex + 0];
    v3 VertSrc1 = VertsPositions[VertIndex + 1];
    v3 VertSrc2 = VertsPositions[VertIndex + 2];
    v3 VertSrc3 = VertsPositions[VertIndex + 3];
    v3 VertSrc4 = VertsPositions[VertIndex + 4];
    v3 VertSrc5 = VertsPositions[VertIndex + 5];

    f32_reg Vert0;
    f32_reg Vert1;
    f32_reg Vert2;
    f32_reg Vert3;
    f32_reg Vert4;
    f32_reg Vert5;

    Vert0.Sse = _mm_set_ps(0, VertSrc0.z, VertSrc0.y, VertSrc0.x);
    Vert1.Sse = _mm_set_ps(0, VertSrc1.z, VertSrc1.y, VertSrc1.x);
    Vert2.Sse = _mm_set_ps(0, VertSrc2.z, VertSrc2.y, VertSrc2.x);
    Vert3.Sse = _mm_set_ps(0, VertSrc3.z, VertSrc3.y, VertSrc3.x);
    Vert4.Sse = _mm_set_ps(0, VertSrc4.z, VertSrc4.y, VertSrc4.x);
    Vert5.Sse = _mm_set_ps(0, VertSrc5.z, VertSrc5.y, VertSrc5.x);

    Vert0.Sse = _mm_add_ps( _mm_mul_ps(Vert0.Sse, mmScale), mmOffset);
    Vert1.Sse = _mm_add_ps( _mm_mul_ps(Vert1.Sse, mmScale), mmOffset);
    Vert2.Sse = _mm_add_ps( _mm_mul_ps(Vert2.Sse, mmScale), mmOffset);
    Vert3.Sse = _mm_add_ps( _mm_mul_ps(Vert3.Sse, mmScale), mmOffset);
    Vert4.Sse = _mm_add_ps( _mm_mul_ps(Vert4.Sse, mmScale), mmOffset);
    Vert5.Sse = _mm_add_ps( _mm_mul_ps(Vert5.Sse, mmScale), mmOffset);

    v3 Result0 = {{ Vert0.F[0], Vert0.F[1], Vert0.F[2] }};
    v3 Result1 = {{ Vert1.F[0], Vert1.F[1], Vert1.F[2] }};
    v3 Result2 = {{ Vert2.F[0], Vert2.F[1], Vert2.F[2] }};
    v3 Result3 = {{ Vert3.F[0], Vert3.F[1], Vert3.F[2] }};
    v3 Result4 = {{ Vert4.F[0], Vert4.F[1], Vert4.F[2] }};
    v3 Result5 = {{ Vert5.F[0], Vert5.F[1], Vert5.F[2] }};

    Dest->VertexData[Dest->VertsFilled + 0] = Result0;
    Dest->VertexData[Dest->VertsFilled + 1] = Result1;
    Dest->VertexData[Dest->VertsFilled + 2] = Result2;
    Dest->VertexData[Dest->VertsFilled + 3] = Result3;
    Dest->VertexData[Dest->VertsFilled + 4] = Result4;
    Dest->VertexData[Dest->VertsFilled + 5] = Result5;

    Dest->NormalData[Dest->VertsFilled + 0] = Normals[VertIndex + 0];
    Dest->NormalData[Dest->VertsFilled + 1] = Normals[VertIndex + 1];
    Dest->NormalData[Dest->VertsFilled + 2] = Normals[VertIndex + 2];
    Dest->NormalData[Dest->VertsFilled + 3] = Normals[VertIndex + 3];
    Dest->NormalData[Dest->VertsFilled + 4] = Normals[VertIndex + 4];
    Dest->NormalData[Dest->VertsFilled + 5] = Normals[VertIndex + 5];

    Dest->ColorData[Dest->VertsFilled + 0] = VertColors[VertIndex + 0];
    Dest->ColorData[Dest->VertsFilled + 1] = VertColors[VertIndex + 1];
    Dest->ColorData[Dest->VertsFilled + 2] = VertColors[VertIndex + 2];
    Dest->ColorData[Dest->VertsFilled + 3] = VertColors[VertIndex + 3];
    Dest->ColorData[Dest->VertsFilled + 4] = VertColors[VertIndex + 4];
    Dest->ColorData[Dest->VertsFilled + 5] = VertColors[VertIndex + 5];

    Dest->VertsFilled += FaceVerts;
  }

#else

  // Left this here for futrue benchmarking.  The memcpy path is fastest by ~2x
#if 1
  for ( s32 VertIndex = 0;
        VertIndex < NumVerts;
        ++VertIndex )
  {
    Dest->VertexData[Dest->VertsFilled] = VertsPositions[VertIndex]*Scale + Offset;
    Dest->NormalData[Dest->VertsFilled] = Normals[VertIndex];
    Dest->ColorData[Dest->VertsFilled] = VertColors[VertIndex];
    ++Dest->VertsFilled;
  }
#else
  s32 sizeofData = NumVerts * sizeof(v3);
  memcpy( &Dest->VertexData[Dest->VertsFilled],  VertsPositions,  sizeofData );
  memcpy( &Dest->NormalData[Dest->VertsFilled],  Normals,         sizeofData );
  memcpy( &Dest->ColorData[Dest->VertsFilled],   VertColors,      sizeofData );
  Dest->VertsFilled += NumVerts;
#endif


#endif


  return;
}

#if 1
inline void
BufferVertsChecked(
    mesh_buffer_target *Target,

    g_buffer_render_group *gBuffer,
    shadow_render_group *SG,
    camera *Camera,

    s32 NumVerts,

    v3* VertsPositions,
    v3* Normals,
    const v3* VertColors,
    v3 Offset = V3(0),
    v3 Scale = V3(1)
  )
{
  TIMED_FUNCTION();

  if ( Target->VertsFilled + NumVerts > Target->VertsAllocated )
  {
    Warn("Flushing %d/%d Verts to gBuffer", Target->VertsFilled, Target->VertsAllocated);
    RenderGBuffer(Target, gBuffer, SG, Camera);
    return;
  }

  BufferVertsDirect( Target, NumVerts, VertsPositions, Normals, VertColors, Offset, Scale);

  return;
}
#endif

inline void
BufferVerts(
    mesh_buffer_target *Source,
    mesh_buffer_target *Dest,

    v3 RenderOffset,

    g_buffer_render_group *gBuffer,
    shadow_render_group *SG,
    camera *Camera,
    r32 Scale
  )
{
  TIMED_FUNCTION();

#if 1
  BufferVertsChecked(Dest, gBuffer, SG, Camera, Source->VertsFilled, Source->VertexData,
      Source->NormalData, Source->ColorData);
  return;
#else
  for ( s32 VertIndex = 0;
        VertIndex < Source->VertsFilled;
        ++VertIndex )
  {
    v3 XYZ = (Source->VertexData[VertIndex]*Scale) + RenderOffset;

#if 1
    Dest->VertexData[Dest->VertsFilled] =  XYZ;
    Dest->NormalData[Dest->VertsFilled] = Source->NormalData[VertIndex];
    Dest->ColorData[Dest->VertsFilled]  = Source->ColorData[VertIndex];
    ++Dest->VertsFilled;
#else

    BufferVerts(Dest, gBuffer, SG, Camera,
        1,
        &XYZ,
        Source->NormalData + VertIndex,
        Source->ColorData + VertIndex);
#endif

  }
#endif

}

void
BuildEntityMesh(chunk_data *chunk, chunk_dimension Dim)
{
  UnSetFlag(chunk, Chunk_BufferMesh);

  for ( int z = 0; z < Dim.z ; ++z )
  {
    for ( int y = 0; y < Dim.y ; ++y )
    {
      for ( int x = 0; x < Dim.x ; ++x )
      {
        voxel_position LocalVoxelP = Voxel_Position(x,y,z);

        if ( NotFilled( chunk, LocalVoxelP, Dim) )
          continue;

        voxel_position P = Voxel_Position(x,y,z);

        voxel *Voxel = &chunk->Voxels[GetIndex(P, chunk, Dim)];

        v3 VP = V3(P);
        v3 Diameter = V3(1.0f);
        v3 VertexData[6];

        v3 FaceColors[FACE_VERT_COUNT];
        FillColorArray(Voxel->Color, FaceColors, FACE_VERT_COUNT);

        voxel_position rightVoxel = LocalVoxelP + Voxel_Position(1, 0, 0);
        voxel_position leftVoxel = LocalVoxelP - Voxel_Position(1, 0, 0);

        voxel_position topVoxel = LocalVoxelP + Voxel_Position(0, 0, 1);
        voxel_position botVoxel = LocalVoxelP - Voxel_Position(0, 0, 1);

        voxel_position frontVoxel = LocalVoxelP + Voxel_Position(0, 1, 0);
        voxel_position backVoxel = LocalVoxelP - Voxel_Position(0, 1, 0);



        if ( (!IsInsideDim(Dim, rightVoxel)) || NotFilled( chunk, rightVoxel, Dim))
        {
          RightFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, RightFaceNormalData, FaceColors);
        }
        if ( (!IsInsideDim( Dim, leftVoxel  )) || NotFilled( chunk, leftVoxel, Dim))
        {
          LeftFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, LeftFaceNormalData, FaceColors);
        }
        if ( (!IsInsideDim( Dim, botVoxel   )) || NotFilled( chunk, botVoxel, Dim))
        {
          BottomFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, BottomFaceNormalData, FaceColors);
        }
        if ( (!IsInsideDim( Dim, topVoxel   )) || NotFilled( chunk, topVoxel, Dim))
        {
          TopFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, TopFaceNormalData, FaceColors);
        }
        if ( (!IsInsideDim( Dim, frontVoxel )) || NotFilled( chunk, frontVoxel, Dim))
        {
          FrontFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, FrontFaceNormalData, FaceColors);
        }
        if ( (!IsInsideDim( Dim, backVoxel  )) || NotFilled( chunk, backVoxel, Dim))
        {
          BackFaceVertexData( VP, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, BackFaceNormalData, FaceColors);
        }

      }
    }
  }
}

void
BuildWorldChunkMesh(world *World, world_chunk *WorldChunk, chunk_dimension WorldChunkDim)
{
  TIMED_FUNCTION();

  chunk_data *chunk = WorldChunk->Data;

  UnSetFlag( chunk, Chunk_BufferMesh );

  for ( int z = 0; z < WorldChunkDim.z ; ++z )
  {
    for ( int y = 0; y < WorldChunkDim.y ; ++y )
    {
      for ( int x = 0; x < WorldChunkDim.x ; ++x )
      {
        canonical_position CurrentP  = Canonical_Position(WorldChunkDim, V3(x,y,z), WorldChunk->WorldP);

        if ( NotFilledInWorld( World, WorldChunk, CurrentP ) )
          continue;

        voxel *Voxel = &chunk->Voxels[GetIndex(CurrentP.Offset, chunk, WorldChunkDim)];

        v3 Diameter = V3(1.0f);
        v3 VertexData[FACE_VERT_COUNT];
        v3 FaceColors[FACE_VERT_COUNT];
        FillColorArray(Voxel->Color, FaceColors, FACE_VERT_COUNT);;

        canonical_position rightVoxel = Canonicalize(WorldChunkDim, CurrentP + V3(1, 0, 0));
        canonical_position leftVoxel  = Canonicalize(WorldChunkDim, CurrentP - V3(1, 0, 0));

        canonical_position topVoxel   = Canonicalize(WorldChunkDim, CurrentP + V3(0, 0, 1));
        canonical_position botVoxel   = Canonicalize(WorldChunkDim, CurrentP - V3(0, 0, 1));

        canonical_position frontVoxel = Canonicalize(WorldChunkDim, CurrentP + V3(0, 1, 0));
        canonical_position backVoxel  = Canonicalize(WorldChunkDim, CurrentP - V3(0, 1, 0));

        if ( NotFilledInWorld( World, WorldChunk, rightVoxel ) )
        {
          RightFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, RightFaceNormalData, FaceColors);
        }
        if ( NotFilledInWorld( World, WorldChunk, leftVoxel ) )
        {
          LeftFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, LeftFaceNormalData, FaceColors);
        }
        if ( NotFilledInWorld( World, WorldChunk, botVoxel   ) )
        {
          BottomFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, BottomFaceNormalData, FaceColors);
        }
        if ( NotFilledInWorld( World, WorldChunk, topVoxel   ) )
        {
          TopFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, TopFaceNormalData, FaceColors);
        }
        if ( NotFilledInWorld( World, WorldChunk, frontVoxel ) )
        {
          FrontFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, FrontFaceNormalData, FaceColors);
        }
        if ( NotFilledInWorld( World, WorldChunk, backVoxel  ) )
        {
          BackFaceVertexData( CurrentP.Offset, Diameter, VertexData);
          BufferVertsDirect(&chunk->Mesh, 6, VertexData, BackFaceNormalData, FaceColors);
        }

      }
    }
  }
}




#endif
