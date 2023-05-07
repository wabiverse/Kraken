/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Kraken Loader.
 * USD & co.
 *
 * This file handles updating the `startup.blend`, this is used when reading old files.
 *
 * Unlike regular versioning this makes changes that ensure the startup file
 * has brushes and other presets setup to take advantage of newer features.
 *
 * To update preference defaults see `userdef_default.c`.
 */

#include "MEM_guardedalloc.h"

#include "KLI_listbase.h"
#include "KLI_math.h"
#include "KLI_math_vec_types.hh"
#include "KLI_string.h"
#include "KLI_system.h"
#include "KLI_utildefines.h"

#include "USD_action.h"
#include "USD_area.h"
#include "USD_curveprofile_types.h"
#include "USD_factory.h"
#include "USD_mask.h"
#include "USD_materials.h"
#include "USD_object.h"
#include "USD_object_types.h"
#include "USD_region.h"
#include "USD_scene_types.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userdef_types.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"
#include "USD_view2d.h"
#include "USD_view3d.h"

#include "KKE_appdir.h"
#include "KKE_colortools.h"
#include "KKE_idprop.h"
#include "KKE_lib_id.h"
#include "KKE_main.h"
#include "KKE_material.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "WM_inline_tools.h"

#include "KLO_readfile.h"

// #include "versioning_common.h"

/* Make preferences read-only, use versioning_userdef.c. */
#define U (*((const UserDef *)&U))

static bool klo_is_builtin_template(const char *app_template)
{
  /* For all builtin templates shipped with Kraken. */
  return (
    !app_template ||
    STR_ELEM(app_template, N_("2D_Animation"), N_("Sculpting"), N_("VFX"), N_("Video_Editing")));
}

