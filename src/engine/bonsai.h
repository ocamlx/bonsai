
struct debug_state;

struct world;
struct heap_allocator;
struct entity;

#define MAX_PICKED_WORLD_CHUNKS (64)

// TODO(Jesse)(metaprogramming, ptr): Once poof can accept pointer types we can generate this struct
/* poof(static_buffer(world_chunk*, 64)) */
/* #include <generated/buffer_world_chunk.h> */
struct picked_world_chunk_static_buffer
{
  picked_world_chunk E[MAX_PICKED_WORLD_CHUNKS];
  u64 At;
};

struct engine_debug
{
  picked_world_chunk_static_buffer PickedChunks;
};

link_internal void
Push(picked_world_chunk_static_buffer *Buf, world_chunk *Chunk, r32 t)
{
  if (Buf->At < MAX_PICKED_WORLD_CHUNKS)
  {
    Buf->E[Buf->At].Chunk = Chunk;
    Buf->E[Buf->At].tChunk = t;

    ++Buf->At;
  }
}

struct engine_resources
{
  os         *Os;
  platform   *Plat;
  graphics   *Graphics;
  hotkeys    *Hotkeys;

  world      *World;
  game_state *GameState;

  thread_local_state *ThreadStates;

  heap_allocator Heap;
  memory_arena *Memory;

  entity **EntityTable;

  u64 FrameIndex;

  // TODO(Jesse): Formalize this
  /* world_position *VisibleRegion; */

  // TODO(Jesse): Put this on the camera?
  canonical_position *CameraTargetP;

  mesh_freelist MeshFreelist;

  renderer_2d GameUiRenderer;

  engine_debug EngineDebug;
  debug_state *DebugState;
};

#define UNPACK_ENGINE_RESOURCES(Res)                                      \
  platform                  *Plat          =  Res->Plat;            \
  world                     *World         =  Res->World;           \
  game_state                *GameState     =  Res->GameState;       \
  memory_arena              *Memory        =  Res->Memory;          \
  heap_allocator            *Heap          = &Res->Heap;            \
  entity                   **EntityTable   =  Res->EntityTable;     \
  hotkeys                   *Hotkeys       =  Res->Hotkeys;         \
  engine_debug              *EngineDebug   = &Res->EngineDebug;     \
  mesh_freelist             *MeshFreelist  = &Res->MeshFreelist;    \
  input                     *Input         = &Res->Plat->Input;     \
  graphics                  *Graphics      =  Res->Graphics;        \
  renderer_2d               *GameUi        = &Res->GameUiRenderer;  \
  gpu_mapped_element_buffer *GpuMap        =  GetCurrentGpuMap(Graphics); \
  g_buffer_render_group     *gBuffer       =  Graphics->gBuffer;          \
  camera                    *Camera        =  Graphics->Camera;








enum frame_event_type
{
  FrameEvent_Undefined,

  FrameEvent_Explosion,
  FrameEvent_GameModeLoss,
  FrameEvent_GameModePlaying,
  FrameEvent_GameModeWon,
};

enum game_mode_type
{
  GameMode_Title,
  GameMode_Playing,
  GameMode_Won,
  GameMode_Loss,
};

struct game_mode
{
  game_mode_type ActiveMode;
  r64 TimeRunning;
};

enum entity_state
{
  EntityState_Free        = 0,
  EntityState_Spawned     = 1 << 0,
  EntityState_Destroyed   = 1 << 1,
  EntityState_Reserved    = 1 << 2,
};

enum entity_type
{
  EntityType_None             = 0,

  EntityType_Player           = 1 << 0,
  EntityType_Enemy            = 1 << 1,
  EntityType_EnemyProjectile  = 1 << 2,
  EntityType_PlayerProjectile = 1 << 3,
  EntityType_Loot             = 1 << 4,
  EntityType_PlayerProton     = 1 << 5,
  EntityType_ParticleSystem   = 1 << 6,

  EntityType_Static           = 1 << 7,

  EntityType_Default           = 1 << 8, // Nothing special about it, just needed an entity
};

