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

  void OnMotionNotify(const xcb_motion_notify_event_t *e);

  void OnButtonPress(const xcb_button_press_event_t *e);

  void OnButtonRelease(const xcb_button_release_event_t *e);

  void OnKeyPress(const xcb_key_press_event_t *e);

  void OnKeyRelease(const xcb_key_release_event_t *e);

  void Redraw();

  xcb_window_t window_;
  xcb_window_t frame_;

private:
  void DrawFrame(uint16_t frame_width, uint16_t frame_height);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  cairo_surface_t *surface_;

  bool moving_;
  uint16_t moving_offset_x_;
  uint16_t moving_offset_y_;
};
