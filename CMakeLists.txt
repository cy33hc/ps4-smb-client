cmake_minimum_required(VERSION 3.0)

project(ps4_smb_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDONT_HAVE_STRUPR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

add_executable(ps4_smb_client
  source/actions.cpp
  source/config.cpp
  source/fs.cpp
  source/gui.cpp
  source/http_request.cpp
  source/ime_dialog.cpp
  source/inifile.c
  source/installer.cpp
  source/lang.cpp
  source/main.cpp
  source/orbis_jbc.c
  source/windows.cpp
  source/smbclient.cpp
  source/imgui_draw.cpp
  source/imgui_impl_sdl.cpp
  source/imgui_impl_sdlrenderer.cpp
  source/imgui_tables.cpp
  source/imgui_widgets.cpp
  source/imgui.cpp
)

add_self(ps4_smb_client)

add_pkg(ps4_smb_client ${CMAKE_SOURCE_DIR}/data "SMBC00001" "PS4 SMB Client" "01.02" 32 0)

target_link_libraries(ps4_smb_client
  dbglogger
  c
  c++
  png
  z
  pthread
  SDL2
  samplerate
  jbc
  smb2
  kernel
  SceShellCoreUtil
  SceSysmodule
  SceSystemService
  ScePigletv2VSH
  ScePrecompiledShaders
  SceRtc
  SceUserService
  ScePad
  SceAudioOut
  SceSysUtil
  SceImeDialog
  SceNet
  SceBgft
  SceAppInstUtil
)
