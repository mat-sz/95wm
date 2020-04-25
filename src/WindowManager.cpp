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
  xcb_window_t root = screen_->root;

  const uint32_t event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
  xcb_void_cookie_t cookie;
  cookie = xcb_change_window_attributes_checked(conn_, root, XCB_CW_EVENT_MASK, &event_mask);

  if (xcb_request_check(conn_, cookie))
  {
    BOOST_LOG_TRIVIAL(error) << "Detected another WM";
  }

  BOOST_LOG_TRIVIAL(info) << "Starting event loop";

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(conn_)))
  {
    switch (event->response_type)
    {
    case XCB_CREATE_NOTIFY:
      OnCreateNotify((xcb_create_notify_event_t *)event);
      break;
    case XCB_DESTROY_NOTIFY:
      OnDestroyNotify((xcb_destroy_notify_event_t *)event);
      break;
    case XCB_REPARENT_NOTIFY:
      OnReparentNotify((xcb_reparent_notify_event_t *)event);
      break;
    case XCB_CONFIGURE_REQUEST:
      OnConfigureRequest((xcb_configure_request_event_t *)event);
      break;
    case XCB_CONFIGURE_NOTIFY:
      OnConfigureNotify((xcb_configure_notify_event_t *)event);
      break;
    case XCB_MAP_REQUEST:
      OnMapRequest((xcb_map_request_event_t *)event);
      break;
    case XCB_UNMAP_NOTIFY:
      OnUnmapNotify((xcb_unmap_notify_event_t *)event);
      break;
    default:
      noop;
    }
  }
}

void WindowManager::OnCreateNotify(const xcb_create_notify_event_t *e)
{
}

void WindowManager::OnDestroyNotify(const xcb_destroy_notify_event_t *e)
{
}

void WindowManager::OnReparentNotify(const xcb_reparent_notify_event_t *e)
{
}

void WindowManager::OnConfigureRequest(const xcb_configure_request_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "Received a configure request";
  const uint32_t values[] = {e->x, e->y, e->width, e->height};
  const uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
  xcb_configure_window(conn_, e->window, mask, values);
  xcb_flush(conn_);
}

void WindowManager::OnConfigureNotify(const xcb_configure_notify_event_t *e)
{
}

void WindowManager::OnMapRequest(const xcb_map_request_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "Received a map request";
  xcb_map_window(conn_, e->window);
  xcb_flush(conn_);
}

void WindowManager::OnUnmapNotify(const xcb_unmap_notify_event_t *e)
{
}