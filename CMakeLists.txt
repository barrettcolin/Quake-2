cmake_minimum_required(VERSION 2.6)

project(quake2)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})

add_definitions(-D_CRT_SECURE_NO_WARNINGS -DC_ONLY -DGAME_HARD_LINKED -DREF_HARD_LINKED)

#add_library(shared game/q_shared.c win32/glw_imp.c win32/q_shwin.c win32/qgl_win.c game/q_shared.h win32/glw_win.h)
add_library(shared
  game/q_shared.c
  linux/glob.c
  linux/q_shlinux.c
  game/q_shared.h
  )

add_subdirectory(game)
add_subdirectory(ref_gl)

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
  linux/net_udp.c
  null/cd_null.c
  null/in_null.c
  null/snddma_null.c
  #null/sys_null.c
  null/vid_null.c
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
  #win32/net_wins.c
  #win32/snd_win.c
  #win32/sys_win.c
  #win32/vid_dll.c
  #win32/vid_menu.c
  )

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

#add_executable(Quake2 WIN32 ${Q2_SRCS} ${Q2_INCS} win32/q2.rc win32/q2.ico)
#target_link_libraries(Quake2 shared winmm wsock32)
add_executable(Quake2 ${Q2_SRCS} ${Q2_INCS})
target_link_libraries(Quake2 game_static ref_gl_static shared ${SDL2_LIBRARIES} GL m)

#add_custom_command(TARGET Quake2 POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETPATH)\" ${PROJECT_SOURCE_DIR})
#add_custom_command(TARGET Quake2 POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETDIR)$(TARGETNAME).pdb\" ${PROJECT_SOURCE_DIR})
