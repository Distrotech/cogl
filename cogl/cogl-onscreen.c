/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2011, 2013 Intel Corporation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl-util.h"
#include "cogl-onscreen-private.h"
#include "cogl-frame-info-private.h"
#include "cogl-framebuffer-private.h"
#include "cogl-onscreen-template-private.h"
#include "cogl-context-private.h"
#include "cogl-object-private.h"
#include "cogl-closure-list-private.h"
#include "cogl-poll-private.h"

static void _cogl_onscreen_free (CoglOnscreen *onscreen);

COGL_OBJECT_DEFINE_WITH_CODE (Onscreen, onscreen,
                              _cogl_onscreen_class.virt_unref =
                              _cogl_framebuffer_unref);

static void
_cogl_onscreen_init_from_template (CoglOnscreen *onscreen,
                                   CoglOnscreenTemplate *onscreen_template)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);

  COGL_LIST_INIT (&onscreen->frame_closures);
  COGL_LIST_INIT (&onscreen->resize_closures);
  COGL_LIST_INIT (&onscreen->dirty_closures);

  framebuffer->config = onscreen_template->config;
}

CoglOnscreen *
cogl_onscreen_new (CoglContext *ctx, int width, int height)
{
  CoglOnscreen *onscreen;

  /* FIXME: We are assuming onscreen buffers will always be
     premultiplied so we'll set the premult flag on the bitmap
     format. This will usually be correct because the result of the
     default blending operations for Cogl ends up with premultiplied
     data in the framebuffer. However it is possible for the
     framebuffer to be in whatever format depending on what
     CoglPipeline is used to render to it. Eventually we may want to
     add a way for an application to inform Cogl that the framebuffer
     is not premultiplied in case it is being used for some special
     purpose. */

  onscreen = g_new0 (CoglOnscreen, 1);
  _cogl_framebuffer_init (COGL_FRAMEBUFFER (onscreen),
                          ctx,
                          COGL_FRAMEBUFFER_TYPE_ONSCREEN,
                          COGL_PIXEL_FORMAT_RGBA_8888_PRE,
                          width, /* width */
                          height); /* height */

  _cogl_onscreen_init_from_template (onscreen, ctx->display->onscreen_template);

  return _cogl_onscreen_object_new (onscreen);
}

static void
_cogl_onscreen_free (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys = _cogl_framebuffer_get_winsys (framebuffer);
  CoglFrameInfo *frame_info;

  _cogl_closure_list_disconnect_all (&onscreen->resize_closures);
  _cogl_closure_list_disconnect_all (&onscreen->frame_closures);
  _cogl_closure_list_disconnect_all (&onscreen->dirty_closures);

  while ((frame_info = g_queue_pop_tail (&onscreen->pending_frame_infos)))
    cogl_object_unref (frame_info);
  g_queue_clear (&onscreen->pending_frame_infos);

  winsys->onscreen_deinit (onscreen);
  _COGL_RETURN_IF_FAIL (onscreen->winsys == NULL);

  /* Chain up to parent */
  _cogl_framebuffer_free (framebuffer);

  g_free (onscreen);
}

static void
notify_event (CoglOnscreen *onscreen,
              CoglFrameEvent event,
              CoglFrameInfo *info)
{
  _cogl_closure_list_invoke (&onscreen->frame_closures,
                             CoglFrameCallback,
                             onscreen, event, info);
}

static void
_cogl_dispatch_onscreen_cb (CoglContext *context)
{
  CoglOnscreenEvent *event, *tmp;
  CoglOnscreenEventList queue;

  /* Dispatching the event callback may cause another frame to be
   * drawn which in may cause another event to be queued immediately.
   * To make sure this loop will only dispatch one set of events we'll
   * steal the queue and iterate that separately */
  COGL_TAILQ_INIT (&queue);
  COGL_TAILQ_CONCAT (&queue, &context->onscreen_events_queue, list_node);
  COGL_TAILQ_INIT (&context->onscreen_events_queue);

  _cogl_closure_disconnect (context->onscreen_dispatch_idle);
  context->onscreen_dispatch_idle = NULL;

  COGL_TAILQ_FOREACH_SAFE (event,
                           &queue,
                           list_node,
                           tmp)
    {
      CoglOnscreen *onscreen = event->onscreen;
      CoglFrameInfo *info = event->info;

      notify_event (onscreen, event->type, info);

      cogl_object_unref (onscreen);
      cogl_object_unref (info);

      g_slice_free (CoglOnscreenEvent, event);
    }

  while (!COGL_TAILQ_EMPTY (&context->onscreen_dirty_queue))
    {
      CoglOnscreenQueuedDirty *qe =
        COGL_TAILQ_FIRST (&context->onscreen_dirty_queue);

      COGL_TAILQ_REMOVE (&context->onscreen_dirty_queue, qe, list_node);

      _cogl_closure_list_invoke (&qe->onscreen->dirty_closures,
                                 CoglOnscreenDirtyCallback,
                                 qe->onscreen,
                                 &qe->info);

      cogl_object_unref (qe->onscreen);

      g_slice_free (CoglOnscreenQueuedDirty, qe);
    }
}

