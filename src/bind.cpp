////////////////////////////////////////////////////////////////////////////////
// Name:      bind.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"
#include "defs.h"
#include "editors.h"
#include "find-files.h"
#include "frame.h"

void frame::bind()
{
  Bind(
    wxEVT_AUINOTEBOOK_BG_DCLICK,
    [=, this](const wxAuiNotebookEvent& event)
    {
      file_history().popup_menu(this, wex::ID_CLEAR_FILES);
    },
    ID_NOTEBOOK_EDITORS);

  Bind(
    wxEVT_AUINOTEBOOK_BG_DCLICK,
    [=, this](const wxAuiNotebookEvent& event)
    {
      get_project_history().popup_menu(this, wex::ID_CLEAR_PROJECTS);
    },
    ID_NOTEBOOK_PROJECTS);

  Bind(
    wxEVT_CLOSE_WINDOW,
    [=, this](wxCloseEvent& event)
    {
      int count = 0;
      for (size_t i = 0; i < m_editors->GetPageCount(); i++)
      {
        if (auto* stc = dynamic_cast<wex::stc*>(m_editors->GetPage(i));
            stc->path().file_exists())
        {
          count++;
        }
      }

      const bool project_open(m_projects->IsShown());

      if (event.CanVeto())
      {
        if (
          m_process->is_running() ||
          !m_editors->for_each<wex::stc>(wex::ID_ALL_CLOSE) ||
          !m_projects->for_each<wex::del::file>(wex::ID_ALL_CLOSE))
        {
          event.Veto();
          if (m_process->is_running())
          {
            wex::log::status(_("Process is running"));
          }
          return;
        }
      }

      wex::ex::get_macros().save_document();

      wex::config("recent.OpenFiles").set(count);
      wex::config("show.History")
        .set(m_history != nullptr && m_history->IsShown());
      wex::config("show.Projects").set(project_open);

      m_find_files->Destroy();

      event.Skip();
    });

  wex::bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        get_debug()->execute(event.GetId() - wex::ID_EDIT_DEBUG_FIRST);
      },
      wex::ID_EDIT_DEBUG_FIRST},
     {[=, this](wxCommandEvent& event)
      {
        m_editors->for_each<wex::stc>(event.GetId());
      },
      wex::ID_ALL_CLOSE},
     {[=, this](const wxCommandEvent& event)
      {
        shift_double_click();
      },
      ID_FIND_FILE}});

  wex::bind(this).ui(
    {{[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(m_editors->GetPageCount() > 0 && allow_browse_forward());
      },
      wxID_FORWARD},
     {[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(m_editors->GetPageCount() > 0 && allow_browse_backward());
      },
      wxID_BACKWARD}});

  Bind(
    wxEVT_SIZE,
    [=, this](wxSizeEvent& event)
    {
      event.Skip();
      if (IsMaximized())
      {
        m_maximized = true;
      }
      else if (m_maximized)
      {
        if (m_editors->is_split())
        {
          m_editors->rearrange(wxLEFT);
          m_editors->reset();
        }

        m_maximized = false;
      };
    });

  Bind(
    wxEVT_UPDATE_UI,
    [=, this](wxUpdateUIEvent& event)
    {
      event.Enable(m_app->is_debug());
    },
    wex::ID_EDIT_DEBUG_FIRST + 2,
    wex::ID_EDIT_DEBUG_LAST);
}
