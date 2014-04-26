/*
 * Cogl
 *
 * A Low Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2010 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 *
 * Authors:
 *  Neil Roberts   <neil@linux.intel.com>
 *  Johan Bilien   <johan.bilien@nokia.com>
 *  Robert Bragg   <robert@linux.intel.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl-debug.h"
#include "cogl-util.h"
#include "cogl-texture-pixmap-x11.h"
#include "cogl-texture-pixmap-x11-private.h"
#include "cogl-private.h"
#include "cogl-texture-gl-private.h"
#include "cogl-error-private.h"
#include "cogl-gtype-private.h"

static void _cogl_texture_pixmap_x11_right_free (CoglTexturePixmapX11Right *tex_right);

COGL_TEXTURE_DEFINE (TexturePixmapX11Right, texture_pixmap_x11_right);
COGL_GTYPE_DEFINE_CLASS (TexturePixmapX11Right, texture_pixmap_x11_right);

static const CoglTextureVtable cogl_texture_pixmap_x11_right_vtable;

CoglTexture *
_cogl_texture_pixmap_x11_right_new (CoglTexturePixmapX11 *tfp_left)
{
  CoglTexture *texture_left = COGL_TEXTURE (tfp_left);
  CoglTexturePixmapX11Right *tex_right = g_new (CoglTexturePixmapX11Right, 1);
  CoglPixelFormat internal_format;

  tex_right->left = cogl_object_ref (tfp_left);

  internal_format = (tfp_left->depth >= 32
                     ? COGL_PIXEL_FORMAT_RGBA_8888_PRE
                     : COGL_PIXEL_FORMAT_RGB_888);
  _cogl_texture_init (COGL_TEXTURE (tex_right),
		      texture_left->context,
		      texture_left->width,
		      texture_left->height,
                      internal_format,
                      NULL, /* no loader */
                      &cogl_texture_pixmap_x11_right_vtable);

  return (CoglTexture *)_cogl_texture_pixmap_x11_right_object_new (tex_right);
}

static CoglBool
_cogl_texture_pixmap_x11_right_allocate (CoglTexture *tex,
                                         CoglError **error)
{
  return TRUE;
}

static CoglBool
_cogl_texture_pixmap_x11_right_set_region (CoglTexture *tex,
                                           int src_x,
                                           int src_y,
                                           int dst_x,
                                           int dst_y,
                                           int dst_width,
                                           int dst_height,
                                           int level,
                                           CoglBitmap *bmp,
                                           CoglError **error)
{
  /* This doesn't make much sense for texture from pixmap so it's not
     supported */
  _cogl_set_error (error,
                   COGL_SYSTEM_ERROR,
                   COGL_SYSTEM_ERROR_UNSUPPORTED,
                   "Explicitly setting a region of a TFP texture unsupported");
  return FALSE;
}

static CoglBool
_cogl_texture_pixmap_x11_right_get_data (CoglTexture *tex,
                                         CoglPixelFormat format,
                                         int rowstride,
                                         uint8_t *data)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  return cogl_texture_get_data (child_tex, format, rowstride, data);
}

static void
_cogl_texture_pixmap_x11_right_foreach_sub_texture_in_region
                                  (CoglTexture              *tex,
                                   float                     virtual_tx_1,
                                   float                     virtual_ty_1,
                                   float                     virtual_tx_2,
                                   float                     virtual_ty_2,
                                   CoglMetaTextureCallback   callback,
                                   void                     *user_data)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  _cogl_texture_pixmap_x11_do_foreach_sub_texture_in_region
                                                 (tex_right->left,
                                                  child_tex,
                                                  virtual_tx_1, virtual_ty_1,
                                                  virtual_tx_2, virtual_ty_2,
                                                  callback, user_data);
}

static int
_cogl_texture_pixmap_x11_right_get_max_waste (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  return cogl_texture_get_max_waste (child_tex);
}

static CoglBool
_cogl_texture_pixmap_x11_right_is_sliced (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  return cogl_texture_is_sliced (child_tex);
}

static CoglBool
_cogl_texture_pixmap_x11_right_can_hardware_repeat (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  return _cogl_texture_can_hardware_repeat (child_tex);
}

static void
_cogl_texture_pixmap_x11_right_transform_coords_to_gl (CoglTexture *tex,
                                                       float       *s,
                                                       float       *t)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  _cogl_texture_transform_coords_to_gl (child_tex, s, t);
}

