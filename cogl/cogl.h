/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2008,2009 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or

 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef __COGL_H__
#define __COGL_H__

#define __COGL_H_INSIDE__

#include <cogl/cogl-defines.h>
#include <cogl/cogl-error.h>

#include <cogl/cogl-object.h>
#include <cogl/cogl-bitmap.h>
#include <cogl/cogl-color.h>
#include <cogl/cogl-matrix.h>
#include <cogl/cogl-matrix-stack.h>
#include <cogl/cogl-offscreen.h>
#include <cogl/cogl-texture.h>
#include <cogl/cogl-types.h>
#include <cogl/cogl-version.h>

#ifdef COGL_HAS_GTYPE_SUPPORT
/* GType integration */
#include <cogl/cogl-enum-types.h>
#endif

#include <cogl/cogl-renderer.h>
#include <cogl/cogl-output.h>
#include <cogl/cogl-display.h>
#include <cogl/cogl-context.h>
#include <cogl/cogl-buffer.h>
#include <cogl/cogl-pixel-buffer.h>
#include <cogl/cogl-vector.h>
#include <cogl/cogl-euler.h>
#include <cogl/cogl-quaternion.h>
#include <cogl/cogl-texture-2d.h>
#include <cogl/cogl-texture-2d-gl.h>
#include <cogl/cogl-texture-rectangle.h>
#include <cogl/cogl-texture-3d.h>
#include <cogl/cogl-texture-2d-sliced.h>
#include <cogl/cogl-sub-texture.h>
#include <cogl/cogl-meta-texture.h>
#include <cogl/cogl-primitive-texture.h>
#include <cogl/cogl-index-buffer.h>
#include <cogl/cogl-attribute-buffer.h>
#include <cogl/cogl-indices.h>
#include <cogl/cogl-attribute.h>
#include <cogl/cogl-primitive.h>
#include <cogl/cogl-depth-state.h>
#include <cogl/cogl-pipeline.h>
#include <cogl/cogl-pipeline-state.h>
#include <cogl/cogl-pipeline-layer-state.h>
#include <cogl/cogl-snippet.h>
#include <cogl/cogl-framebuffer.h>
#include <cogl/cogl-onscreen.h>
#include <cogl/cogl-frame-info.h>
#include <cogl/cogl-poll.h>
#include <cogl/cogl-fence.h>
#if defined (COGL_HAS_EGL_PLATFORM_KMS_SUPPORT)
#include <cogl/cogl-kms-renderer.h>
#include <cogl/cogl-kms-display.h>
#endif
#ifdef COGL_HAS_WIN32_SUPPORT
#include <cogl/cogl-win32-renderer.h>
#endif
#ifdef COGL_HAS_GLIB_SUPPORT
#include <cogl/cogl-glib-source.h>
#endif
/* XXX: This will definitly go away once all the Clutter winsys
 * code has been migrated down into Cogl! */
#include <cogl/cogl-clutter.h>
#ifdef COGL_HAS_SDL_SUPPORT
#include <cogl/cogl-sdl.h>
#endif

#undef __COGL_H_INSIDE__

#endif /* __COGL_H__ */
