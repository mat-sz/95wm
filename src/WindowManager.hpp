extern "C"
{
#include <X11/Xlib.h>
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
  WindowManager(Display *display);

  // Handle to the underlying Xlib Display struct.
  Display *display_;
  // Handle to root window.
  const Window root_;
};
