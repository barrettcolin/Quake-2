set(REF_SOFT_SRCS
  r_aclip.c
  r_alias.c
  r_bsp.c
  r_draw.c
  r_edge.c
  r_image.c
  r_light.c
  r_main.c
  r_misc.c
  r_model.c
  r_part.c
  r_poly.c
  r_polyse.c
  r_rast.c
  r_scan.c
  r_sprite.c
  r_surf.c)

set(REF_SOFT_INCS
  anorms.h)
  
add_library(ref_soft SHARED ${REF_SOFT_SRCS} ${REF_SOFT_INCS} ../win32/rw_imp.c ref_soft.def)
add_dependencies(ref_soft shared)
target_link_libraries(ref_soft shared winmm)

add_custom_command(TARGET ref_soft POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETPATH)\" ${PROJECT_SOURCE_DIR})
add_custom_command(TARGET ref_soft POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different \"$(TARGETDIR)$(TARGETNAME).pdb\" ${PROJECT_SOURCE_DIR})
