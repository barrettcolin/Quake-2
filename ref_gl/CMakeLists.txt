set(REF_GL_SRCS
  gl_draw.c
  gl_image.c
  gl_light.c
  gl_mesh.c
  gl_model.c
  gl_rmain.c
  gl_rmisc.c
  gl_rsurf.c
  gl_warp.c)

set(REF_GL_INCS
  anorms.h
  anormtab.h
  gl_local.h
  gl_model.h
  qgl.h
  #qmenu.h
  #ref_gl.h
  warpsin.h)
  
add_library(ref_gl SHARED ${REF_GL_SRCS} ${REF_GL_INCS} ../win32/glw_imp.c ref_gl.def)
add_dependencies(ref_gl shared)
target_link_libraries(ref_gl shared winmm)

add_custom_command(TARGET ref_gl POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETPATH)\" ${PROJECT_SOURCE_DIR})
add_custom_command(TARGET ref_gl POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETDIR)$(TARGETNAME).pdb\" ${PROJECT_SOURCE_DIR})
