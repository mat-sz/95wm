extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>

#define TASKBAR_HEIGHT 28

class Taskbar
{
public:
  Taskbar(xcb_connection_t *conn, xcb_screen_t *screen);

  void Draw();

  ~Taskbar();

private:
  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;

  xcb_get_geometry_reply_t *geometry_;

  xcb_window_t window_;
};
