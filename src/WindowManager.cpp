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
      screen_(xcb_setup_roots_iterator(xcb_get_setup(conn)).data)
{
}

WindowManager::~WindowManager()
{
  xcb_disconnect(conn_);
}

void WindowManager::Run()
{
  while (true)
  {
  }
}
