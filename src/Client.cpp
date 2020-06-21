#include "Client.hpp"
#include <boost/log/trivial.hpp>
#include "util.hpp"

Client::Client(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window)
    : conn_(conn),
      screen_(screen),
      window_(window),
      frame_(0),
      surface_(nullptr),
      moving_(false),
      resizing_(RESIZE_NONE),
      focused_(false),
      close_button_(new Button())
{
  CreateFrame();

  const uint32_t values[] = {0};
  xcb_configure_window(conn_, window_,
                       XCB_CONFIG_WINDOW_BORDER_WIDTH,
                       values);
}

Client::~Client()
{
  DestroyFrame();
}

void Client::CreateFrame()
{
  frame_ = xcb_generate_id(conn_);

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

  const uint16_t frame_width = geometry->width + BORDER_WIDTH * 2 + 1;
  const uint16_t frame_height = geometry->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1;
  const uint32_t values[] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                             XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                             XCB_EVENT_MASK_FOCUS_CHANGE |
                             XCB_EVENT_MASK_POINTER_MOTION |
                             XCB_EVENT_MASK_BUTTON_MOTION |
                             XCB_EVENT_MASK_BUTTON_PRESS |
                             XCB_EVENT_MASK_BUTTON_RELEASE |
                             XCB_EVENT_MASK_KEY_PRESS |
                             XCB_EVENT_MASK_KEY_RELEASE |
                             XCB_EVENT_MASK_FOCUS_CHANGE};

  xcb_create_window(conn_, screen_->root_depth, frame_,
                    screen_->root, geometry->x, geometry->y, frame_width,
                    frame_height, 0, XCB_COPY_FROM_PARENT,
                    screen_->root_visual, XCB_CW_EVENT_MASK, values);

  xcb_change_save_set(conn_, XCB_SET_MODE_INSERT, window_);
  xcb_reparent_window(conn_, window_, frame_, BORDER_WIDTH + 1, TITLEBAR_HEIGHT + BORDER_WIDTH + 1);
  xcb_map_window(conn_, frame_);

  const uint32_t event_mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                              XCB_EVENT_MASK_PROPERTY_CHANGE |
                              XCB_EVENT_MASK_FOCUS_CHANGE;
  xcb_change_window_attributes(conn_, window_, XCB_CW_EVENT_MASK, &event_mask);

  const uint32_t back_pixel = 0xc0c0c0;
  xcb_change_window_attributes(conn_, frame_, XCB_CW_BACK_PIXEL, &back_pixel);
  xcb_clear_area(conn_, 0, frame_, 0, 0, 0, 0);

  close_button_->width_ = 16;
  close_button_->height_ = 14;

  visualtype_ = FindVisualtype(screen_);
  Redraw();
}

void Client::DestroyFrame()
{
  if (frame_ == 0)
  {
    return;
  }

  if (surface_)
  {
    cairo_surface_finish(surface_);
    cairo_surface_destroy(surface_);
  }

  xcb_unmap_window(conn_, frame_);
  xcb_reparent_window(conn_, window_, screen_->root, 0, 0);
  xcb_change_save_set(conn_, XCB_SET_MODE_DELETE, window_);
  xcb_destroy_window(conn_, frame_);
  frame_ = 0;
}

void Client::Redraw()
{
  if (frame_ == 0)
  {
    return;
  }

  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, window_);
  xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

  const uint16_t frame_width = geometry->width + BORDER_WIDTH * 2 + 1;
  const uint16_t frame_height = geometry->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1;

  xcb_pixmap_t pixmap = xcb_generate_id(conn_);
  xcb_create_pixmap(conn_, screen_->root_depth, pixmap, frame_, frame_width, frame_height);
  xcb_aux_sync(conn_);

  surface_ = cairo_xcb_surface_create(conn_, pixmap, visualtype_, frame_width, frame_height);

  DrawFrame(frame_width, frame_height);

  xcb_change_window_attributes(conn_, frame_, XCB_CW_BACK_PIXMAP, &pixmap);
  xcb_clear_area(conn_, 0, frame_, 0, 0, frame_width, frame_height);

  cairo_surface_finish(surface_);
  cairo_surface_destroy(surface_);
  surface_ = nullptr;
}

