extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>

#define TITLEBAR_HEIGHT 21
#define BORDER_WIDTH 3

class Client
{
public:
  Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window);

  void CreateFrame();
  void DestroyFrame();

  void OnConfigureRequest(const xcb_configure_request_event_t *e);

  void Redraw();

private:
  void DrawFrame(uint16_t frame_width, uint16_t frame_height);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  xcb_window_t window_;
  xcb_window_t frame_;

  cairo_surface_t *surface_;
};