static CoglTransformResult
_cogl_texture_pixmap_x11_right_transform_quad_coords_to_gl (CoglTexture *tex,
                                                            float       *coords)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  return _cogl_texture_transform_quad_coords_to_gl (child_tex, coords);
}

static CoglBool
_cogl_texture_pixmap_x11_right_get_gl_texture (CoglTexture *tex,
                                               GLuint      *out_gl_handle,
                                               GLenum      *out_gl_target)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  return cogl_texture_get_gl_texture (child_tex,
                                      out_gl_handle,
                                      out_gl_target);
}

static void
_cogl_texture_pixmap_x11_right_gl_flush_legacy_texobj_filters (CoglTexture *tex,
                                                         GLenum min_filter,
                                                         GLenum mag_filter)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  _cogl_texture_gl_flush_legacy_texobj_filters (child_tex,
                                                min_filter, mag_filter);
}

static void
_cogl_texture_pixmap_x11_right_pre_paint (CoglTexture *tex,
                                    CoglTexturePrePaintFlags flags)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex;

  _cogl_texture_pixmap_x11_update (tex_right->left,
                                   TRUE,
                                   !!(flags & COGL_TEXTURE_NEEDS_MIPMAP));

  child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  _cogl_texture_pre_paint (child_tex, flags);
}

static void
_cogl_texture_pixmap_x11_right_ensure_non_quad_rendering (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  _cogl_texture_ensure_non_quad_rendering (child_tex);
}

static void
_cogl_texture_pixmap_x11_right_gl_flush_legacy_texobj_wrap_modes (CoglTexture *tex,
                                                                  GLenum wrap_mode_s,
                                                                  GLenum wrap_mode_t,
                                                                  GLenum wrap_mode_p)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  _cogl_texture_gl_flush_legacy_texobj_wrap_modes (child_tex,
                                                   wrap_mode_s,
                                                   wrap_mode_t,
                                                   wrap_mode_p);
}

static CoglPixelFormat
_cogl_texture_pixmap_x11_right_get_format (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  return _cogl_texture_get_format (child_tex);
}

static GLenum
_cogl_texture_pixmap_x11_right_get_gl_format (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  return _cogl_texture_gl_get_format (child_tex);
}

static CoglTextureType
_cogl_texture_pixmap_x11_right_get_type (CoglTexture *tex)
{
  CoglTexturePixmapX11Right *tex_right = COGL_TEXTURE_PIXMAP_X11_RIGHT (tex);
  CoglTexture *child_tex;

  child_tex = _cogl_texture_pixmap_x11_get_texture (tex_right->left, TRUE);

  /* Forward on to the child texture */
  return _cogl_texture_get_type (child_tex);
}

static void
_cogl_texture_pixmap_x11_right_free (CoglTexturePixmapX11Right *tex_right)
{
  tex_right->left->right = NULL;
  cogl_object_unref (tex_right->left);

  /* Chain up */
  _cogl_texture_free (COGL_TEXTURE (tex_right));
}

static const CoglTextureVtable
cogl_texture_pixmap_x11_right_vtable =
  {
    FALSE, /* not primitive */
    _cogl_texture_pixmap_x11_right_allocate,
    _cogl_texture_pixmap_x11_right_set_region,
    _cogl_texture_pixmap_x11_right_get_data,
    _cogl_texture_pixmap_x11_right_foreach_sub_texture_in_region,
    _cogl_texture_pixmap_x11_right_get_max_waste,
    _cogl_texture_pixmap_x11_right_is_sliced,
    _cogl_texture_pixmap_x11_right_can_hardware_repeat,
    _cogl_texture_pixmap_x11_right_transform_coords_to_gl,
    _cogl_texture_pixmap_x11_right_transform_quad_coords_to_gl,
    _cogl_texture_pixmap_x11_right_get_gl_texture,
    _cogl_texture_pixmap_x11_right_gl_flush_legacy_texobj_filters,
    _cogl_texture_pixmap_x11_right_pre_paint,
    _cogl_texture_pixmap_x11_right_ensure_non_quad_rendering,
    _cogl_texture_pixmap_x11_right_gl_flush_legacy_texobj_wrap_modes,
    _cogl_texture_pixmap_x11_right_get_format,
    _cogl_texture_pixmap_x11_right_get_gl_format,
    _cogl_texture_pixmap_x11_right_get_type,
    NULL, /* is_foreign */
    NULL /* set_auto_mipmap */
  };
