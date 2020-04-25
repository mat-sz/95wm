#include "Client.hpp"
#include <boost/log/trivial.hpp>

Client::Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window)
    : conn_(conn),
      screen_(screen),
      window_(window)
{
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(conn_, cookie, nullptr);
  const uint32_t values[] = {0xff0000};

  xcb_window_t id = xcb_generate_id(conn_);
  xcb_create_window(conn_, screen_->root_depth, id,
                    screen->root, reply->x, reply->y, reply->width,
                    reply->height, 3, XCB_COPY_FROM_PARENT,
                    screen->root_visual, XCB_CW_BACKING_PIXEL, values);

  xcb_change_save_set(conn_, XCB_SET_MODE_INSERT, window_);
  xcb_reparent_window(conn_, window_, id, 0, 0);
  xcb_map_window(conn_, id);
}