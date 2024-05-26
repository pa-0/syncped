////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar-handle.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "defs.h"
#include "frame.h"

void frame::toolbar_handle()
{
  get_toolbar()->add_standard(false); // no realize yet
  get_toolbar()->add_checkboxes(
    {{ID_VIEW_DIRCTRL,
      _("Explorer"),
      "",
      "",
      _("Explorer"),
      pane_is_shown("DIRCTRL"),
      [&](wxCheckBox* cb)
      {
        pane_toggle("DIRCTRL");
        cb->SetValue(pane_is_shown("DIRCTRL"));
        get_toolbar()->Realize();
        if (pane_is_shown("DIRCTRL") && pane_is_shown("FILES"))
        {
          if (auto* editor = get_stc(); editor != nullptr)
          {
            m_dirctrl->expand_and_select_path(editor->path());
          }
        }
      }},

     {ID_VIEW_HISTORY,
      _("History"),
      "",
      "",
      _("History"),
      wex::config("show.History").get(false),
      [&](wxCheckBox* cb)
      {
        if (m_history == nullptr)
        {
          add_pane_history();
        }
        else
        {
          pane_toggle("HISTORY");
        }

        cb->SetValue(pane_is_shown("HISTORY"));
        get_toolbar()->Realize();
        update_statusbar(m_history);

        if (pane_is_shown("HISTORY"))
        {
          m_history->SetFocus();
        }
      }}}); // realize
}
