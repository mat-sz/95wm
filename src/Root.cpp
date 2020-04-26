#include "Root.hpp"
#include <boost/log/trivial.hpp>
#include "util.hpp"

Root::Root(xcb_connection_t *conn, xcb_screen_t *screen)
    : conn_(conn),
      screen_(screen),
      window_(screen_->root),
      surface_(nullptr)
{
  Draw();
}

void Root::Draw()
{
  if (surface_ != nullptr)
  {
    cairo_surface_finish(surface_);
    cairo_surface_destroy(surface_);
  }

  xcb_visualtype_t *visualtype = FindVisualtype(screen_);
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

  surface_ = cairo_xcb_surface_create(conn_, window_, visualtype, geometry->width, geometry->height);
  cairo_t *context = cairo_create(surface_);

  cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);
  cairo_rectangle(context, 0, 0, geometry->width, geometry->height);
  cairo_set_source_rgb(context, 0.0, 0.51, 0.51);
  cairo_fill(context);
}