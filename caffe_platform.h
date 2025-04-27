//
// caf_platform.h
//
// Caleb Barger
// 11/28/2024
//

#ifndef CAF_PLATFORM_H

typedef struct File File;
struct File
{
  void* memory;
  U32 size;
};

typedef struct GameBuffer GameBuffer;
struct GameBuffer
{
  void* buffer;
  U32 offscreen_width; // Dimensions of pixel buffer
  U32 offscreen_height;

  U32 width; // Dimensions of window
  U32 height;
};

typedef U8 Button;
enum
{
  Button_none = 0,

  Button_a,
  Button_d,
  Button_w,
  Button_s,
  Button_e,
  Button_r,
  Button_t,
  Button_up,
  Button_down,
  Button_left,
  Button_right,
  Button_enter,

  Button_mouse_left,
  Button_mouse_right,

  Button_COUNT
};

typedef struct MouseState MouseState;
struct MouseState
{
  B8 mouse1;
  B8 mouse2;
  Vec2_S32 p;
};

typedef struct ButtonState ButtonState;
struct ButtonState
{
  union
  {
    struct
    {
      B8 none;

      B8 a;
      B8 d;
      B8 w;
      B8 s;
      B8 e;
      B8 r;
      B8 t;
      B8 up;
      B8 down;
      B8 left;
      B8 right;
      B8 enter;

      B8 mouse_left;
      B8 mouse_right;
    };
    B8 buttons[Button_COUNT];
  };
};

typedef struct GameController GameController;
struct GameController
{
  U32 mousex;
  U32 mousey;
  ButtonState started_down;
  ButtonState ended_down;
};

typedef struct GameInput GameInput;
struct GameInput
{
  GameController controller;
  F64 time_now_ms;
  F64 last_time_ms;
  U64 tick;
  U32 target_fps;
};

typedef struct GameSoundOutputBuffer GameSoundOutputBuffer;
struct GameSoundOutputBuffer
{
  U32 samples_per_second;
  U32 sample_count;
  S16* samples;
};

////////////////////////////////
//~ calebarg: Services platform provides to the game

#define DEBUG_FILE_SIZE(name) U32 name(U8* file_pathz)
typedef DEBUG_FILE_SIZE(DEBUGFileSize);

#define DEBUG_READ_ENTIRE_FILE(name) File name(String8 file_pathz)
typedef DEBUG_READ_ENTIRE_FILE(DEBUGReadEntireFile);

#define DEBUG_FREE_ENTIRE_FILE(name) void name(File* file)
typedef DEBUG_FREE_ENTIRE_FILE(DEBUGFreeEntireFile);

#define LOAD_FUNCTION(name) void* name(void* handle, char* func)
typedef LOAD_FUNCTION(LoadFunction);

#define VULKAN_MAKE_SURFACE(name) VkSurfaceKHR name(VkInstance instance)
typedef VULKAN_MAKE_SURFACE(VulkanMakeSurface);

#define VULKAN_SURFACE_EXTENSION_NAME(name) U8* name()
typedef VULKAN_SURFACE_EXTENSION_NAME(VulkanSurfaceExtensionName);

typedef struct GameMemory GameMemory;
struct GameMemory
{
  B8 did_init;
  Arena* perm_arena;
  Arena* scratch_arena;
  void* vulkan_code;

  DEBUGFileSize* DEBUG_platform_file_size;
  DEBUGReadEntireFile* DEBUG_platform_read_entire_file;
  DEBUGFreeEntireFile* DEBUG_platform_free_entire_file;
  LoadFunction* platform_load_function;
  VulkanSurfaceExtensionName* platform_vulkan_surface_extension_name;
  VulkanMakeSurface* platform_vulkan_make_surface;
};

////////////////////////////////
//~ calebarg: Services game provides to the platform

#define GAME_UPDATE_AND_RENDER(name) void name(\
                                               GameMemory* game_memory,\
                                               GameBuffer* game_buffer,\
                                               GameInput* game_input)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#define GAME_GET_SOUND_SAMPLES(name) void name(\
                                               GameMemory* game_memory,\
                                               GameSoundOutputBuffer* game_sound)
typedef GAME_GET_SOUND_SAMPLES(GameGetSoundSamples);

#define CAF_PLATFORM_H
#endif
