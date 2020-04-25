extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
}
#include <memory>

class WindowManager
{
public:
  // Factory method for establishing a connection to an X server and creating a
  // WindowManager instance.
  static ::std::unique_ptr<WindowManager> Create();
  // Disconnects from the X server.
  ~WindowManager();
  // The entry point to this class. Enters the main event loop.
  void Run();

private:
  // Invoked internally by Create().
  WindowManager(xcb_connection_t *conn);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;
};
