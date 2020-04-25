extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>

class Root
{
public:
  Root(xcb_connection_t *conn, xcb_screen_t *screen);

private:
  void Draw(uint16_t width, uint16_t height);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  xcb_window_t window_;

  cairo_surface_t *surface_;
  cairo_t *context_;
};