static void
_cogl_onscreen_queue_dispatch_idle (CoglOnscreen *onscreen)
{
  CoglContext *ctx = COGL_FRAMEBUFFER (onscreen)->context;

  if (!ctx->onscreen_dispatch_idle)
    {
      ctx->onscreen_dispatch_idle =
        _cogl_poll_renderer_add_idle (ctx->display->renderer,
                                      (CoglIdleCallback)
                                      _cogl_dispatch_onscreen_cb,
                                      ctx,
                                      NULL);
    }
}

void
_cogl_onscreen_queue_dirty (CoglOnscreen *onscreen,
                            const CoglOnscreenDirtyInfo *info)
{
  CoglContext *ctx = COGL_FRAMEBUFFER (onscreen)->context;
  CoglOnscreenQueuedDirty *qe = g_slice_new (CoglOnscreenQueuedDirty);

  qe->onscreen = cogl_object_ref (onscreen);
  qe->info = *info;
  COGL_TAILQ_INSERT_TAIL (&ctx->onscreen_dirty_queue, qe, list_node);

  _cogl_onscreen_queue_dispatch_idle (onscreen);
}

void
_cogl_onscreen_queue_full_dirty (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  CoglOnscreenDirtyInfo info;

  info.x = 0;
  info.y = 0;
  info.width = framebuffer->width;
  info.height = framebuffer->height;

  _cogl_onscreen_queue_dirty (onscreen, &info);
}

static void
_cogl_onscreen_queue_event (CoglOnscreen *onscreen,
                            CoglFrameEvent type,
                            CoglFrameInfo *info)
{
  CoglContext *ctx = COGL_FRAMEBUFFER (onscreen)->context;

  CoglOnscreenEvent *event = g_slice_new (CoglOnscreenEvent);

  event->onscreen = cogl_object_ref (onscreen);
  event->info = cogl_object_ref (info);
  event->type = type;

  COGL_TAILQ_INSERT_TAIL (&ctx->onscreen_events_queue, event, list_node);

  _cogl_onscreen_queue_dispatch_idle (onscreen);
}

void
cogl_onscreen_swap_buffers_with_damage (CoglOnscreen *onscreen,
                                        const int *rectangles,
                                        int n_rectangles)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys;
  CoglFrameInfo *info;

  _COGL_RETURN_IF_FAIL  (framebuffer->type == COGL_FRAMEBUFFER_TYPE_ONSCREEN);

  info = _cogl_frame_info_new ();
  info->frame_counter = onscreen->frame_counter;
  g_queue_push_tail (&onscreen->pending_frame_infos, info);

  _cogl_framebuffer_flush_journal (framebuffer);

  winsys = _cogl_framebuffer_get_winsys (framebuffer);
  winsys->onscreen_swap_buffers_with_damage (onscreen,
                                             rectangles, n_rectangles);
  cogl_framebuffer_discard_buffers (framebuffer,
                                    COGL_BUFFER_BIT_COLOR |
                                    COGL_BUFFER_BIT_DEPTH |
                                    COGL_BUFFER_BIT_STENCIL);

  if (!_cogl_winsys_has_feature (COGL_WINSYS_FEATURE_SYNC_AND_COMPLETE_EVENT))
    {
      CoglFrameInfo *info;

      g_warn_if_fail (onscreen->pending_frame_infos.length == 1);

      info = g_queue_pop_tail (&onscreen->pending_frame_infos);

      _cogl_onscreen_queue_event (onscreen, COGL_FRAME_EVENT_SYNC, info);
      _cogl_onscreen_queue_event (onscreen, COGL_FRAME_EVENT_COMPLETE, info);

      cogl_object_unref (info);
    }

  onscreen->frame_counter++;
  framebuffer->mid_scene = FALSE;
}

void
cogl_onscreen_swap_buffers (CoglOnscreen *onscreen)
{
  cogl_onscreen_swap_buffers_with_damage (onscreen, NULL, 0);
}

