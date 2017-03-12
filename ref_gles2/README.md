# ref_gles2

An OpenGL ES 2.0 refresh module for Quake 2. What's called a 'Material' below might might be called a shader elsewhere (the 'Material' term comes from Tom Forsyth's 'Shader Abstraction' article from ShaderX2).

## Init/Shutdown

    R_Init
     GLimp_Init
     R_SetMode
      GLimp_SetMode
       GLimp_Shutdown
       SDL_CreateWindow
       SDL_GL_CreateContext
       ri.Vid_NewWindow
     MaterialSystem_Create
     GL_InitImages
     R_InitParticleTexture
      r_particletexture = GL_LoadPic("***particle***", ..., it_sprite, ...)
      r_notexture = GL_LoadPic("***r_notexture***", ..., it_wall, ...)
     Draw_InitLocal
      draw_chars = GL_FindPic("pics/conchars.pcx", it_pic)
      MaterialSystem_Find draw_material // (unlit +blend_opaque)
      MaterialSystem_Find draw_alpha_material // (unlit +use_alpha_test +blend_opaque)
     MaterialSystem_Find lightmapped_material // (lightmapped +blend_opaque)
     MaterialSystem_Find lightmapped_alpha_material // (lightmapped +use_diffuse_color +blend_alpha)
     MaterialSystem_Find unlit_material // (unlit +use_diffuse_color +blend_opaque)
     MaterialSystem_Find unlit_alpha_material // (unlit +use_diffuse_color +blend_alpha)
     MaterialSystem_Find vertexlit_material // (vertexlit +blend_opaque)
     MaterialSystem_Find vertexlit_alpha_material // (vertexlit +blend_alpha)

    R_Shutdown
     Mod_FreeAll
     GL_ShutdownImages
     MaterialSystem_Destroy
     GLimp_Shutdown
      SDL_GL_DeleteContext
      SDL_DestroyWindow

## Load map

    R_BeginRegistration
     registration_sequence++
     if free_mod_known[0] // current map
      Mod_Free(&mod_known[0]) // free model buffers
     r_worldmodel = Mod_ForName(fullname, ...) // Mod_LoadBrushModel for mod_known[0]

    R_RegisterModel
     Mod_ForName(name, ...) // Mod_LoadAliasModel/Mod_LoadSpriteModel for name
      Mod_LoadAliasModel
       glGenBuffers for alias model data

    R_EndRegistration
     Mod_Free // for all models not with this registration sequence
      glDeleteBuffers // for mod_alias
     GL_FreeUnusedImages // except r_notexture, r_particletexture and it_pic

### Render frame

    R_BeginFrame
     GLimp_BeginFrame
     R_SetGL2D
      Matrix_Orthographic(0, vid.width, vid.height, 0, -99999, 99999, gl_state.clip_from_view)

    R_RenderFrame
     R_RenderView
      R_PushDlights
      R_SetupFrame // Sets current view cluster (leaf->cluster of leaf containing view origin)
      R_SetFrustum
      R_SetupGL
       glViewport
       Matrix_Perspective(camera_separation, fov_y, screenaspect, 4, 4096, gl_state.clip_from_view)
       glCullFace(GL_FRONT)
       Matrix_InverseFromAnglesOrigin(viewangles, vieworg, ref_from_world)
       // gl_state.view_from_world.view_from_world = view_from_ref * ref_from_world
       GL_CULL_FACE
       glEnable(GL_DEPTH_TEST)
      R_MarkVisLeaves // Calculates clusters visible from current view cluster, marks all cluster leaves with current frame count
	  R_MarkViewLeaves // Mark PVS leaves actually in view
      R_DrawWorld
       Material_SetCurrent(g_lightmapped_material)
       Material_SetClipFromView(g_lightmapped_material, gl_state.clip_from_view)
       Material_SetViewFromWorld(g_lightmapped_material, gl_state.view_from_world)
       Material_SetWorldFromModel(g_lightmapped_material, g_identity_matrix)
       RecursiveWorldNode (r_worldmodel->nodes) // (leaf) if leaf has current frame count, mark all surfaces as visible/(node) render node surfaces (i.e. in plane)
       // R_RenderBrushPoly for SURF_DRAWTURB surfaces not rendered by RecursiveWorldNode
       Material_SetCurrent(g_unlit_material)
       Material_SetClipFromView(g_unlit_material, gl_state.clip_from_view)
       Material_SetViewFromWorld(g_unlit_material, gl_state.view_from_world)
       Material_SetWorldFromModel(g_unlit_material, g_identity_matrix)
       Material_SetDiffuseColor(g_unlit_material, 1/intensity, 1/intensity, 1/intensity, 1)
       DrawTextureChains ()
       Material_SetDiffuseColor(g_unlit_material, 1, 1, 1, 1)
       R_DrawSkyBox
       R_DrawTriangleOutlines
      R_DrawEntitiesOnList
       // draw non-transparent entities
       R_DrawAliasModel
        Material_SetCurrent((e->flags & RF_TRANSLUCENT) ? g_vertexlit_alpha_material : g_vertexlit_material)
        Material_SetClipFromView(mat, gl_state.clip_from_view)
        Material_SetViewFromWorld(mat, gl_state.view_from_world)
        Matrix_FromAnglesOrigin(e->angles, e->origin, world_from_entity)
        Material_SetWorldFromModel(mat, world_from_entity)
        GL_MBind(GL_TEXTURE0, skin->texnum)
        GL_DrawAliasFrameLerp (paliashdr, currententity->backlerp)
         glDrawElements
       R_DrawBrushModel
        R_DrawInlineBModel
         foreach surface
          Matrix_FromAnglesOrigin(currententity->angles, currententity->origin, world_from_model)
          Material_SetCurrent(is_translucent ? g_lightmapped_alpha_material : g_lightmapped_material)
          Material_SetWorldFromModel(mat, world_from_model) // proj + view already set when drawing world
          if(is_translucent)
           Material_SetDiffuseColor(mat, 1, 1, 1, alpha)
          GL_RenderLightmappedPoly(psurf)
       R_DrawSpriteModel
       // draw transparent entities
        functions called as for non-transparent entities
      R_RenderDlights
      R_DrawParticles
      R_DrawAlphaSurfaces
       Material_SetCurrent(g_unlit_alpha_material)
       Material_SetClipFromView(g_unlit_alpha_material, gl_state.clip_from_view)
       Material_SetViewFromWorld(g_unlit_alpha_material, gl_state.view_from_world)
       Material_SetWorldFromModel(g_unlit_alpha_material, g_identity_matrix)
      R_Flash
       R_PolyBlend
     R_SetLightLevel // 'BIG HACK!'
     R_SetGL2D
      Matrix_Orthographic(0, vid.width, vid.height, 0, -99999, 99999, gl_state.clip_from_view)

    GLimp_EndFrame
     SDL_GL_SwapWindow