global_variable const entity_type ENTITY_TYPES = (entity_type)
  ( EntityType_Player           |
    EntityType_Enemy            |
    EntityType_EnemyProjectile  |
    EntityType_PlayerProjectile |
    EntityType_Loot             |
    EntityType_PlayerProton     |
    EntityType_ParticleSystem
   );

enum collision_type
{
  Collision_Player_Enemy            = EntityType_Player | EntityType_Enemy,
  Collision_Player_EnemyProjectile  = EntityType_Player | EntityType_EnemyProjectile,
  Collision_Player_PlayerProjectile = EntityType_Player | EntityType_PlayerProjectile,
  Collision_Player_Loot             = EntityType_Player | EntityType_Loot,
  Collision_Enemy_PlayerProjectile  = EntityType_Enemy  | EntityType_PlayerProjectile,
  Collision_Enemy_PlayerProton      = EntityType_Enemy  | EntityType_PlayerProton,
  Collision_Enemy_EnemyProjectile   = EntityType_Enemy  | EntityType_EnemyProjectile,
  Collision_Enemy_Enemy             = EntityType_Enemy,
};

enum model_index
{
  ModelIndex_None,

  ModelIndex_Enemy_Skeleton_Axe,
  ModelIndex_Enemy_Skeleton_Sword,
  ModelIndex_Enemy_Skeleton_Lasher,
  ModelIndex_Enemy_Skeleton_Archer,
  ModelIndex_Enemy_Skeleton_Spear,
  ModelIndex_Enemy_Skeleton_AxeArmor,
  ModelIndex_Enemy_Skeleton_Hounds,
  ModelIndex_Enemy_Skeleton_Horserider,
  ModelIndex_Enemy_Skeleton_Horsebanner,
  ModelIndex_Enemy_Skeleton_Shaman,
  ModelIndex_Enemy_Skeleton_Champion,
  ModelIndex_Enemy_Skeleton_ChampionChampion,
  ModelIndex_Enemy_Skeleton_Concubiner,
  ModelIndex_Enemy_Skeleton_King,

  ModelIndex_FirstEnemyModel = ModelIndex_Enemy_Skeleton_Axe,
  ModelIndex_LastEnemyModel = ModelIndex_Enemy_Skeleton_King,

  ModelIndex_Player_jp,
  ModelIndex_Player_bow,
  ModelIndex_Player_cat,
  ModelIndex_Player_fox,
  ModelIndex_Player_gumi,
  ModelIndex_Player_knight,
  ModelIndex_Player_man,
  ModelIndex_Player_mom,
  ModelIndex_Player_old,
  ModelIndex_Player_poem,
  ModelIndex_Player_rain,
  ModelIndex_Player_sasami,
  ModelIndex_Player_sol,
  ModelIndex_Player_sword,
  ModelIndex_Player_tale,
  ModelIndex_Player_tama,
  ModelIndex_Player_tsurugi,

  ModelIndex_FirstPlayerModel = ModelIndex_Player_jp,
  ModelIndex_LastPlayerModel = ModelIndex_Player_tsurugi,

  ModelIndex_Loot,
  ModelIndex_Projectile,
  ModelIndex_Proton,
  ModelIndex_Bitty0,
  ModelIndex_Bitty1,

  ModelIndex_Level,

  ModelIndex_Count,
};

struct model
{
  untextured_3d_geometry_buffer Mesh;
  chunk_dimension Dim;
  animation Animation;

  /* v4 *Palette; // Optional */
};

struct physics
{
  v3 Velocity;
  v3 Force;
  v3 Delta;

  /* v3 Drag; */
  r32 Mass;

  r32 Speed;
};

struct particle
{
  // TODO(Jesse, id: 85, tags: robustness, memory_consumption): Compress to 16 bit float?
  v3 Velocity;
  v3 Offset;

  /* physics Physics; */

  u8 Color;
  r32 RemainingLifespan;
};

enum particle_spawn_type
{
  ParticleSpawnType_None,

  ParticleSpawnType_Random, // Particles spawn with random velocity
  ParticleSpawnType_Expanding, // Spawn velocity pointing away from center of spawn region
  ParticleSpawnType_Contracting, // Spawn velocity pointing towards center of spawn region
};

// TODO(Jesse)(metaprogramming): Make a struct paste thing such that we can
// just splat the init params into here

