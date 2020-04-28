extern "C"
{
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <cairo/cairo-xcb.h>
}
#include <memory>

class Button
{
public:
  Button();

  ~Button();

  void Draw(cairo_surface_t *surface);

  bool CheckRect(uint16_t x, uint16_t y);

  uint16_t x_;
  uint16_t y_;
  uint16_t width_;
  uint16_t height_;
  bool pressed_;

private:
};
