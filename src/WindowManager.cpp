#include "WindowManager.hpp"
#include <memory>
#include <utility>
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

  const uint32_t event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                              XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                              XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                              XCB_EVENT_MASK_PROPERTY_CHANGE |
                              XCB_EVENT_MASK_EXPOSURE;
  xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn_, root, XCB_CW_EVENT_MASK, &event_mask);

  if (xcb_request_check(conn_, cookie))
  {
    BOOST_LOG_TRIVIAL(error) << "Detected another WM";
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Framing existing windows";

  xcb_grab_server(conn_);
  xcb_query_tree_cookie_t tree_cookie = xcb_query_tree_unchecked(conn_, root);
  xcb_query_tree_reply_t *tree_reply = xcb_query_tree_reply(conn_, tree_cookie, nullptr);
  if (tree_reply)
  {
    xcb_window_t *windows = xcb_query_tree_children(tree_reply);
    if (windows)
    {
      const uint16_t length = xcb_query_tree_children_length(tree_reply);
      for (uint16_t i = 0; i < length; i++)
      {
        BOOST_LOG_TRIVIAL(info) << "Found window";
        const xcb_window_t window = windows[i];
        xcb_get_window_attributes_cookie_t attributes_cookie = xcb_get_window_attributes_unchecked(conn_, window);
        xcb_get_window_attributes_reply_t *attributes = xcb_get_window_attributes_reply(conn_, attributes_cookie, nullptr);

        xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry_unchecked(conn_, window);
        xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn_, geometry_cookie, nullptr);

        if (!geometry || !attributes || attributes->map_state == XCB_MAP_STATE_UNMAPPED || attributes->override_redirect)
        {
          continue;
        }

        BOOST_LOG_TRIVIAL(info) << "Framing window";
        Client *client = new Client(conn_, screen_, window);
        clients_[window] = client;
        xcb_map_window(conn_, window);
        xcb_flush(conn_);
      }
    }
  }
  xcb_ungrab_server(conn_);
  xcb_flush(conn_);

  root_ = std::unique_ptr<Root>(new Root(conn_, screen_));
  taskbar_ = std::unique_ptr<Taskbar>(new Taskbar(conn_, screen_));

  BOOST_LOG_TRIVIAL(info) << "Starting event loop";

  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(conn_)))
  {
    switch (XCB_EVENT_RESPONSE_TYPE(event))
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
    case XCB_EXPOSE:
      OnExpose((xcb_expose_event_t *)event);
      break;
    case XCB_MOTION_NOTIFY:
      OnMotionNotify((xcb_motion_notify_event_t *)event);
      break;
    case XCB_BUTTON_PRESS:
      OnButtonPress((xcb_button_release_event_t *)event);
      break;
    case XCB_BUTTON_RELEASE:
      OnButtonRelease((xcb_button_release_event_t *)event);
      break;
    case XCB_KEY_PRESS:
      OnKeyPress((xcb_key_press_event_t *)event);
      break;
    case XCB_KEY_RELEASE:
      OnKeyRelease((xcb_key_release_event_t *)event);
      break;
    case XCB_FOCUS_IN:
      OnFocusIn((xcb_focus_in_event_t *)event);
      break;
    case XCB_FOCUS_OUT:
      OnFocusOut((xcb_focus_out_event_t *)event);
      break;
    case XCB_PROPERTY_NOTIFY:
      OnPropertyNotify((xcb_property_notify_event_t *)event);
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
  BOOST_LOG_TRIVIAL(info) << "OnDestroyNotify";

  if (clients_.count(e->window))
  {
    Client *client = clients_[e->window];
    clients_.erase(e->window);
    delete client;
  }
}

void WindowManager::OnReparentNotify(const xcb_reparent_notify_event_t *e)
{
}

void WindowManager::OnConfigureRequest(const xcb_configure_request_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnConfigureRequest";

  const uint32_t geometry[] = {e->x, e->y, e->width, e->height};
  const uint32_t values[] = {0};
  xcb_configure_window(conn_, e->window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       geometry);
  xcb_configure_window(conn_, e->window,
                       XCB_CONFIG_WINDOW_BORDER_WIDTH,
                       values);
  xcb_flush(conn_);

  if (clients_.count(e->window))
  {
    Client *client = clients_[e->window];
    client->OnConfigureRequest(e);
  }
}

void WindowManager::OnConfigureNotify(const xcb_configure_notify_event_t *e)
{
}

void WindowManager::OnMapRequest(const xcb_map_request_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnMapRequest";

  Client *client = new Client(conn_, screen_, e->window);
  clients_[e->window] = client;
  xcb_map_window(conn_, e->window);
  xcb_flush(conn_);
}

void WindowManager::OnUnmapNotify(const xcb_unmap_notify_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnUnmapNotify";

  if (e->event != screen_->root)
  {
    if (clients_.count(e->window))
    {
      Client *client = clients_[e->window];
      clients_.erase(e->window);
      delete client;
    }
  }

  // Not exactly the most performant way.
  // TODO: Rewrite in OnExpose.
  std::for_each(clients_.begin(), clients_.end(), [](std::pair<xcb_window_t, Client *> client) {
    client.second->Redraw();
  });
}

void WindowManager::OnExpose(const xcb_expose_event_t *e)
{
}

void WindowManager::OnMotionNotify(const xcb_motion_notify_event_t *e)
{
  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnMotionNotify(e);
    }

    if (client.second->frame_ == e->event || client.second->window_ == e->event)
    {
      client.second->OnFocusIn(nullptr);
    }
    else
    {
      client.second->OnFocusOut(nullptr);
    }
  });
}

void WindowManager::OnButtonPress(const xcb_button_press_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnButtonPress";

  const uint32_t values[] = {XCB_STACK_MODE_ABOVE};
  xcb_configure_window(conn_, e->event,
                       XCB_CONFIG_WINDOW_STACK_MODE,
                       values);

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnButtonPress(e);
    }
  });
}

void WindowManager::OnButtonRelease(const xcb_button_release_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnButtonRelease";

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnButtonRelease(e);
    }
  });
}

void WindowManager::OnKeyPress(const xcb_key_press_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnKeyPress";

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnKeyPress(e);
    }
  });
}

void WindowManager::OnKeyRelease(const xcb_key_release_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnKeyRelease";

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnKeyRelease(e);
    }
  });
}

void WindowManager::OnFocusIn(const xcb_focus_in_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnFocusIn";

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnFocusIn(e);
    }
  });
}

void WindowManager::OnFocusOut(const xcb_focus_out_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnFocusOut";

  std::for_each(clients_.begin(), clients_.end(), [e](std::pair<xcb_window_t, Client *> client) {
    if (client.second->frame_ == e->event)
    {
      client.second->OnFocusOut(e);
    }
  });
}

void WindowManager::OnPropertyNotify(const xcb_property_notify_event_t *e)
{
  BOOST_LOG_TRIVIAL(info) << "OnPropertyNotify";
}