static void klo_update_defaults_screen(kScreen *screen,
                                       const char *app_template,
                                       const char *workspace_name)
{
  /* For all app templates. */
  LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
  {
    LISTBASE_FOREACH(ARegion *, region, &area->regions)
    {
      /* Some toolbars have been saved as initialized,
       * we don't want them to have odd zoom-level or scrolling set, see: T47047 */
      if (ELEM(region->regiontype, RGN_TYPE_UI, RGN_TYPE_TOOLS, RGN_TYPE_TOOL_PROPS)) {
        region->v2d.flag &= ~V2D_IS_INIT;
      }
    }

    /* Set default folder. */

    LISTBASE_FOREACH(SpaceLink *, sl, &area->spacedata)
    {
      if (sl->spacetype == SPACE_FILE) {
        SpaceFile *sfile = (SpaceFile *)sl;
        if (sfile->params) {
          const char *dir_default = KKE_appdir_folder_default();
          if (dir_default) {
            STRNCPY(sfile->params->dir, dir_default);
            sfile->params->file[0] = '\0';
          }
        }
      }
    }
  }

  /* For builtin templates only. */
  if (!klo_is_builtin_template(app_template)) {
    return;
  }

  LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
  {
    LISTBASE_FOREACH(ARegion *, region, &area->regions)
    {
      /* Remove all stored panels, we want to use defaults
       * (order, open/closed) as defined by UI code here! */
      KKE_area_region_panels_free(&region->panels);
      KLI_freelistN(&region->panels_category_active);

      /* Reset size so it uses consistent defaults from the region types. */
      FormFactory(region->size, GfVec2f(0, 0));
    }

    TfToken st = FormFactory(area->spacetype);

    if (WM_spacetype_enum_from_token(st) == SPACE_IMAGE) {
      if (STREQ(workspace_name, "UV Editing")) {
        SpaceImage *sima = static_cast<SpaceImage *>(area->spacedata.first);
        if (sima->mode == SI_MODE_VIEW) {
          sima->mode = SI_MODE_UV;
        }
      }
    } else if (WM_spacetype_enum_from_token(st) == SPACE_ACTION) {
      /* Show markers region, hide channels and collapse summary in timelines. */
      SpaceAction *saction = static_cast<SpaceAction *>(area->spacedata.first);
      saction->flag |= SACTION_SHOW_MARKERS;
      if (saction->mode == SACTCONT_TIMELINE) {
        saction->ads.flag |= ADS_FLAG_SUMMARY_COLLAPSED;

        LISTBASE_FOREACH(ARegion *, region, &area->regions)
        {
          if (region->regiontype == RGN_TYPE_CHANNELS) {
            region->flag |= RGN_FLAG_HIDDEN;
          }
        }
      } else {
        /* Open properties panel by default. */
        LISTBASE_FOREACH(ARegion *, region, &area->regions)
        {
          if (region->regiontype == RGN_TYPE_UI) {
            region->flag &= ~RGN_FLAG_HIDDEN;
          }
        }
      }
    } else if (WM_spacetype_enum_from_token(st) == SPACE_GRAPH) {
      SpaceGraph *sipo = static_cast<SpaceGraph *>(area->spacedata.first);
      sipo->flag |= SIPO_SHOW_MARKERS;
    } else if (WM_spacetype_enum_from_token(st) == SPACE_NLA) {
      SpaceNla *snla = static_cast<SpaceNla *>(area->spacedata.first);
      snla->flag |= SNLA_SHOW_MARKERS;
    } else if (WM_spacetype_enum_from_token(st) == SPACE_SEQ) {
      SpaceSeq *seq = static_cast<SpaceSeq *>(area->spacedata.first);
      seq->flag |= SEQ_SHOW_MARKERS | SEQ_ZOOM_TO_FIT | SEQ_USE_PROXIES | SEQ_SHOW_OVERLAY;
      seq->render_size = SEQ_RENDER_SIZE_PROXY_100;
      seq->timeline_overlay.flag |= SEQ_TIMELINE_SHOW_STRIP_SOURCE | SEQ_TIMELINE_SHOW_STRIP_NAME |
                                    SEQ_TIMELINE_SHOW_STRIP_DURATION | SEQ_TIMELINE_SHOW_GRID |
                                    SEQ_TIMELINE_SHOW_STRIP_COLOR_TAG;
      seq->preview_overlay.flag |= SEQ_PREVIEW_SHOW_OUTLINE_SELECTED;
    } else if (WM_spacetype_enum_from_token(st) == SPACE_TEXT) {
      /* Show syntax and line numbers in Script workspace text editor. */
      SpaceText *stext = static_cast<SpaceText *>(area->spacedata.first);
      stext->showsyntax = true;
      stext->showlinenrs = true;
    } else if (WM_spacetype_enum_from_token(st) == SPACE_VIEW3D) {
      View3D *v3d = static_cast<View3D *>(area->spacedata.first);
      /* Screen space cavity by default for faster performance. */
      v3d->shading.cavity_type = V3D_SHADING_CAVITY_CURVATURE;
      v3d->shading.flag |= V3D_SHADING_SPECULAR_HIGHLIGHT;
      v3d->overlay.texture_paint_mode_opacity = 1.0f;
      v3d->overlay.weight_paint_mode_opacity = 1.0f;
      v3d->overlay.vertex_paint_mode_opacity = 1.0f;
      /* Use dimmed selected edges. */
      v3d->overlay.edit_flag &= ~V3D_OVERLAY_EDIT_EDGES;
      /* grease pencil settings */
      v3d->vertex_opacity = 1.0f;
      v3d->gp_flag |= V3D_GP_SHOW_EDIT_LINES;
      /* Remove dither pattern in wireframe mode. */
      v3d->shading.xray_alpha_wire = 0.0f;
      v3d->clip_start = 0.01f;
      /* Skip startups that use the viewport color by default. */
      if (v3d->shading.background_type != V3D_SHADING_BACKGROUND_VIEWPORT) {
        copy_v3_fl(v3d->shading.background_color, 0.05f);
      }
      /* Disable Curve Normals. */
      v3d->overlay.edit_flag &= ~V3D_OVERLAY_EDIT_CU_NORMALS;
      v3d->overlay.normals_constant_screen_size = 7.0f;
    } else if (WM_spacetype_enum_from_token(st) == SPACE_CLIP) {
      SpaceClip *sclip = static_cast<SpaceClip *>(area->spacedata.first);
      sclip->around = V3D_AROUND_CENTER_MEDIAN;
      sclip->mask_info.blend_factor = 0.7f;
      sclip->mask_info.draw_flag = MASK_DRAWFLAG_SPLINE;
    }
  }

  /* Show tool-header by default (for most cases at least, hide for others). */
  const bool hide_image_tool_header = STREQ(workspace_name, "Rendering");
  LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
  {
    LISTBASE_FOREACH(SpaceLink *, sl, &area->spacedata)
    {
      ListBase *regionbase = (sl == static_cast<SpaceLink *>(area->spacedata.first)) ?
                               &area->regions :
                               &sl->regionbase;

      LISTBASE_FOREACH(ARegion *, region, regionbase)
      {
        if (region->regiontype == RGN_TYPE_TOOL_HEADER) {
          if (((sl->spacetype == SPACE_IMAGE) && hide_image_tool_header) ||
              sl->spacetype == SPACE_SEQ) {
            region->flag |= RGN_FLAG_HIDDEN;
          } else {
            region->flag &= ~(RGN_FLAG_HIDDEN | RGN_FLAG_HIDDEN_BY_USER);
          }
        }
      }
    }
  }

  /* 2D animation template. */
  if (app_template && STREQ(app_template, "2D_Animation")) {
    LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
    {
      TfToken st;
      area->spacetype.Get(&st);

      if (WM_spacetype_enum_from_token(st) == SPACE_ACTION) {
        SpaceAction *saction = static_cast<SpaceAction *>(area->spacedata.first);
        /* Enable Sliders. */
        saction->flag |= SACTION_SLIDERS;
      } else if (WM_spacetype_enum_from_token(st) == SPACE_VIEW3D) {
        View3D *v3d = static_cast<View3D *>(area->spacedata.first);
        /* Set Material Color by default. */
        v3d->shading.color_type = V3D_SHADING_MATERIAL_COLOR;
        /* Enable Annotations. */
        v3d->flag2 |= V3D_SHOW_ANNOTATION;
      }
    }
  }
}

