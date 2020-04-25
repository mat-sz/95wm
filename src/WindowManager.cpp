#include "WindowManager.hpp"

using ::std::unique_ptr;

unique_ptr<WindowManager> WindowManager::Create()
{
  // 1. Open X display.
  Display *display = XOpenDisplay(nullptr);
  if (display == nullptr)
  {
    return nullptr;
  }
  // 2. Construct WindowManager instance.
  return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display *display)
    : display_(display),
      root_(DefaultRootWindow(display_))
{
}

WindowManager::~WindowManager()
{
  XCloseDisplay(display_);
}

void WindowManager::Run()
{ /* TODO */
}
