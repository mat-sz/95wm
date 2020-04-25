#include "WindowManager.hpp"
#include <boost/log/trivial.hpp>

using ::std::unique_ptr;

unique_ptr<WindowManager> WindowManager::Create()
{
  xcb_connection_t *conn = xcb_connect(nullptr, nullptr);
  if (conn == nullptr || xcb_connection_has_error(conn))
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to open X display " << xcb_connection_has_error(conn);
    return nullptr;
  }

  return unique_ptr<WindowManager>(new WindowManager(conn));
}

WindowManager::WindowManager(xcb_connection_t *conn)
    : conn_(conn),
      screen_(xcb_aux_get_screen(conn, 0))
{
}

WindowManager::~WindowManager()
{
  xcb_disconnect(conn_);
}

void WindowManager::Run()
{
  xcb_grab_server(conn_);

  xcb_window_t root = screen_->root;

  const uint32_t event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
  xcb_void_cookie_t cookie;
  cookie = xcb_change_window_attributes_checked(conn_, root, XCB_CW_EVENT_MASK, &event_mask);

  if (xcb_request_check(conn_, cookie))
  {
    BOOST_LOG_TRIVIAL(error) << "Detected another WM";
  }

  xcb_generic_event_t *event;
  while (event = xcb_wait_for_event(conn_))
  {
    switch (event->response_type & ~0x80)
    {
    case XCB_CREATE_NOTIFY:
      xcb_expose_event_t *e = (xcb_expose_event_t *)event;
      break;
    case XCB_DESTROY_NOTIFY:
      xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)event;
      break;
    case XCB_REPARENT_NOTIFY:
      xcb_reparent_notify_event_t *e = (xcb_reparent_notify_event_t *)event;
      break;
    }
  }
}
