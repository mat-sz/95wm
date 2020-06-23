extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>
#include "Button.hpp"

#define TITLEBAR_HEIGHT 21
#define BORDER_WIDTH 3

#define RESIZE_NONE 0
#define RESIZE_E 1
#define RESIZE_SE 2
#define RESIZE_S 3
#define RESIZE_SW 4
#define RESIZE_W 5
#define RESIZE_NW 6
#define RESIZE_N 7
#define RESIZE_NE 8

class Client
{
public:
  Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window);

  ~Client();

  void OnConfigureRequest(const xcb_configure_request_event_t *e);

  void OnMotionNotify(const xcb_motion_notify_event_t *e);

  void OnButtonPress(const xcb_button_press_event_t *e);

  void OnButtonRelease(const xcb_button_release_event_t *e);

  void OnKeyPress(const xcb_key_press_event_t *e);

  void OnKeyRelease(const xcb_key_release_event_t *e);

  void OnFocusIn(const xcb_focus_in_event_t *e);

  void OnFocusOut(const xcb_focus_out_event_t *e);

  void Redraw();

  xcb_connection_t *conn_;
  xcb_screen_t *screen_;
  xcb_window_t window_;
  xcb_window_t frame_;

private:
  void CreateFrame();
  void DestroyFrame();

  void DrawFrame(uint16_t frame_width, uint16_t frame_height);

  xcb_visualtype_t *visualtype_;

  cairo_surface_t *surface_;

  cairo_t *context_;

  bool moving_;
  uint16_t moving_offset_x_;
  uint16_t moving_offset_y_;

  char resizing_;
  uint16_t resizing_original_x_;
  uint16_t resizing_original_y_;
  uint16_t resizing_original_width_;
  uint16_t resizing_original_height_;
  uint16_t resizing_previous_mouse_x_;
  uint16_t resizing_previous_mouse_y_;

  bool focused_;

  std::unique_ptr<Button> close_button_;
};