void Client::DrawFrame(uint16_t frame_width, uint16_t frame_height)
{
  if (frame_ == 0)
  {
    return;
  }

  cairo_xcb_surface_set_size(surface_, frame_width, frame_height);
  cairo_t *context = cairo_create(surface_);

  cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);

  cairo_set_source_rgb(context, 0.753, 0.753, 0.753);
  cairo_rectangle(context, 0, 0, frame_width, frame_height);
  cairo_fill(context);

  cairo_rectangle(context, 2, 2, frame_width - 2, frame_height - 2);
  cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  cairo_set_line_width(context, 1.0);
  cairo_stroke(context);

  cairo_set_source_rgb(context, 0.502, 0.502, 0.502);
  cairo_move_to(context, frame_width, 1);
  cairo_line_to(context, frame_width, frame_height);
  cairo_stroke(context);
  cairo_move_to(context, frame_width, frame_height);
  cairo_line_to(context, 1, frame_height);
  cairo_stroke(context);

  cairo_rectangle(context, 4, 4, frame_width - 7, 18);
  if (focused_)
  {
    cairo_set_source_rgb(context, 0, 0, 0.502);
  }
  else
  {
    cairo_set_source_rgb(context, 0.502, 0.502, 0.502);
  }
  cairo_fill(context);

  xcb_get_property_cookie_t cookie = xcb_get_property(conn_, 0, window_, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100);
  xcb_get_property_reply_t *reply = xcb_get_property_reply(conn_, cookie, nullptr);

  if (focused_)
  {
    cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  }
  else
  {
    cairo_set_source_rgb(context, 0.753, 0.753, 0.753);
  }
  cairo_select_font_face(context, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(context, 12);

  cairo_font_options_t *font_options = cairo_font_options_create();
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_NONE);
  cairo_set_font_options(context, font_options);

  cairo_move_to(context, 7, 16);
  cairo_show_text(context, (char *)xcb_get_property_value(reply));

  close_button_->x_ = frame_width - 21;
  close_button_->y_ = 6;
  close_button_->Draw(surface_);

  cairo_surface_flush(surface_);
  cairo_destroy(context);
}

void Client::OnConfigureRequest(const xcb_configure_request_event_t *e)
{
  const uint16_t frame_width = e->width + BORDER_WIDTH * 2 + 1;
  const uint16_t frame_height = e->height + BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1;

  DrawFrame(frame_width, frame_height);
}

void Client::OnMotionNotify(const xcb_motion_notify_event_t *e)
{
  if (moving_)
  {
    const uint32_t geometry[] = {e->root_x - moving_offset_x_, e->root_y - moving_offset_y_};
    xcb_configure_window(conn_, frame_,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                         geometry);
    xcb_flush(conn_);
  }

  if (resizing_ != RESIZE_NONE)
  {
    switch (resizing_)
    {
    case RESIZE_NE:
    {
      const uint32_t geometry_window[] = {(resizing_original_x_ + resizing_original_width_ - e->root_x - (BORDER_WIDTH * 2 + 1)),
                                          (resizing_original_y_ + resizing_original_height_ - e->root_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1))};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->root_x, e->root_y,
                                         (resizing_original_x_ + resizing_original_width_ - e->root_x),
                                         (resizing_original_y_ + resizing_original_height_ - e->root_y)};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                               XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_NW:
    {
      const uint32_t geometry_window[] = {e->event_x - (BORDER_WIDTH * 2 + 1),
                                          (resizing_original_y_ + resizing_original_height_ - e->root_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1))};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->root_y,
                                         e->event_x,
                                         (resizing_original_y_ + resizing_original_height_ - e->root_y)};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_Y |
                               XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_N:
    {
      const uint32_t geometry_window[] = {(resizing_original_y_ + resizing_original_height_ - e->root_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1))};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->root_y,
                                         (resizing_original_y_ + resizing_original_height_ - e->root_y)};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_E:
    {
      const uint32_t geometry_window[] = {(resizing_original_x_ + resizing_original_width_ - e->root_x - (BORDER_WIDTH * 2 + 1))};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->root_x, (resizing_original_x_ + resizing_original_width_ - e->root_x)};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH,
                           geometry_frame);
    }
    break;
    case RESIZE_SE:
    {
      const uint32_t geometry_window[] = {(resizing_original_x_ + resizing_original_width_ - e->root_x - (BORDER_WIDTH * 2 + 1)),
                                          e->event_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1)};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->root_x,
                                         (resizing_original_x_ + resizing_original_width_ - e->root_x),
                                         e->event_y};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_X |
                               XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_S:
    {
      const uint32_t geometry_window[] = {e->event_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1)};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->event_y};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_SW:
    {
      const uint32_t geometry_window[] = {e->event_x - (BORDER_WIDTH * 2 + 1),
                                          e->event_y - (BORDER_WIDTH * 2 + TITLEBAR_HEIGHT + 1)};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->event_x, e->event_y};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           geometry_frame);
    }
    break;
    case RESIZE_W:
    {
      const uint32_t geometry_window[] = {e->event_x - (BORDER_WIDTH * 2 + 1)};
      xcb_configure_window(conn_, window_,
                           XCB_CONFIG_WINDOW_WIDTH,
                           geometry_window);

      const uint32_t geometry_frame[] = {e->event_x};
      xcb_configure_window(conn_, frame_,
                           XCB_CONFIG_WINDOW_WIDTH,
                           geometry_frame);
    }
    break;
    }

    xcb_flush(conn_);
    Redraw();
  }
}

