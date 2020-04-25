extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>

#define noop

class Client
{
public:
  Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window);

private:
  void DrawFrame(uint16_t width, uint16_t height);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  xcb_window_t window_;

  cairo_surface_t *surface_;
  cairo_t *context_;
};
