#include "Button.hpp"

Button::Button()
    : pressed_(false)
{
}

Button::~Button()
{
}

void Button::Draw(cairo_surface_t *surface)
{
  cairo_t *context = cairo_create(surface);

  cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);
  cairo_rectangle(context, x_, y_, width_, height_);
  cairo_set_source_rgb(context, 0.753, 0.753, 0.753);
  cairo_fill(context);

  cairo_set_line_width(context, 1.0);

  if (pressed_)
  {
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
  }
  else
  {
    cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  }
  cairo_move_to(context, x_, y_ + 1);
  cairo_line_to(context, x_ + width_, y_ + 1);
  cairo_stroke(context);

  cairo_move_to(context, x_ + 1, y_);
  cairo_line_to(context, x_ + 1, y_ + height_);
  cairo_stroke(context);

  cairo_set_source_rgb(context, 0.502, 0.502, 0.502);
  if (pressed_)
  {
    cairo_move_to(context, x_ + 1, y_ + 2);
    cairo_line_to(context, x_ + width_ - 2, y_ + 2);
    cairo_stroke(context);

    cairo_move_to(context, x_ + 2, y_ + 2);
    cairo_line_to(context, x_ + 2, y_ + height_ - 2);
    cairo_stroke(context);
  }
  else
  {
    cairo_move_to(context, x_ + 1, y_ + height_ - 1);
    cairo_line_to(context, x_ + width_ - 1, y_ + height_ - 1);
    cairo_stroke(context);

    cairo_move_to(context, x_ + width_ - 1, y_ + 1);
    cairo_line_to(context, x_ + width_ - 1, y_ + height_);
    cairo_stroke(context);
  }

  if (pressed_)
  {
    cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  }
  else
  {
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
  }
  cairo_move_to(context, x_ + width_, y_);
  cairo_line_to(context, x_ + width_, y_ + height_);
  cairo_stroke(context);

  cairo_move_to(context, x_, y_ + height_);
  cairo_line_to(context, x_ + width_, y_ + height_);
  cairo_stroke(context);
  cairo_surface_flush(surface);
  cairo_destroy(context);
}

bool Button::CheckRect(uint16_t x, uint16_t y)
{
  return (x > x_ && x < (x_ + width_) && y > y_ && y < (y_ + height_));
}