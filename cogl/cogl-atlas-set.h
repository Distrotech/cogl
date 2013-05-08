/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2013,2014 Intel Corporation.
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
 */

#if !defined(__COGL_H_INSIDE__) && !defined(COGL_COMPILATION)
#error "Only <cogl/cogl.h> can be included directly."
#endif

#ifndef _COGL_ATLAS_SET_H_
#define _COGL_ATLAS_SET_H_

#include <cogl/cogl-types.h>
#include <cogl/cogl-object.h>
#include <cogl/cogl-atlas.h>

typedef struct _CoglAtlasSet CoglAtlasSet;

CoglAtlasSet *
cogl_atlas_set_new (CoglContext *context);

CoglBool
cogl_is_atlas_set (void *object);

void
cogl_atlas_set_set_components (CoglAtlasSet *set,
                               CoglTextureComponents components);

CoglTextureComponents
cogl_atlas_set_get_components (CoglAtlasSet *set);

void
cogl_atlas_set_set_premultiplied (CoglAtlasSet *set,
                                  CoglBool premultiplied);

CoglBool
cogl_atlas_set_get_premultiplied (CoglAtlasSet *set);

void
cogl_atlas_set_set_clear_enabled (CoglAtlasSet *set,
                                  CoglBool clear_enabled);

CoglBool
cogl_atlas_set_get_clear_enabled (CoglAtlasSet *set,
                                  CoglBool clear_enabled);

void
cogl_atlas_set_set_migration_enabled (CoglAtlasSet *set,
                                      CoglBool migration_enabled);

CoglBool
cogl_atlas_set_get_migration_enabled (CoglAtlasSet *set);


void
cogl_atlas_set_clear (CoglAtlasSet *set);

typedef enum _CoglAtlasSetEvent
{
  COGL_ATLAS_SET_EVENT_ADDED = 1,
  COGL_ATLAS_SET_EVENT_REMOVED = 2
} CoglAtlasSetEvent;

typedef struct _CoglClosure CoglAtlasSetAtlasClosure;

typedef void (*CoglAtlasSetAtlasCallback) (CoglAtlasSet *set,
                                           CoglAtlas *atlas,
                                           CoglAtlasSetEvent event,
                                           void *user_data);

CoglAtlasSetAtlasClosure *
cogl_atlas_set_add_atlas_callback (CoglAtlasSet *set,
                                   CoglAtlasSetAtlasCallback callback,
                                   void *user_data,
                                   CoglUserDataDestroyCallback destroy);

void
cogl_atlas_set_remove_atlas_callback (CoglAtlasSet *set,
                                      CoglAtlasSetAtlasClosure *closure);

CoglAtlas *
cogl_atlas_set_allocate_space (CoglAtlasSet *set,
                               int width,
                               int height,
                               void *allocation_data);

typedef void (* CoglAtlasSetForeachCallback) (CoglAtlas *atlas,
                                              void *user_data);

void
cogl_atlas_set_foreach (CoglAtlasSet *atlas_set,
                        CoglAtlasSetForeachCallback callback,
                        void *user_data);

#endif /* _COGL_ATLAS_SET_H_ */