#define PARTICLE_SYSTEM_COLOR_COUNT 6
#define PARTICLES_PER_SYSTEM   (4096)
struct particle_system
{
  random_series Entropy;

  particle_spawn_type SpawnType;

  r32 Drag;
  r32 EmissionLifespan; // How long the system emits for

  u32 ActiveParticles;

  r32 LifespanMod;
  r32 ParticleLifespan; // How long an individual particle lasts
  r32 ParticlesPerSecond;

  v3 ParticleStartingDim;
  f32 ParticleEndingDim;

  v3 ParticleTurbMin;
  v3 ParticleTurbMax;

  aabb SpawnRegion;

  r32 SystemMovementCoefficient;

  u8 Colors[PARTICLE_SYSTEM_COLOR_COUNT];

  r32 ElapsedSinceLastEmission;
  particle Particles[PARTICLES_PER_SYSTEM];
};

struct entity;
typedef void (*update_callback)(engine_resources *, entity *);

struct entity
{
  model Model;
  v3 CollisionVolumeRadius;

  particle_system* Emitter;

  physics Physics;

  canonical_position P;

  Quaternion Rotation;

  entity_state State;
  entity_type Type;

  r32 Scale;

   // TODO(Jesse, id: 86, tags: memory_consumption, entity): Unneeded for projectiles. factor out of here?
  r32 RateOfFire;
  r32 FireCooldown;

  s32 Health;

  update_callback Update;
  void* UserData;
};

struct frame_event
{
  frame_event_type Type;
  entity *Entity;

  frame_event *Next;

  frame_event(frame_event_type Type_in)
  {
    this->Type = Type_in;
    this->Entity = 0;
    this->Next = 0;
  }

  frame_event(entity *Entity_in, frame_event_type Type_in)
  {
    this->Type = Type_in;
    this->Entity = Entity_in;
    this->Next = 0;
  }
};

struct event_queue
{
  u64 CurrentFrameIndex;
  frame_event **Queue;

  frame_event *FirstFreeEvent;
};

inline frame_event*
GetFreeFrameEvent(event_queue *Queue)
{
  frame_event *FreeEvent = Queue->FirstFreeEvent;

  if (FreeEvent)
  {
    Queue->FirstFreeEvent = FreeEvent->Next;
    FreeEvent->Next = 0;
  }

  return FreeEvent;
}

struct entity_list
{
  entity *This;
  entity *Next;
};

#define POINT_BUFFER_SIZE (12)
struct point_buffer
{
  s32 Count;
  voxel_position Points[POINT_BUFFER_SIZE];

  voxel_position Min;
  voxel_position Max;
};

struct collision_event
{
  u32 Count;
  canonical_position MinP;
  canonical_position MaxP;
};

inline void
UnSetFlag( voxel_flag *Flags, voxel_flag Flag )
{
  *Flags = (voxel_flag)(*Flags & ~Flag);
  return;
}

inline void
UnSetFlag( chunk_flag *Flags, chunk_flag Flag )
{
  *Flags = (chunk_flag)(*Flags & ~Flag);
  return;
}

inline void
UnSetFlag( world_chunk *Chunk, chunk_flag Flag )
{
  UnSetFlag(&Chunk->Flags, Flag);
  return;
}

/* inline void */
/* UnSetFlag( world_chunk *Chunk, chunk_flag Flag ) */
/* { */
/*   UnSetFlag(Chunk->Data, Flag); */
/*   return; */
/* } */

inline void
SetFlag( u8 *Flags, voxel_flag Flag )
{
  /* Assert(Flag < u8_MAX); */
  *Flags = (u8)(*Flags | Flag);
  return;
}

inline void
SetFlag( chunk_flag *Flags, chunk_flag Flag )
{
  *Flags = (chunk_flag)(*Flags | Flag);
  return;
}

/* inline void */
/* SetFlag( chunk_data *Chunk, chunk_flag Flag ) */
/* { */
/*   SetFlag(&Chunk->Flags, Flag); */
/*   return; */
/* } */

inline void
SetFlag( world_chunk *Chunk, chunk_flag Flag )
{
  Chunk->Flags = (chunk_flag)(Chunk->Flags | Flag);
  return;
}