void KLO_update_defaults_workspace(WorkSpace *workspace, const char *app_template)
{
  LISTBASE_FOREACH(WorkSpaceLayout *, layout, &workspace->layouts)
  {
    if (layout->screen) {
      klo_update_defaults_screen(layout->screen, app_template, workspace->id.name + 2);
    }
  }

  if (klo_is_builtin_template(app_template)) {
    /* Clear all tools to use default options instead, ignore the tool saved in the file. */
    // while (!KLI_listbase_is_empty(&workspace->tools)) {
    //   KKE_workspace_tool_remove(workspace, static_cast<kToolRef *>(workspace->tools.first));
    // }

    /* For 2D animation template. */
    // if (STREQ(workspace->id.name + 2, "Drawing")) {
    //   workspace->object_mode = OB_MODE_PAINT_GPENCIL;
    // }

    /* For Sculpting template. */
    if (STREQ(workspace->id.name + 2, "Sculpting")) {
      LISTBASE_FOREACH(WorkSpaceLayout *, layout, &workspace->layouts)
      {
        kScreen *screen = layout->screen;
        if (screen) {
          LISTBASE_FOREACH(ScrArea *, area, &screen->areas)
          {
            LISTBASE_FOREACH(ARegion *, region, &area->regions)
            {
              TfToken st;
              region->spacetype.Get(&st);

              if (WM_spacetype_enum_from_token(st) == SPACE_VIEW3D) {
                View3D *v3d = static_cast<View3D *>(area->spacedata.first);
                v3d->shading.flag &= ~V3D_SHADING_CAVITY;
                copy_v3_fl(v3d->shading.single_color, 1.0f);
                STRNCPY(v3d->shading.matcap, "basic_1");
              }
            }
          }
        }
      }
    }
  }
}

static ID *do_versions_rename_id(Main *kmain,
                                 const short id_type,
                                 const char *name_src,
                                 const char *name_dst)
{
  /* We can ignore libraries */
  ListBase *lb = which_libbase(kmain, id_type);
  ID *id = nullptr;
  LISTBASE_FOREACH(ID *, idtest, lb)
  {
    if (!ID_IS_LINKED(idtest)) {
      if (STREQ(idtest->name + 2, name_src)) {
        id = idtest;
      }
      if (STREQ(idtest->name + 2, name_dst)) {
        return nullptr;
      }
    }
  }
  if (id != nullptr) {
    KKE_main_namemap_remove_name(kmain, id, id->name + 2);
    KLI_strncpy(id->name + 2, name_dst, sizeof(id->name) - 2);
    /* We know it's unique, this just sorts. */
    KKE_libblock_ensure_unique_name(kmain, id->name);
  }
  return id;
}

