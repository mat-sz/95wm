#include "Taskbar.hpp"
#include <boost/log/trivial.hpp>
#include "util.hpp"

Taskbar::Taskbar(xcb_connection_t *conn, xcb_screen_t *screen)
    : conn_(conn),
      screen_(screen)
{
  window_ = xcb_generate_id(conn_);

  const uint32_t values[] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                             XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                             XCB_EVENT_MASK_FOCUS_CHANGE |
                             XCB_EVENT_MASK_POINTER_MOTION |
                             XCB_EVENT_MASK_BUTTON_MOTION |
                             XCB_EVENT_MASK_BUTTON_PRESS |
                             XCB_EVENT_MASK_BUTTON_RELEASE |
                             XCB_EVENT_MASK_KEY_PRESS |
                             XCB_EVENT_MASK_KEY_RELEASE |
                             XCB_EVENT_MASK_FOCUS_CHANGE};

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, screen_->root);
  geometry_ = xcb_get_geometry_reply(conn_, cookie, nullptr);

  xcb_create_window(conn_, screen_->root_depth, window_,
                    screen_->root, 0, geometry_->height - TASKBAR_HEIGHT, geometry_->width, TASKBAR_HEIGHT, 0, XCB_COPY_FROM_PARENT,
                    screen_->root_visual, XCB_CW_EVENT_MASK, values);
  xcb_change_save_set(conn_, XCB_SET_MODE_INSERT, window_);
  xcb_map_window(conn_, window_);
  xcb_flush(conn_);
  Draw();
}

Taskbar::~Taskbar()
{
  xcb_unmap_window(conn_, window_);
  xcb_destroy_window(conn_, window_);
}

void Taskbar::Draw()
{
  xcb_visualtype_t *visualtype = FindVisualtype(screen_);
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

  xcb_pixmap_t pixmap = xcb_generate_id(conn_);
  xcb_create_pixmap(conn_, screen_->root_depth, pixmap, window_, geometry->width, geometry->height);
  xcb_aux_sync(conn_);

  cairo_surface_t *surface = cairo_xcb_surface_create(conn_, pixmap, visualtype, geometry->width, geometry->height);
  cairo_t *context = cairo_create(surface);

  cairo_rectangle(context, 0, 0, geometry->width, geometry->height);
  cairo_set_source_rgb(context, 0.753, 0.753, 0.753);
  cairo_fill(context);
  cairo_destroy(context);
  cairo_surface_flush(surface);

  xcb_change_window_attributes(conn_, window_, XCB_CW_BACK_PIXMAP, &pixmap);
  xcb_clear_area(conn_, 0, window_, 0, 0, 0, 0);

  cairo_surface_finish(surface);
  cairo_surface_destroy(surface);
}