inline void
SetFlag(voxel *Voxel, voxel_flag Flag )
{
  SetFlag(&Voxel->Flags, Flag);
  return;
}

inline void
SetFlag(boundary_voxel *Voxel, voxel_flag Flag )
{
  SetFlag(&Voxel->V.Flags, Flag);
  return;
}

inline b32
IsSet( u8 Flags, voxel_flag Flag )
{
  b32 Result = ( (Flags & Flag) != 0 );
  return Result;
}

inline b32
IsSet( chunk_flag Flags, chunk_flag Flag )
{
  b32 Result = ( (Flags & Flag) != 0 );
  return Result;
}

/* inline b32 */
/* IsSet( chunk_data *C, chunk_flag Flag ) */
/* { */
/*   b32 Result = IsSet(C->Flags, Flag); */
/*   return Result; */
/* } */

inline b32
IsSet( world_chunk *Chunk, chunk_flag Flag )
{
  b32 Result = IsSet(Chunk->Flags, Flag);
  return Result;
}

inline b32
IsSet( voxel *V, voxel_flag Flag )
{
  b32 Result = IsSet(V->Flags, Flag);
  return Result;
}

inline b32
IsSet( boundary_voxel *V, voxel_flag Flag )
{
  b32 Result = IsSet(&V->V, Flag);
  return Result;
}

inline b32
NotSet( voxel_flag Flags, voxel_flag Flag )
{
  b32 Result = !(IsSet(Flags, Flag));
  return Result;
}

inline b32
NotSet( chunk_flag Flags, chunk_flag Flag )
{
  b32 Result = !(IsSet(Flags, Flag));
  return Result;
}

/* inline b32 */
/* NotSet( chunk_data *Chunk, chunk_flag Flag ) */
/* { */
/*   b32 Result = !(IsSet(Chunk, Flag)); */
/*   return Result; */
/* } */

inline b32
NotSet( world_chunk *Chunk, chunk_flag Flag )
{
  b32 Result = !(IsSet(Chunk, Flag));
  return Result;
}

inline b32
NotSet( voxel *Voxel, voxel_flag Flag )
{
  b32 Result = !(IsSet(Voxel, Flag));
  return Result;
}

inline b32
Spawned(entity *Entity)
{
  b32 Result = Entity->State == EntityState_Spawned;
  return Result;
}

inline b32
Active(particle_system *System)
{
  b32 Result = (System->EmissionLifespan > 0) || (System->ActiveParticles > 0);
  return Result;
}

inline b32
Inactive(particle_system *System)
{
  b32 Result = !Active(System);
  return Result;
}

inline b32
Reserved(entity *Entity)
{
  b32 Result = Entity->State == EntityState_Reserved;
  return Result;
}

inline b32
Destroyed(entity *Entity)
{
  b32 Result = Entity->State == EntityState_Destroyed;
  return Result;
}

inline b32
Unspawned(entity *Entity)
{
  b32 Result = !Spawned(Entity);
  return Result;
}

#if 0
typedef umm packed_voxel;
typedef umm unpacked_voxel;

inline u8
GetVoxelColor(packed_voxel *V)
{
  u8 Color = (V->Data >> (FINAL_POSITION_BIT) ) & ~( 0xFFFFFFFF << (COLOR_BIT_WIDTH));

  Assert(Color < PALETTE_SIZE);
  return Color;
}

inline void
SetVoxelColor(packed_voxel *Voxel, int w)
{
  u32 flagMask = (0xFFFFFFFF << FINAL_COLOR_BIT);
  u32 colorMask = ( flagMask | ~(0xFFFFFFFF << (FINAL_POSITION_BIT)) );

  u32 currentFlags = Voxel->Data & colorMask;

  Voxel->Data = currentFlags;
  Voxel->Data |= (w << (FINAL_POSITION_BIT));

  u8 color = GetVoxelColor(Voxel);
  Assert(color == w);
}

