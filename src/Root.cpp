#include "Root.hpp"
#include <boost/log/trivial.hpp>

Root::Root(xcb_connection_t *conn, xcb_screen_t *screen)
    : conn_(conn),
      screen_(screen)
{
  window_ = screen_->root;

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(conn_, cookie, nullptr);

  xcb_visualtype_t *visualtype = nullptr;
  xcb_depth_iterator_t depths = xcb_screen_allowed_depths_iterator(screen_);
  while (depths.rem)
  {
    xcb_visualtype_iterator_t visuals = xcb_depth_visuals_iterator(depths.data);
    while (visuals.rem)
    {
      if (screen->root_visual == visuals.data->visual_id)
      {
        visualtype = visuals.data;
        break;
      }
      xcb_visualtype_next(&visuals);
    }

    if (visualtype)
    {
      break;
    }
    xcb_depth_next(&depths);
  }

  surface_ = cairo_xcb_surface_create(conn, window_, visualtype, reply->width, reply->height);
  context_ = cairo_create(surface_);
  Draw(reply->width, reply->height);
}

void Root::Draw(uint16_t width, uint16_t height)
{
  cairo_set_antialias(context_, CAIRO_ANTIALIAS_NONE);
  cairo_rectangle(context_, 0, 0, width, height);
  cairo_set_source_rgb(context_, 0.0, 0.51, 0.51);
  cairo_fill(context_);
}