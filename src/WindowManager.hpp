extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
}
#include <memory>

#define noop

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

  void OnConfigureRequest(const xcb_configure_request_event_t *e);

  void OnMapRequest(const xcb_map_request_event_t *e);

  void OnConfigureNotify(const xcb_configure_notify_event_t *e);

  void OnUnmapNotify(const xcb_unmap_notify_event_t *e);

  void OnDestroyNotify(const xcb_destroy_notify_event_t *e);

  void OnCreateNotify(const xcb_create_notify_event_t *e);

  void OnReparentNotify(const xcb_reparent_notify_event_t *e);

  // Handle to the XCB connection.
  xcb_connection_t *conn_;
  // Handle to the screen.
  xcb_screen_t *screen_;
};