void KLO_update_defaults_startup_usd(Main *kmain, const char *app_template)
{
  /* For all app templates. */
  LISTBASE_FOREACH(WorkSpace *, workspace, &kmain->workspaces)
  {
    KLO_update_defaults_workspace(workspace, app_template);
  }

  /* New grease pencil brushes and vertex paint setup. */
  {
    /* Update Grease Pencil brushes. */
    Brush *brush;

    /* Pencil brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw Pencil", "Pencil");

    /* Pen brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw Pen", "Pen");

    /* Pen Soft brush. */
    // brush = reinterpret_cast<Brush *>(
    //   do_versions_rename_id(kmain, ID_BR, "Draw Soft", "Pencil Soft"));
    // if (brush) {
    //   brush->gpencil_settings->icon_id = GP_BRUSH_ICON_PEN;
    // }

    /* Ink Pen brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw Ink", "Ink Pen");

    /* Ink Pen Rough brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw Noise", "Ink Pen Rough");

    /* Marker Bold brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw Marker", "Marker Bold");

    /* Marker Chisel brush. */
    do_versions_rename_id(kmain, ID_BR, "Draw KLOck", "Marker Chisel");

    /* Remove useless Fill Area.001 brush. */
    // brush = static_cast<Brush *>(
    //   KLI_findstring(&kmain->brushes, "Fill Area.001", offsetof(ID, name) + 2));
    // if (brush) {
    //   KKE_id_delete(kmain, brush);
    // }

    /* Rename and fix materials and enable default object lights on. */
    if (app_template && STREQ(app_template, "2D_Animation")) {
      Material *ma = NULL;
      do_versions_rename_id(kmain, ID_MA, "Black", "Solid Stroke");
      do_versions_rename_id(kmain, ID_MA, "Red", "Squares Stroke");
      do_versions_rename_id(kmain, ID_MA, "Grey", "Solid Fill");
      do_versions_rename_id(kmain, ID_MA, "Black Dots", "Dots Stroke");

      /* Dots Stroke. */
      ma = static_cast<Material *>(
        KLI_findstring(&kmain->materials, "Dots Stroke", offsetof(ID, name) + 2));
      if (ma == NULL) {
        ma = KKE_gpencil_material_add(kmain, "Dots Stroke");
      }
      ma->gp_style->mode = GP_MATERIAL_MODE_DOT;

      /* Squares Stroke. */
      ma = static_cast<Material *>(
        KLI_findstring(&kmain->materials, "Squares Stroke", offsetof(ID, name) + 2));
      if (ma == NULL) {
        ma = KKE_gpencil_material_add(kmain, "Squares Stroke");
      }
      ma->gp_style->mode = GP_MATERIAL_MODE_SQUARE;

      /* Change Solid Stroke settings. */
      ma = static_cast<Material *>(
        KLI_findstring(&kmain->materials, "Solid Stroke", offsetof(ID, name) + 2));
      if (ma != NULL) {
        ma->gp_style->mix_rgba[3] = 1.0f;
        ma->gp_style->texture_offset[0] = -0.5f;
        ma->gp_style->mix_factor = 0.5f;
      }

      /* Change Solid Fill settings. */
      ma = static_cast<Material *>(
        KLI_findstring(&kmain->materials, "Solid Fill", offsetof(ID, name) + 2));
      if (ma != NULL) {
        ma->gp_style->flag &= ~GP_MATERIAL_STROKE_SHOW;
        ma->gp_style->mix_rgba[3] = 1.0f;
        ma->gp_style->texture_offset[0] = -0.5f;
        ma->gp_style->mix_factor = 0.5f;
      }

      Object *ob = static_cast<Object *>(
        KLI_findstring(&kmain->objects, "Stroke", offsetof(ID, name) + 2));
      if (ob && ob->type == OB_GPENCIL) {
        ob->dtx |= OB_USE_GPENCIL_LIGHTS;
      }
    }

    /* Reset all grease pencil brushes. */
    Scene *scene = static_cast<Scene *>(kmain->scenes.first);
    // KKE_brush_gpencil_paint_presets(kmain, scene->toolsettings, true);
    // KKE_brush_gpencil_sculpt_presets(kmain, scene->toolsettings, true);
    // KKE_brush_gpencil_vertex_presets(kmain, scene->toolsettings, true);
    // KKE_brush_gpencil_weight_presets(kmain, scene->toolsettings, true);

    /* Ensure new Paint modes. */
    // KKE_paint_ensure_from_paintmode(scene, PAINT_MODE_VERTEX_GPENCIL);
    // KKE_paint_ensure_from_paintmode(scene, PAINT_MODE_SCULPT_GPENCIL);
    // KKE_paint_ensure_from_paintmode(scene, PAINT_MODE_WEIGHT_GPENCIL);

    /* Enable cursor. */
    GpPaint *gp_paint = scene->toolsettings->gp_paint;
    gp_paint->paint.flags |= PAINT_SHOW_BRUSH;

    /* Ensure Palette by default. */
    // KKE_gpencil_palette_ensure(kmain, scene);
  }

  /* For builtin templates only. */
  if (!klo_is_builtin_template(app_template)) {
    return;
  }

  /* Workspaces. */
  LISTBASE_FOREACH(wmWindowManager *, wm, &kmain->wm)
  {
    LISTBASE_FOREACH(wmWindow *, win, &wm->windows)
    {
      LISTBASE_FOREACH(WorkSpace *, workspace, &kmain->workspaces)
      {
        WorkSpaceLayout *layout = KKE_workspace_active_layout_for_workspace_get(win->workspace_hook,
                                                                                workspace);
        /* Name all screens by their workspaces (avoids 'Default.###' names). */
        /* Default only has one window. */
        if (layout->screen) {
          kScreen *screen = layout->screen;
          if (!STREQ(screen->path.GetName().data() + 2, workspace->id.name + 2)) {
            KKE_main_namemap_remove_name(kmain, (ID *)screen->path.GetName().data(), screen->path.GetName().data() + 2);
            screen->path.ReplaceName(TfToken(workspace->id.name + 2));

            KKE_libblock_ensure_unique_name(kmain, screen->path.GetName().data());
          }
        }

        /* For some reason we have unused screens, needed until re-saving.
         * Clear unused layouts because they're visible in the outliner & Python API. */
        LISTBASE_FOREACH_MUTABLE(WorkSpaceLayout *, layout_iter, &workspace->layouts)
        {
          if (layout != layout_iter) {
            KKE_workspace_layout_remove(kmain, workspace, layout_iter);
          }
        }
      }
    }
  }

  /* Scenes */
  LISTBASE_FOREACH(Scene *, scene, &kmain->scenes)
  {
    //KLO_update_defaults_scene(kmain, scene);

    if (app_template && STREQ(app_template, "Video_Editing")) {
      /* Filmic is too slow, use standard until it is optimized. */
      STRNCPY(scene->view_settings.view_transform, "Standard");
      STRNCPY(scene->view_settings.look, "None");
    } else {
      /* AV Sync break physics sim caching, disable until that is fixed. */
      scene->audio.flag &= ~AUDIO_SYNC;
      scene->flag &= ~SCE_FRAME_DROP;
    }

    /* Change default selection mode for Grease Pencil. */
    if (app_template && STREQ(app_template, "2D_Animation")) {
      ToolSettings *ts = scene->toolsettings;
      ts->gpencil_selectmode_edit = GP_SELECTMODE_STROKE;
    }
  }

  /* Objects */
  do_versions_rename_id(kmain, ID_OB, "Lamp", "Light");
  do_versions_rename_id(kmain, ID_LA, "Lamp", "Light");

  // if (app_template && STREQ(app_template, "2D_Animation")) {
  //   LISTBASE_FOREACH(Object *, object, &kmain->objects)
  //   {
  //     if (object->type == OB_GPENCIL) {
  //       /* Set grease pencil object in drawing mode */
  //       bGPdata *gpd = (bGPdata *)object->data;
  //       object->mode = OB_MODE_PAINT_GPENCIL;
  //       gpd->flag |= GP_DATA_STROKE_PAINTMODE;
  //       break;
  //     }
  //   }
  // }

  // LISTBASE_FOREACH(Mesh *, mesh, &kmain->meshes)
  // {
  //   /* Match default for new meshes. */
  //   mesh->smoothresh = DEG2RADF(30);
  //   /* Match voxel remesher options for all existing meshes in templates. */
  //   mesh->flag |= ME_REMESH_REPROJECT_VOLUME | ME_REMESH_REPROJECT_PAINT_MASK |
  //                 ME_REMESH_REPROJECT_SCULPT_FACE_SETS | ME_REMESH_REPROJECT_VERTEX_COLORS;

  //   /* For Sculpting template. */
  //   if (app_template && STREQ(app_template, "Sculpting")) {
  //     mesh->remesh_voxel_size = 0.035f;
  //     KKE_mesh_smooth_flag_set(mesh, false);
  //   } else {
  //     /* Remove sculpt-mask data in default mesh objects for all non-sculpt templates. */
  //     CustomData_free_layers(&mesh->vdata, CD_PAINT_MASK, mesh->totvert);
  //     CustomData_free_layers(&mesh->ldata, CD_GRID_PAINT_MASK, mesh->totloop);
  //   }
  //   mesh->attributes_for_write().remove(".sculpt_face_set");
  // }

  // LISTBASE_FOREACH(Camera *, camera, &kmain->cameras)
  // {
  //   /* Initialize to a useful value. */
  //   camera->dof.focus_distance = 10.0f;
  //   camera->dof.aperture_fstop = 2.8f;
  // }

  // LISTBASE_FOREACH(Light *, light, &kmain->lights)
  // {
  //   /* Fix lights defaults. */
  //   light->clipsta = 0.05f;
  //   light->att_dist = 40.0f;
  // }

  /* Materials */
  LISTBASE_FOREACH(Material *, ma, &kmain->materials)
  {
    /* Update default material to be a bit more rough. */
    ma->roughness = 0.5f;

    // if (ma->nodetree) {
    //   LISTBASE_FOREACH(kNode *, node, &ma->nodetree->nodes)
    //   {
    //     if (node->type == SH_NODE_BSDF_PRINCIPLED) {
    //       bNodeSocket *roughness_socket = nodeFindSocket(node, SOCK_IN, "Roughness");
    //       bNodeSocketValueFloat *roughness_data = static_cast<bNodeSocketValueFloat *>(
    //         roughness_socket->default_value);
    //       roughness_data->value = 0.5f;
    //       node->custom2 = SHD_SUBSURFACE_RANDOM_WALK;
    //       KKE_ntree_update_tag_node_property(ma->nodetree, node);
    //     } else if (node->type == SH_NODE_SUBSURFACE_SCATTERING) {
    //       node->custom1 = SHD_SUBSURFACE_RANDOM_WALK;
    //       KKE_ntree_update_tag_node_property(ma->nodetree, node);
    //     }
    //   }
    // }
  }

  /* Brushes */
  {
    /* Enable for UV sculpt (other brush types will be created as needed),
     * without this the grab brush will be active but not selectable from the list. */
    const char *brush_name = "Grab";
    Brush *brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (brush) {
    //   brush->ob_mode |= OB_MODE_EDIT;
    // }
  }

  // LISTBASE_FOREACH(Brush *, brush, &kmain->brushes)
  // {
  //   brush->blur_kernel_radius = 2;

  //   /* Use full strength for all non-sculpt brushes,
  //    * when painting we want to use full color/weight always.
  //    *
  //    * Note that sculpt is an exception,
  //    * its values are overwritten by #KKE_brush_sculpt_reset below. */
  //   brush->alpha = 1.0;

  //   /* Enable antialiasing by default */
  //   brush->sampling_flag |= BRUSH_PAINT_ANTIALIASING;
  // }

  {
    /* Change the spacing of the Smear brush to 3.0% */
    const char *brush_name;
    Brush *brush;

    brush_name = "Smear";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (brush) {
    //   brush->spacing = 3.0;
    // }

    brush_name = "Draw Sharp";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_DRAW_SHARP;
    // }

    brush_name = "Elastic Deform";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_ELASTIC_DEFORM;
    // }

    brush_name = "Pose";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_POSE;
    // }

    brush_name = "Multi-plane Scrape";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_MULTIPLANE_SCRAPE;
    // }

    brush_name = "Clay Thumb";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_CLAY_THUMB;
    // }

    brush_name = "Cloth";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_CLOTH;
    // }

    brush_name = "Slide Relax";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_SLIDE_RELAX;
    // }

    brush_name = "Paint";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_PAINT;
    // }

    brush_name = "Smear";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_SMEAR;
    // }

    brush_name = "Boundary";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_BOUNDARY;
    // }

    brush_name = "Simplify";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_SIMPLIFY;
    // }

    brush_name = "Draw Face Sets";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_DRAW_FACE_SETS;
    // }

    brush_name = "Multires Displacement Eraser";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_DISPLACEMENT_ERASER;
    // }

    brush_name = "Multires Displacement Smear";
    brush = static_cast<Brush *>(
      KLI_findstring(&kmain->brushes, brush_name, offsetof(ID, name) + 2));
    // if (!brush) {
    //   brush = KKE_brush_add(kmain, brush_name, OB_MODE_SCULPT);
    //   id_us_min(&brush->id);
    //   brush->sculpt_tool = SCULPT_TOOL_DISPLACEMENT_SMEAR;
    // }

    /* Use the same tool icon color in the brush cursor */
    // LISTBASE_FOREACH(Brush *, brush, &kmain->brushes)
    // {
    //   if (brush->ob_mode & OB_MODE_SCULPT) {
    //     KLI_assert(brush->sculpt_tool != 0);
    //     KKE_brush_sculpt_reset(brush);
    //   }
    // }
  }
}