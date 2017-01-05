cmake_minimum_required(VERSION 2.6)

project(quake2)

if(WIN32)
  set(SDL2_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/win32/SDL2-2.0.5/include)
  set(SDL2_LIBRARIES ${CMAKE_SOURCE_DIR}/win32/SDL2-2.0.5/lib/x86/SDL2.lib ${CMAKE_SOURCE_DIR}/win32/SDL2-2.0.5/lib/x86/SDL2main.lib)
  set(SDL2_VERSION_STRING "2.0.5")
  set(SDL2_FOUND 1)
else()
  include(FindPkgConfig)
  pkg_search_module(SDL2 REQUIRED sdl2)
endif()

include_directories(${SDL2_INCLUDE_DIRS})
link_directories(/opt/vc/lib)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -DC_ONLY -DGAME_HARD_LINKED -DREF_HARD_LINKED)

set(SHARED_SRCS game/q_shared.c game/q_shared.h)

if(WIN32)
  set(SHARED_SRCS ${SHARED_SRCS} win32/q_shwin.c)
else()
  set(SHARED_SRCS ${SHARED_SRCS} linux/glob.c linux/q_shlinux.c)
endif()

add_library(shared ${SHARED_SRCS})

add_subdirectory(game)
#add_subdirectory(ref_gl)
#add_subdirectory(ref_gl2)
add_subdirectory(ref_gles2)

set(Q2_SRCS
  client/cl_cin.c
  client/cl_ents.c
  client/cl_fx.c
  client/cl_input.c
  client/cl_inv.c
  client/cl_main.c
  client/cl_newfx.c
  client/cl_parse.c
  client/cl_pred.c
  client/cl_scrn.c
  client/cl_tent.c
  client/cl_view.c
  client/console.c
  client/keys.c
  client/menu.c
  client/qmenu.c
  client/snd_dma.c
  client/snd_mem.c
  client/snd_mix.c
  client/x86.c
  game/m_flash.c
  qcommon/cmd.c
  qcommon/cmodel.c
  qcommon/common.c
  qcommon/crc.c
  qcommon/cvar.c
  qcommon/files.c
  qcommon/md4.c
  qcommon/net_chan.c
  qcommon/pmove.c
  server/sv_ccmds.c
  server/sv_ents.c
  server/sv_game.c
  server/sv_init.c
  server/sv_main.c
  server/sv_send.c
  server/sv_user.c
  server/sv_world.c
  sdl2.c
  #win32/cd_win.c
  #win32/conproc.c
  #win32/in_win.c
  #win32/snd_win.c
  #win32/sys_win.c
  #win32/vid_dll.c
  #win32/vid_menu.c
  )

if(WIN32)
  set(Q2_SRCS ${Q2_SRCS} win32/net_wins.c)
else()
  set(Q2_SRCS ${Q2_SRCS} linux/net_udp.c)
endif()

set(Q2_INCS
  client/anorms.h
  client/cdaudio.h
  client/client.h
  client/console.h
  client/input.h
  client/keys.h
  client/qmenu.h
  client/ref.h
  client/screen.h
  client/snd_loc.h
  client/sound.h
  client/vid.h
  game/game.h
  game/q_shared.h
  #qcommon/bspfile.h
  qcommon/qcommon.h
  qcommon/qfiles.h
  server/server.h
  #win32/conproc.h
  #win32/winquake.h
  )

if(WIN32)
  add_executable(Quake2 WIN32 ${Q2_SRCS} ${Q2_INCS} win32/q2.rc win32/q2.ico)
  target_link_libraries(Quake2 game_static ref_gles2_static shared ${SDL2_LIBRARIES} wsock32 winmm)
else()
  add_executable(Quake2 ${Q2_SRCS} ${Q2_INCS})
  target_link_libraries(Quake2 game_static ref_gles2_static shared ${SDL2_LIBRARIES} m)
  #target_link_libraries(Quake2 game_static ref_gl2_static shared ${SDL2_LIBRARIES} GL m)
endif()

#add_custom_command(TARGET Quake2 POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETPATH)\" ${PROJECT_SOURCE_DIR})
#add_custom_command(TARGET Quake2 POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETDIR)$(TARGETNAME).pdb\" ${PROJECT_SOURCE_DIR})
