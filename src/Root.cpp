#include "Root.hpp"
#include <boost/log/trivial.hpp>
#include "util.hpp"

Root::Root(xcb_connection_t *conn, xcb_screen_t *screen)
    : conn_(conn),
      screen_(screen),
      window_(screen_->root)
{
  Draw();
}

void Root::Draw()
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
  cairo_set_source_rgb(context, 0.0, 0.51, 0.51);
  cairo_fill(context);
  cairo_destroy(context);
  cairo_surface_flush(surface);

  xcb_change_window_attributes(conn_, window_, XCB_CW_BACK_PIXMAP, &pixmap);
  xcb_clear_area(conn_, 0, window_, 0, 0, 0, 0);

  cairo_surface_finish(surface);
  cairo_surface_destroy(surface);
}