void Client::OnButtonPress(const xcb_button_press_event_t *e)
{
  if (e->detail == XCB_BUTTON_INDEX_1)
  {
    moving_ = false;
    resizing_ = RESIZE_NONE;

    if (e->event_y > BORDER_WIDTH && e->event_y < (TITLEBAR_HEIGHT + BORDER_WIDTH * 2 + 1))
    {
      if (close_button_->CheckRect(e->event_x, e->event_y))
      {
        close_button_->pressed_ = true;
        close_button_->Draw(surface_);
      }
      else
      {
        moving_ = true;
        moving_offset_x_ = e->event_x;
        moving_offset_y_ = e->event_y;
      }
    }
    else
    {
      xcb_get_geometry_cookie_t cookie = xcb_get_geometry(conn_, frame_);
      xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, cookie, nullptr);

      resizing_original_x_ = geometry->x;
      resizing_original_y_ = geometry->y;
      resizing_original_width_ = geometry->width;
      resizing_original_height_ = geometry->height;

      if (e->event_x <= BORDER_WIDTH && e->event_y <= BORDER_WIDTH)
      {
        resizing_ = RESIZE_NE;
      }
      else if (e->event_x <= BORDER_WIDTH && e->event_y < geometry->height - BORDER_WIDTH * 2)
      {
        resizing_ = RESIZE_N;
      }
      else if (e->event_y <= BORDER_WIDTH)
      {
        resizing_ = RESIZE_NW;
      }
      else if (e->event_x <= BORDER_WIDTH && e->event_y < geometry->height - BORDER_WIDTH * 2)
      {
        resizing_ = RESIZE_E;
      }
      else if (e->event_x <= BORDER_WIDTH)
      {
        resizing_ = RESIZE_SE;
      }
      else if (e->event_x >= geometry->width - BORDER_WIDTH * 2 && e->event_y < geometry->height - BORDER_WIDTH * 2)
      {
        resizing_ = RESIZE_W;
      }
      else if (e->event_x >= geometry->width - BORDER_WIDTH * 2)
      {
        resizing_ = RESIZE_SW;
      }
      else
      {
        resizing_ = RESIZE_S;
      }

      // xcb_warp_pointer(conn_, XCB_NONE, frame_, 0, 0, 0, 0, geometry->width, geometry->height);
    }
  }
}

void Client::OnButtonRelease(const xcb_button_release_event_t *e)
{
  if (e->detail == XCB_BUTTON_INDEX_1)
  {
    moving_ = false;
    resizing_ = false;

    if (close_button_->pressed_)
    {
      close_button_->pressed_ = false;
      close_button_->Draw(surface_);

      if (close_button_->CheckRect(e->event_x, e->event_y))
      {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom_unchecked(conn_, false, 16, "WM_DELETE_WINDOW");
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn_, cookie, nullptr);

        xcb_client_message_event_t event;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.window = window_;
        event.format = 32;
        event.sequence = 0;
        event.type = 0;
        event.data.data32[1] = e->time;
        event.data.data32[0] = reply->atom;

        xcb_send_event(conn_, false, window_, XCB_EVENT_MASK_NO_EVENT, (char *)&event);
      }
    }
  }
}

void Client::OnKeyPress(const xcb_key_press_event_t *e) {}

void Client::OnKeyRelease(const xcb_key_release_event_t *e) {}

void Client::OnFocusIn(const xcb_focus_in_event_t *e)
{
  if (!focused_)
  {
    focused_ = true;
    Redraw();
  }
}

void Client::OnFocusOut(const xcb_focus_out_event_t *e)
{
  if (focused_)
  {
    focused_ = false;
    Redraw();
  }
}