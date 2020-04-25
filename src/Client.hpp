extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
}
#include <memory>

#define noop

class Client
{
public:
  Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window);

private:
  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  xcb_window_t window_;
};
