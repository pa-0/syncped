////////////////////////////////////////////////////////////////////////////////
// Name:      decorated-frame.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "app.h"
#include "decorated-frame.h"
#include "defs.h"
#include "editors.h"

const long pane_flag = wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_CLOSE_ON_ALL_TABS |
                       wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_WINDOWLIST_BUTTON |
                       wxAUI_NB_SCROLL_BUTTONS;

decorated_frame::decorated_frame(app* app)
  : wex::del::frame(
      25, // maxFiles
      4,  // maxProjects
      wex::data::window().name("mainFrame").style(wxDEFAULT_FRAME_STYLE))
  , m_app(app)
  , m_dirctrl(new wex::del::dirctrl(this))
  , m_editors(new editors(
      this,
      wex::data::window().id(ID_NOTEBOOK_EDITORS).style(pane_flag)))
  , m_lists(new wex::notebook(
      wex::data::window().id(ID_NOTEBOOK_LISTS).style(pane_flag)))
  , m_process(new wex::process())
  , m_projects(new wex::notebook(
      wex::data::window().id(ID_NOTEBOOK_PROJECTS).style(pane_flag)))
{
  SetIcon(wxICON(app));

  wex::process::prepare_output(this);

  pane_add(
    {{m_editors,
      wxAuiPaneInfo().CenterPane().MaximizeButton(true).Name("FILES").Caption(
        _("Files"))},
     {m_dirctrl,
      wxAuiPaneInfo()
        .Left()
        .Hide()
        .MaximizeButton(true)
        .CloseButton(false)
        .Name("DIRCTRL")
        .MinSize(200, 150)
        .Caption(_("Explorer"))},
     {m_lists,
      wxAuiPaneInfo()
        .Bottom()
        .Hide()
        .MaximizeButton(true)
        .MinSize(250, 100)
        .Name("OUTPUT")
        .Row(0)
        .Caption(_("Output"))},
     {wex::process::get_shell(),
      wxAuiPaneInfo().Bottom().Hide().Name("PROCESS").MinSize(250, 100).Caption(
        _("Process"))},
     {m_projects,
      wxAuiPaneInfo()
        .Left()
        .Hide()
        .MaximizeButton(true)
        .Name("PROJECTS")
        .MinSize(150, 150)
        .Caption(_("Projects"))}});

  if (wex::config("show.History").get(false))
  {
    add_pane_history();
  }

#ifdef __WXMSW__
  const int lexer_size = 60;
#else
  const int lexer_size = 75;
#endif
  setup_statusbar(
    {{"PaneFileType", 50},
     {"PaneInfo", 100},
     {"PaneLexer", lexer_size},
     {"PaneTheme", lexer_size},
     {"PaneVCS", -2},
     {"PaneDBG", 50, false},
     {"PaneMacro", -1, false},
     {"PaneMode", 100, false}});

  if (wex::vcs vcs; !vcs.use() || wex::vcs::size() == 0)
  {
    m_statusbar->pane_show("PaneVCS", false);
  }

  if (wex::lexers::get()->get_lexers().empty())
  {
    m_statusbar->pane_show("PaneLexer", false);
    m_statusbar->pane_show("PaneTheme", false);
  }

  menu();
}

void decorated_frame::add_pane_history()
{
  m_history = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::HISTORY));

  pane_add(
    {{m_history,
      wxAuiPaneInfo()
        .Left()
        .MaximizeButton(true)
        .Name("HISTORY")
        .CloseButton(false)
        .MinSize(150, 150)
        .Caption(_("History"))}},
    std::string());
}

bool decorated_frame::allow_close(wxWindowID id, wxWindow* page)
{
  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      if (auto* stc = (wex::stc*)page;
          wex::file_dialog(&stc->get_file()).show_modal_if_changed() ==
          wxID_CANCEL)
      {
        return false;
      }
      else if (wex::beautify b; b.is_active() && stc->get_file().is_written())
      {
        stc->get_file().close();
        b.file(stc->path());
      }
      break;

    case ID_NOTEBOOK_PROJECTS:
      if (
        wex::file_dialog((wex::del::file*)page).show_modal_if_changed() ==
        wxID_CANCEL)
      {
        return false;
      }
      break;
  }

  return wex::del::frame::allow_close(id, page);
}

const std::string decorated_frame::allow_move_ext() const
{
  // Allow move if more than 2 open files have same extension.
  if (m_editors->GetPageCount() <= 2)
  {
    return std::string();
  }

  size_t      count = 0;
  std::string same_ext("/"); // invalid extension, so not used

  for (size_t i = 0; i < m_editors->GetPageCount(); ++i)
  {
    auto* stc  = (wex::stc*)m_editors->GetPage(i);
    auto& path = stc->path();

    if (path.extension() == same_ext)
    {
      count++;

      if (count > 2)
      {
        return same_ext;
      }
    }
    else
    {
      same_ext = path.extension();
      count    = 1;
    }
  }

  return std::string();
}

void decorated_frame::on_notebook(wxWindowID id, wxWindow* page)
{
  wex::del::frame::on_notebook(id, page);

  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      ((wex::stc*)page)->properties_message();
      break;

    case ID_NOTEBOOK_LISTS:
      break;

    case ID_NOTEBOOK_PROJECTS:
      wex::log::status() << ((wex::del::file*)page)->path();
      update_statusbar((wex::del::file*)page);
      break;

    default:
      assert(0);
  }
}
