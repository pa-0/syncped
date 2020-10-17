////////////////////////////////////////////////////////////////////////////////
// Name:      editors.cpp
// Purpose:   Implementation of editors class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>
#include "defs.h"
#include "editors.h"

editors::editors(const wex::data::window& data)
  : wex::notebook(data)
{
  Bind(
    wxEVT_MENU,
    [=](wxCommandEvent& event) {
      wxPostEvent(wxAuiNotebook::GetCurrentPage(), event);
    },
    wex::ID_EDIT_VCS_LOWEST,
    wex::ID_EDIT_VCS_HIGHEST);

  Bind(wxEVT_AUINOTEBOOK_END_DRAG, [=](wxAuiNotebookEvent& event) {
    event.Skip();
    m_split = true;
  });

  Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, [=](wxAuiNotebookEvent& event) {
    wex::menu menu(wex::menu::menu_t().set(wex::menu::IS_POPUP));

    auto* split = new wex::menu(
      {{ID_SPLIT_VERTICALLY, _("Split Vertically")},
       {ID_SPLIT_HORIZONTALLY, _("Split Horizontally")},
       {},
       {ID_SPLIT, _("Split")}});

    if (GetPageCount() > 1)
    {
      split->append(
        {{},
         {wxWindow::NewControlId(),
          _("Rearrange Vertically"),
          wex::data::menu().action([=](wxCommandEvent&) {
            rearrange(wxLEFT);
          })},
         {wxWindow::NewControlId(),
          _("Rearrange Horizontally"),
          wex::data::menu().action([=](wxCommandEvent&) {
            rearrange(wxTOP);
          })}});
    }

    menu.append(
      {{split, _("Split"), wxWindow::NewControlId()},
       {},
       {wxID_CLOSE},
       {wex::ID_ALL_CLOSE, _("Close A&ll")}});

    if (GetPageCount() > 2)
    {
      menu.append({{wex::ID_ALL_CLOSE_OTHERS, _("Close Others")}});
    }

    if (auto* stc = dynamic_cast<wex::stc*>(wxAuiNotebook::GetCurrentPage());
        stc->get_file().get_filename().file_exists() &&
        wex::vcs::dir_exists(stc->get_file().get_filename()))
    {
      menu.append({{}, {stc->get_file().get_filename()}});
    }

    PopupMenu(&menu);
  });
}