void
cogl_onscreen_swap_region (CoglOnscreen *onscreen,
                           const int *rectangles,
                           int n_rectangles)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys;
  CoglFrameInfo *info;

  _COGL_RETURN_IF_FAIL  (framebuffer->type == COGL_FRAMEBUFFER_TYPE_ONSCREEN);

  info = _cogl_frame_info_new ();
  info->frame_counter = onscreen->frame_counter;
  g_queue_push_tail (&onscreen->pending_frame_infos, info);

  _cogl_framebuffer_flush_journal (framebuffer);

  winsys = _cogl_framebuffer_get_winsys (framebuffer);

  /* This should only be called if the winsys advertises
     COGL_WINSYS_FEATURE_SWAP_REGION */
  _COGL_RETURN_IF_FAIL (winsys->onscreen_swap_region != NULL);

  winsys->onscreen_swap_region (COGL_ONSCREEN (framebuffer),
                                rectangles,
                                n_rectangles);

  cogl_framebuffer_discard_buffers (framebuffer,
                                    COGL_BUFFER_BIT_COLOR |
                                    COGL_BUFFER_BIT_DEPTH |
                                    COGL_BUFFER_BIT_STENCIL);

  if (!_cogl_winsys_has_feature (COGL_WINSYS_FEATURE_SYNC_AND_COMPLETE_EVENT))
    {
      CoglFrameInfo *info;

      g_warn_if_fail (onscreen->pending_frame_infos.length == 1);

      info = g_queue_pop_tail (&onscreen->pending_frame_infos);

      _cogl_onscreen_queue_event (onscreen, COGL_FRAME_EVENT_SYNC, info);
      _cogl_onscreen_queue_event (onscreen, COGL_FRAME_EVENT_COMPLETE, info);

      cogl_object_unref (info);
    }

  onscreen->frame_counter++;
  framebuffer->mid_scene = FALSE;
}

int
cogl_onscreen_get_buffer_age (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys;

  _COGL_RETURN_VAL_IF_FAIL  (framebuffer->type == COGL_FRAMEBUFFER_TYPE_ONSCREEN, 0);

  winsys = _cogl_framebuffer_get_winsys (framebuffer);

  if (!winsys->onscreen_get_buffer_age)
    return 0;

  return winsys->onscreen_get_buffer_age (onscreen);
}

#ifdef COGL_HAS_X11_SUPPORT
void
cogl_x11_onscreen_set_foreign_window_xid (CoglOnscreen *onscreen,
                                          uint32_t xid,
                                          CoglOnscreenX11MaskCallback update,
                                          void *user_data)
{
  /* We don't wan't applications to get away with being lazy here and not
   * passing an update callback... */
  _COGL_RETURN_IF_FAIL (update);

  onscreen->foreign_xid = xid;
  onscreen->foreign_update_mask_callback = update;
  onscreen->foreign_update_mask_data = user_data;
}

uint32_t
cogl_x11_onscreen_get_window_xid (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);

  if (onscreen->foreign_xid)
    return onscreen->foreign_xid;
  else
    {
      const CoglWinsysVtable *winsys = _cogl_framebuffer_get_winsys (framebuffer);

      /* This should only be called for x11 onscreens */
      _COGL_RETURN_VAL_IF_FAIL (winsys->onscreen_x11_get_window_xid != NULL, 0);

      return winsys->onscreen_x11_get_window_xid (onscreen);
    }
}

uint32_t
cogl_x11_onscreen_get_visual_xid (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys = _cogl_framebuffer_get_winsys (framebuffer);
  XVisualInfo *visinfo;
  uint32_t id;

  /* This should only be called for xlib based onscreens */
  _COGL_RETURN_VAL_IF_FAIL (winsys->xlib_get_visual_info != NULL, 0);

  visinfo = winsys->xlib_get_visual_info ();
  id = (uint32_t)visinfo->visualid;

  XFree (visinfo);
  return id;
}
#endif /* COGL_HAS_X11_SUPPORT */

#ifdef COGL_HAS_WIN32_SUPPORT

void
cogl_win32_onscreen_set_foreign_window (CoglOnscreen *onscreen,
                                        HWND hwnd)
{
  onscreen->foreign_hwnd = hwnd;
}

HWND
cogl_win32_onscreen_get_window (CoglOnscreen *onscreen)
{
  if (onscreen->foreign_hwnd)
    return onscreen->foreign_hwnd;
  else
    {
      CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
      const CoglWinsysVtable *winsys =
        _cogl_framebuffer_get_winsys (framebuffer);

      /* This should only be called for win32 onscreens */
      _COGL_RETURN_VAL_IF_FAIL (winsys->onscreen_win32_get_window != NULL, 0);

      return winsys->onscreen_win32_get_window (onscreen);
    }
}

#endif /* COGL_HAS_WIN32_SUPPORT */

CoglFrameClosure *
cogl_onscreen_add_frame_callback (CoglOnscreen *onscreen,
                                  CoglFrameCallback callback,
                                  void *user_data,
                                  CoglUserDataDestroyCallback destroy)
{
  return _cogl_closure_list_add (&onscreen->frame_closures,
                                 callback,
                                 user_data,
                                 destroy);
}

