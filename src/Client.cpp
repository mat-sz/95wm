#include "Client.hpp"
#include <boost/log/trivial.hpp>
#include "util.hpp"

Client::Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window)
    : conn_(conn),
      screen_(screen),
      window_(window),
      frame_(0),
      surface_(nullptr)
{
  CreateFrame();
}

void Client::CreateFrame()
{
  frame_ = xcb_generate_id(conn_);

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

  const uint16_t frame_width = geometry->width + BORDER_WIDTH * 2;
  const uint16_t frame_height = geometry->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT;
  const uint32_t values[] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

  xcb_create_window(conn_, screen_->root_depth, frame_,
                    screen_->root, geometry->x, geometry->y, frame_width,
                    frame_height, 0, XCB_COPY_FROM_PARENT,
                    screen_->root_visual, XCB_CW_EVENT_MASK, values);

  xcb_change_save_set(conn_, XCB_SET_MODE_INSERT, window_);
  xcb_reparent_window(conn_, window_, frame_, BORDER_WIDTH, TITLEBAR_HEIGHT + BORDER_WIDTH);
  xcb_map_window(conn_, frame_);

  DrawFrame(frame_width, frame_height);
}

void Client::DestroyFrame()
{
  if (frame_ == 0)
  {
    return;
  }

  xcb_unmap_window(conn_, frame_);
  xcb_reparent_window(conn_, window_, screen_->root, 0, 0);
  xcb_change_save_set(conn_, XCB_SET_MODE_DELETE, window_);
  xcb_destroy_window(conn_, frame_);
  frame_ = 0;
}

void Client::DrawFrame(uint16_t frame_width, uint16_t frame_height)
{
  if (frame_ == 0)
  {
    return;
  }

  if (surface_ != nullptr)
  {
    cairo_surface_finish(surface_);
    cairo_surface_destroy(surface_);
  }

  xcb_visualtype_t *visualtype = FindVisualtype(screen_);
  surface_ = cairo_xcb_surface_create(conn_, frame_, visualtype, frame_width, frame_height);
  cairo_t *context = cairo_create(surface_);

  cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);
  cairo_rectangle(context, 0, 0, frame_width, frame_height);
  cairo_set_source_rgb(context, 0.765, 0.765, 0.765);
  cairo_fill(context);

  cairo_rectangle(context, 2, 2, frame_width - 3, frame_height - 3);
  cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  cairo_set_line_width(context, 1.0);
  cairo_stroke(context);

  cairo_rectangle(context, 3, 3, frame_width - 6, 18);
  cairo_set_source_rgb(context, 0, 0, 0.51);
  cairo_fill(context);

  cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  cairo_select_font_face(context, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(context, 12);

  cairo_font_options_t *font_options = cairo_font_options_create();
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_NONE);
  cairo_set_font_options(context, font_options);

  cairo_move_to(context, 6, 16);
  cairo_show_text(context, "Window");
}

void Client::OnConfigureRequest(const xcb_configure_request_event_t *e)
{
  const uint16_t frame_width = e->width + BORDER_WIDTH * 2;
  const uint16_t frame_height = e->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT;

  DrawFrame(frame_width, frame_height);
}