#include "util.hpp"

xcb_visualtype_t *FindVisualtype(xcb_screen_t *screen)
{
  xcb_visualtype_t *visualtype = nullptr;
  xcb_depth_iterator_t depths = xcb_screen_allowed_depths_iterator(screen);

  while (depths.rem)
  {
    xcb_visualtype_iterator_t visuals = xcb_depth_visuals_iterator(depths.data);
    while (visuals.rem)
    {
      if (screen->root_visual == visuals.data->visual_id)
      {
        visualtype = visuals.data;
        break;
      }
      xcb_visualtype_next(&visuals);
    }

    if (visualtype)
    {
      break;
    }
    xcb_depth_next(&depths);
  }

  return visualtype;
}