void
cogl_onscreen_remove_frame_callback (CoglOnscreen *onscreen,
                                     CoglFrameClosure *closure)
{
  _COGL_RETURN_IF_FAIL (closure);

  _cogl_closure_disconnect (closure);
}

void
cogl_onscreen_set_swap_throttled (CoglOnscreen *onscreen,
                                  CoglBool throttled)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  framebuffer->config.swap_throttled = throttled;
  if (framebuffer->allocated)
    {
      const CoglWinsysVtable *winsys =
        _cogl_framebuffer_get_winsys (framebuffer);
      winsys->onscreen_update_swap_throttled (onscreen);
    }
}

void
cogl_onscreen_show (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  const CoglWinsysVtable *winsys;

  if (!framebuffer->allocated)
    {
      if (!cogl_framebuffer_allocate (framebuffer, NULL))
        return;
    }

  winsys = _cogl_framebuffer_get_winsys (framebuffer);
  if (winsys->onscreen_set_visibility)
    winsys->onscreen_set_visibility (onscreen, TRUE);
}

void
cogl_onscreen_hide (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);

  if (framebuffer->allocated)
    {
      const CoglWinsysVtable *winsys =
        _cogl_framebuffer_get_winsys (framebuffer);
      if (winsys->onscreen_set_visibility)
        winsys->onscreen_set_visibility (onscreen, FALSE);
    }
}

void
_cogl_onscreen_notify_frame_sync (CoglOnscreen *onscreen, CoglFrameInfo *info)
{
  notify_event (onscreen, COGL_FRAME_EVENT_SYNC, info);
}

void
_cogl_onscreen_notify_complete (CoglOnscreen *onscreen, CoglFrameInfo *info)
{
  notify_event (onscreen, COGL_FRAME_EVENT_COMPLETE, info);
}

void
_cogl_onscreen_notify_resize (CoglOnscreen *onscreen)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);

  _cogl_closure_list_invoke (&onscreen->resize_closures,
                             CoglOnscreenResizeCallback,
                             onscreen,
                             framebuffer->width,
                             framebuffer->height);
}

void
_cogl_framebuffer_winsys_update_size (CoglFramebuffer *framebuffer,
                                      int width, int height)
{
  if (framebuffer->width == width && framebuffer->height == height)
    return;

  framebuffer->width = width;
  framebuffer->height = height;

  cogl_framebuffer_set_viewport (framebuffer, 0, 0, width, height);

  if (!(framebuffer->context->private_feature_flags &
        COGL_PRIVATE_FEATURE_DIRTY_EVENTS))
    _cogl_onscreen_queue_full_dirty (COGL_ONSCREEN (framebuffer));
}

void
cogl_onscreen_set_resizable (CoglOnscreen *onscreen,
                             CoglBool resizable)
{
  CoglFramebuffer *framebuffer;
  const CoglWinsysVtable *winsys;

  if (onscreen->resizable == resizable)
    return;

  onscreen->resizable = resizable;

  framebuffer = COGL_FRAMEBUFFER (onscreen);
  if (framebuffer->allocated)
    {
      winsys = _cogl_framebuffer_get_winsys (COGL_FRAMEBUFFER (onscreen));

      if (winsys->onscreen_set_resizable)
        winsys->onscreen_set_resizable (onscreen, resizable);
    }
}

CoglBool
cogl_onscreen_get_resizable (CoglOnscreen *onscreen)
{
  return onscreen->resizable;
}

CoglOnscreenResizeClosure *
cogl_onscreen_add_resize_callback (CoglOnscreen *onscreen,
                                   CoglOnscreenResizeCallback callback,
                                   void *user_data,
                                   CoglUserDataDestroyCallback destroy)
{
  return _cogl_closure_list_add (&onscreen->resize_closures,
                                 callback,
                                 user_data,
                                 destroy);
}

void
cogl_onscreen_remove_resize_callback (CoglOnscreen *onscreen,
                                      CoglOnscreenResizeClosure *closure)
{
  _cogl_closure_disconnect (closure);
}

CoglOnscreenDirtyClosure *
cogl_onscreen_add_dirty_callback (CoglOnscreen *onscreen,
                                  CoglOnscreenDirtyCallback callback,
                                  void *user_data,
                                  CoglUserDataDestroyCallback destroy)
{
  return _cogl_closure_list_add (&onscreen->dirty_closures,
                                 callback,
                                 user_data,
                                 destroy);
}

void
cogl_onscreen_remove_dirty_callback (CoglOnscreen *onscreen,
                                     CoglOnscreenDirtyClosure *closure)
{
  _COGL_RETURN_IF_FAIL (closure);

  _cogl_closure_disconnect (closure);
}

int64_t
cogl_onscreen_get_frame_counter (CoglOnscreen *onscreen)
{
  return onscreen->frame_counter;
}