inline voxel_position
GetVoxelP(packed_voxel *V)
{
  voxel_position P = Voxel_Position(
    V->Data >> (POSITION_BIT_WIDTH * 0) & 0x000000FF >> (8 - POSITION_BIT_WIDTH),
    V->Data >> (POSITION_BIT_WIDTH * 1) & 0x000000FF >> (8 - POSITION_BIT_WIDTH),
    V->Data >> (POSITION_BIT_WIDTH * 2) & 0x000000FF >> (8 - POSITION_BIT_WIDTH)
  );

  return P;
}

inline void
SetVoxelP(packed_voxel *Voxel, voxel_position P)
{
  Assert( P.x < Pow2(POSITION_BIT_WIDTH) );
  Assert( P.y < Pow2(POSITION_BIT_WIDTH) );
  Assert( P.z < Pow2(POSITION_BIT_WIDTH) );

  int currentFlags = ( Voxel->Data & (0xFFFFFFFF << FINAL_POSITION_BIT));
  Voxel->Data = currentFlags;

  Voxel->Data |= P.x << (POSITION_BIT_WIDTH * 0);
  Voxel->Data |= P.y << (POSITION_BIT_WIDTH * 1);
  Voxel->Data |= P.z << (POSITION_BIT_WIDTH * 2);

  voxel_position SetP = GetVoxelP(Voxel);
  Assert(SetP == P);

  return;
}

inline packed_voxel
PackVoxel(unpacked_voxel *V)
{
  packed_voxel Result = {};

  Result.Data = V->Flags; // Must come first

  SetVoxelP(&Result, V->Offset);
  SetVoxelColor(&Result, V->ColorIndex);

  Result.Data = SetFlag(Result.Data, Voxel_Filled);

  return Result;
}

inline unpacked_voxel
GetUnpackedVoxel(int x, int y, int z, int w)
{
  unpacked_voxel V;

  V.Offset = Voxel_Position(x,y,z);
  V.ColorIndex = w;
  V.Flags = (voxel_flag)0;

  return V;
}

inline packed_voxel
GetPackedVoxel(int x, int y, int z, int w)
{
  packed_voxel Result = {};
  voxel_position P = Voxel_Position(x,y,z);

  SetVoxelP(&Result, P );
  SetVoxelColor(&Result, w);

  Assert(GetVoxelP(&Result) == P);
  Assert(GetVoxelColor(&Result) == w);

  return Result;
}

#endif

void
ZeroMesh( untextured_3d_geometry_buffer *Mesh )
{
  Mesh->At = 0;
  return;
}

void
ClearWorldChunk( world_chunk *Chunk )
{
  Chunk->Flags = {};
  Chunk->WorldP = {};
  Chunk->FilledCount = {};
  Chunk->Picked = {};
  Chunk->DrawBoundingVoxels = {};
  Chunk->PointsToLeaveRemaining = {};
  Chunk->TriCount = {};
  Chunk->EdgeBoundaryVoxelCount = {};
}

inline s32
GetIndexUnsafe(voxel_position P, chunk_dimension Dim)
{
  s32 i =
    (P.x) +
    (P.y*Dim.x) +
    (P.z*Dim.x*Dim.y);

  return i;
}

inline s32
TryGetIndex(chunk_dimension P, chunk_dimension Dim)
{
  s32 Result = -1;
  if (P.x >= 0 && P.y >= 0 && P.z >= 0 &&
      P.x < Dim.x && P.y < Dim.y && P.z < Dim.z)
  {
    Result = P.x +
            (P.y*Dim.x) +
            (P.z*Dim.x*Dim.y);
    Assert(Result >= 0);
  }

  Assert(Result < Volume(Dim));
  return Result;
}

inline s32
GetIndex(s32 X, s32 Y, s32 Z, chunk_dimension Dim)
{
  Assert(X >= 0);
  Assert(Y >= 0);
  Assert(Z >= 0);

  Assert(X < Dim.x);
  Assert(Y < Dim.y);
  Assert(Z < Dim.z);

  s32 Result = X +
              (Y*Dim.x) +
              (Z*Dim.x*Dim.y);

  Assert(Result >= 0);
  Assert(Result < Volume(Dim));

  return Result;
}

inline s32
GetIndex(voxel_position P, chunk_dimension Dim)
{
  s32 Result = GetIndex(P.x, P.y, P.z, Dim);
  return Result;
}

