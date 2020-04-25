#include "Client.hpp"
#include <boost/log/trivial.hpp>

Client::Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window)
    : conn_(conn),
      screen_(screen),
      window_(window)
{
  const uint16_t TITLEBAR_HEIGHT = 30;
  const uint16_t BORDER_WIDTH = 5;

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(conn_, cookie, nullptr);

  const uint16_t frame_width = reply->width + BORDER_WIDTH * 2;
  const uint16_t frame_height = reply->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT;

  xcb_window_t frame = xcb_generate_id(conn_);
  xcb_create_window(conn_, screen_->root_depth, frame,
                    screen->root, reply->x, reply->y, frame_width,
                    frame_height, 0, XCB_COPY_FROM_PARENT,
                    screen->root_visual, 0, nullptr);

  xcb_change_save_set(conn_, XCB_SET_MODE_INSERT, window_);
  xcb_reparent_window(conn_, window_, frame, BORDER_WIDTH, TITLEBAR_HEIGHT + BORDER_WIDTH);
  xcb_map_window(conn_, frame);

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

  surface_ = cairo_xcb_surface_create(conn, frame, visualtype, frame_width, frame_height);
  context_ = cairo_create(surface_);
  DrawFrame(frame_width, frame_height);
}

void Client::DrawFrame(uint16_t width, uint16_t height)
{
  cairo_rectangle(context_, 0, 0, width, height);
  cairo_set_source_rgb(context_, 1, 0, 0);
  cairo_fill(context_);
}