inline s32
GetIndex(v3 Offset, chunk_dimension Dim)
{
  s32 Index = GetIndex( Voxel_Position(Offset), Dim);
  return Index;
}

inline s32
GetIndexUnsafe(s32 X, s32 Y, s32 Z, chunk_dimension Dim)
{
  s32 Result = X +
              (Y*Dim.x) +
              (Z*Dim.x*Dim.y);

  Assert(Result >= 0);
  return Result;
}


inline voxel_position
V3iFromIndex(s32 Index, chunk_dimension Dim)
{
  Assert(Index >= 0);
  int x = Index % Dim.x;
  int y = (Index/Dim.x) % Dim.y;
  int z = Index / (Dim.x*Dim.y);

  // TODO(Jesse): Should this acutally not be strictly less than ..?
  Assert(x < Dim.x);
  Assert(y < Dim.y);
  Assert(z < Dim.z);

  voxel_position Result = Voxel_Position(x,y,z);
  return Result;
}

inline voxel_position
GetPosition(s32 Index, chunk_dimension Dim)
{
  auto Result = V3iFromIndex(Index, Dim);
  return Result;
}

inline b32
IsFilled(voxel *Voxel)
{
  b32 Result = (Voxel->Flags & Voxel_Filled) == Voxel_Filled;

#if BONSAI_INTERNAL
  if (!Result) Assert( (Voxel->Flags&VoxelFaceMask) == 0);
#endif
  return Result;
}

inline b32
NotFilled(voxel *Voxel)
{
  b32 Result = !IsFilled(Voxel);
  return Result;
}

inline b32
IsFilled( voxel *Voxels, voxel_position VoxelP, chunk_dimension Dim)
{
  s32 VoxelIndex = GetIndex(VoxelP, Dim);
  b32 isFilled = IsSet(Voxels + VoxelIndex, Voxel_Filled);
  return isFilled;
}

inline b32
NotFilled(voxel *Voxels, voxel_position VoxelP, chunk_dimension Dim)
{
  b32 Result = !IsFilled(Voxels, VoxelP, Dim);
  return Result;
}

inline voxel*
GetVoxel( world_chunk* Chunk, voxel_position VoxelP)
{
  s32 VoxelIndex = GetIndex(VoxelP, Chunk->Dim);
  voxel *Result = Chunk->Voxels + VoxelIndex;
  return Result;
}

inline world_position
GetAbsoluteP( world_position P, chunk_dimension WorldChunkDim)
{
  world_position Result = World_Position((WorldChunkDim.x*P.x), (WorldChunkDim.y*P.y), (WorldChunkDim.z*P.z));
  return Result;
}

// FIXME(Jesse): this is misguided for-sure.  Will certainly fail when entities
// are far from the origin due to floating point percision issues.  Change
// to a view-space computation.
inline v3
GetAbsoluteP( canonical_position CP, chunk_dimension WorldChunkDim)
{
  v3 Result = V3(CP.Offset.x+(r32)(WorldChunkDim.x*CP.WorldP.x),
                 CP.Offset.y+(r32)(WorldChunkDim.y*CP.WorldP.y),
                 CP.Offset.z+(r32)(WorldChunkDim.z*CP.WorldP.z));
  return Result;
}

#if 0
inline aabb
GetAABB(entity *Entity, chunk_dimension WorldChunkDim)
{
  v3 Radius = Entity->CollisionVolumeRadius;
  v3 Center = GetAbsoluteP(Entity->P, WorldChunkDim) + Radius;

  aabb Result(Center, Radius);
  return Result;
}
#endif

inline b32
Contains( voxel_position Dim, voxel_position P )
{
  b32 Result = ( P.x >= 0    && P.y >= 0    && P.z >= 0 &&
                 P.x < Dim.x && P.y < Dim.y && P.z < Dim.z );
  return Result;
}

inline b32
IsInsideDim( voxel_position Dim, voxel_position P )
{
  b32 Result = Contains(Dim, P);
  return Result;
}

inline b32
IsInsideDim( voxel_position Dim, v3 P )
{
  b32 Result = IsInsideDim(Dim, Voxel_Position(P) );
  return Result;
}
