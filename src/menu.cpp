////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/generic/textdlgg.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/valtext.h>

#include "app.h"
#include "decorated-frame.h"
#include "defs.h"
#include "editors.h"

void decorated_frame::menu()
{
  auto* menuFind = new wex::menu();

  if (wex::config(_("stc.vi mode")).get(true))
  {
    // No accelerators for vi mode, Ctrl F is page down.
    menuFind->append(
      {{wxID_FIND, wxGetStockLabel(wxID_FIND, wxSTOCK_NOFLAGS)}});

    if (!m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
    {
      menuFind->append(
        {{wxID_REPLACE, wxGetStockLabel(wxID_REPLACE, wxSTOCK_NOFLAGS)}});
    }
  }
  else
  {
    menuFind->append({{wxID_FIND}});

    if (!m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
    {
      menuFind->append({{wxID_REPLACE}});
    }
  }

  menuFind->append(
    {{},
     {ID_FIND_FILE, _("Find Files\tCtrl+L")},
     {wex::ID_TOOL_REPORT_FIND, wex::ellipsed(_("Find &in Files"))}});

  if (!m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
  {
    menuFind->append(
      {{wex::ID_TOOL_REPLACE, wex::ellipsed(_("Replace in File&s"))}});
  }

  auto* menuMacro = new wex::menu(
    {{ID_EDIT_MACRO_START_RECORD, wex::ellipsed(_("Start Record"))},
     {ID_EDIT_MACRO_STOP_RECORD, _("Stop Record")},
     {},
     {ID_EDIT_MACRO_PLAYBACK, wex::ellipsed(_("Playback"))}});

  if (wex::ex::get_macros().path().file_exists())
  {
    menuMacro->append(
      {{},
       {ID_EDIT_MACRO,
        wxGetStockLabel(wxID_EDIT),
        wex::data::menu().action(
          [=, this](wxCommandEvent&)
          {
            open_file(wex::ex::get_macros().path(), wex::data::stc());
          })}});
  }

  wex::menu* menuDebug = nullptr;

  if (m_app->is_debug())
  {
    menuDebug = new wex::menu();

    if (get_debug()->add_menu(menuDebug) == 0)
    {
      delete menuDebug;
      menuDebug = nullptr;
      wex::log("no debug menu");
    }
    else
    {
      statustext(get_debug()->debug_entry().name(), "PaneDBG");
    }

    m_statusbar->pane_show("PaneDBG", true);
  }

  auto* menuOptions = new wex::menu();

  if (wex::vcs::size() > 0)
  {
    menuOptions->append(
      {{NewControlId(),
        wex::ellipsed(_("Set &VCS")),
        wex::data::menu().action(
          [=, this](wxCommandEvent&)
          {
            if (wex::vcs().config_dialog() == wxID_OK)
            {
              wex::vcs vcs;
              vcs.set_entry_from_base(this);
              m_statusbar->pane_show("PaneVCS", vcs.use());
              statustext(vcs.name(), "PaneVCS");
            }
          })},
       {}});
  }

  menuOptions->append({
#ifndef __WXOSX__
    {wxID_PREFERENCES, wex::ellipsed(_("Set &Editor Options"))},
#else
    {wxID_PREFERENCES},
#endif
    {ID_OPTION_LIST,
     wex::ellipsed(_("Set &List Options")),
     wex::data::menu().action(
       [=](wxCommandEvent& event)
       {
         wex::listview::config_dialog(wex::data::window()
                                        .button(wxOK | wxCANCEL | wxAPPLY)
                                        .id(ID_OPTION_LIST));
       })},
    {ID_OPTION_TAB,
     wex::ellipsed(_("Set &Tab Options")),
     wex::data::menu().action(
       [=](wxCommandEvent& event)
       {
         wex::notebook::config_dialog(wex::data::window()
                                        .button(wxOK | wxCANCEL | wxAPPLY)
                                        .id(ID_OPTION_TAB));
       })}});

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_NEW,
          wex::ellipsed(wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS), "\tCtrl+N"),
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              // In hex mode we cannot edit the file.
              if (wex::config("is_hexmode").get(false))
                return;

              static std::string name;

              if (event.GetString().empty())
              {
                wxTextEntryDialog dlg(
                  this,
                  _("Input") + ":",
                  _("File Name"),
                  name);
                wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
                validator.SetCharExcludes("/\\?%*:|\"<>");
                dlg.SetTextValidator(validator);

                if (dlg.ShowModal() == wxID_CANCEL)
                  return;

                name = dlg.GetValue();
              }
              else
              {
                name = event.GetString();
              }

              if (!name.empty())
              {
                auto* page = new wex::stc(
                  wex::path(name),
                  wex::data::stc(m_app->data())
                    .window(wex::data::window().parent(m_editors)));
                // This file does yet exist, so do not give it a bitmap.
                m_editors->add_page(
                  wex::data::notebook().page(page).key(name).select());
                pane_show("FILES");
                page->config_get();
              };
            })},

         {wxID_OPEN,
          std::string(),
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              open_from_event(event, allow_move_ext());
            })},

         {},

         {NewControlId(),
          file_history(),
          wex::data::menu().ui(
            [=, this](wxUpdateUIEvent& event)
            {
              event.Enable(!file_history().empty());
            })},

         {wxID_CLOSE,
          wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS) + "\tCtrl+W",
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                if (auto* stc = get_stc(); stc != nullptr)
                {
                  if (!allow_close(m_editors->GetId(), stc))
                    return;
                  sync(false);
                  stc->sync(false);
                  m_editors->delete_page(m_editors->key_by_page(stc));
                  sync(true);
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Enable(
                  m_editors->IsShown() && m_editors->GetPageCount() > 0);
              })},

         {},

         {wxID_SAVE},

         {wxID_SAVEAS,
          "",
          wex::data::menu().ui(
            [=, this](wxUpdateUIEvent& event)
            {
              event.Enable(
                m_editors->IsShown() && m_editors->GetPageCount() > 0);
            })},

         {wex::ID_ALL_SAVE,
          _("Save A&ll"),
          wex::data::menu().art(wxART_FILE_SAVE)},

         {},

         {wex::menu_item::PRINT},

         {},

         {wex::menu_item::EXIT}}),

      wxGetStockLabel(wxID_FILE)},

     {new wex::menu(
        {{wxID_UNDO},
         {wxID_REDO},
         {},
         {wxID_CUT},
         {wxID_COPY},
         {wxID_PASTE},
         {},
         {wxID_JUMP_TO},
         {wxID_CLEAR},
         {wxID_SELECTALL},
         {},
         {menuFind,
          !m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY) ?
            _("&Find and Replace") :
            _("&Find")},
         {},
         {wex::ID_EDIT_CONTROL_CHAR,
          wex::ellipsed(_("&Control Char"), "Ctrl+K"),
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              if (get_stc() != nullptr)
              {
                wxPostEvent(get_stc(), event);
              }
            })},
         {},
         {menuMacro, _("&Macro"), ID_EDIT_MACRO_MENU}}),
      wxGetStockLabel(wxID_EDIT)},

     {new wex::menu(
        {{this},
         {},
         {NewControlId(),
          _("&Files"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                pane_toggle("FILES");
                if (!pane_is_shown("FILES") && pane_is_shown("PROJECTS"))
                {
                  pane_maximize("PROJECTS");
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(pane_is_shown("FILES"));
              })},

         {NewControlId(),
          _("&Projects"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                pane_toggle("PROJECTS");
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(pane_is_shown("PROJECTS"));
              })},

         {ID_VIEW_DIRCTRL,
          _("&Explorer"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                pane_toggle("DIRCTRL");
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(pane_is_shown("DIRCTRL"));
              })},

         {ID_VIEW_HISTORY,
          _("&History"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                if (m_history == nullptr)
                {
                  add_pane_history();
                }
                else
                {
                  pane_toggle("HISTORY");
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(m_history != nullptr && pane_is_shown("HISTORY"));
              })},

         {NewControlId(),
          _("&Output"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                pane_toggle("OUTPUT");
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(pane_is_shown("OUTPUT"));
              })},

         {},

         {NewControlId(),
          _("&Ascii Table"),
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              build_ascii_table();
            })}}),
      _("&View")},

     {new wex::menu(
        {{NewControlId(),
          wex::ellipsed(_("&Select")),
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              if (wex::process::config_dialog() == wxID_OK)
              {
                pane_show("PROCESS");
                delete m_process;
                m_process = new wex::process;
                m_process->async_system(wex::process_data());
              };
            })},

         {},

         {wxID_EXECUTE,
          "",
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              pane_show("PROCESS");
              delete m_process;
              m_process = new wex::process;
              m_process->async_system(wex::process_data());
            })},

         {wxID_STOP,
          "",
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              m_process->stop();
              pane_show("PROCESS", false);
            })}}),
      _("&Process")},

     {new wex::menu(
        {{NewControlId(),
          wxGetStockLabel(wxID_NEW),
          wex::data::menu().art(wxART_NEW).action(
            [=, this](wxCommandEvent& event)
            {
              const std::string text =
                wxString::Format("%s%d", _("project"), m_project_id++)
                  .ToStdString();
              const wex::path fn(
                (!get_project_history()[0].empty() ?
                   wex::path(get_project_history()[0].parent_path()) :
                   wex::config::dir()),
                text + ".prj");
              wxWindow* page =
                new wex::del::file(fn, wex::data::window().parent(m_projects));
              ((wex::del::file*)page)->file_new(fn);
              // This file does yet exist, so do not give it a bitmap.
              m_projects->add_page(wex::data::notebook()
                                     .page(page)
                                     .key(fn.string())
                                     .caption(text)
                                     .select());
              set_recent_project(fn);
              pane_show("PROJECTS");
            })},

         {NewControlId(),
          wxGetStockLabel(wxID_OPEN),
          wex::data::menu()
            .art(wxART_FILE_OPEN)
            .action(
              [=, this](wxCommandEvent& event)
              {
                wxFileDialog dlg(
                  this,
                  _("Select Projects"),
                  (!get_project_history()[0].empty() ?
                     get_project_history()[0].parent_path() :
                     wex::config::dir().string()),
                  wxEmptyString,
                  m_project_wildcard,
// osx asserts on GetPath with wxFD_MULTIPLE flag,
// and the to_vector_path does not do anything
#ifdef __WXOSX__
                  wxFD_OPEN);
#else
                  wxFD_OPEN | wxFD_MULTIPLE);
#endif
                if (dlg.ShowModal() == wxID_CANCEL)
                  return;
                const std::vector<wex::path> v(
#ifdef __WXOSX__
                  {wex::path(dlg.GetPath().ToStdString())});
#else
                  wex::to_vector_path(dlg).get());
#endif
                wex::open_files(
                  this,
                  v,
                  wex::data::stc().flags(wex::data::stc::window_t().set(
                    wex::data::stc::WIN_IS_PROJECT)));
              })},

         {NewControlId(),
          _("&Open as Text"),
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                if (auto* project = get_project(); project != nullptr)
                {
                  if (
                    wex::file_dialog(project).show_modal_if_changed() !=
                    wxID_CANCEL)
                  {
                    wex::open_files(this, {project->path()});
                  }
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Enable(
                  get_project() != nullptr && !get_project()->path().empty());
              })},

         {},

         {NewControlId(),
          get_project_history(),
          wex::data::menu().ui(
            [=, this](wxUpdateUIEvent& event)
            {
              event.Enable(!get_project_history()[0].empty());
            })},

         {NewControlId(),
          wxGetStockLabel(wxID_CLOSE),
          wex::data::menu()
            .art(wxART_CLOSE)
            .action(
              [=, this](wxCommandEvent& event)
              {
                if (auto* project = get_project(); project != nullptr)
                {
                  m_projects->delete_page(project->path().string());
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Enable(
                  get_project() != nullptr && get_project()->IsShown());
              })},

         {wex::del::ID_PROJECT_SAVE,
          wxGetStockLabel(wxID_SAVE),
          wex::data::menu().art(wxART_FILE_SAVE)},

         {NewControlId(),
          wxGetStockLabel(wxID_SAVEAS),
          wex::data::menu()
            .art(wxART_FILE_SAVE_AS)
            .action(
              [=, this](wxCommandEvent& event)
              {
                if (auto* project = get_project(); project != nullptr)
                {
                  wex::file_dialog dlg(
                    project,
                    wex::data::window()
                      .style(wxFD_SAVE)
                      .parent(this)
                      .title(_("Project Save As").ToStdString())
                      .wildcard(m_project_wildcard));
                  if (dlg.ShowModal() == wxID_OK)
                  {
                    project->file_save(wex::path(dlg.GetPath().ToStdString()));
                    m_projects->set_page_text(
                      m_projects->key_by_page(project),
                      project->path().string(),
                      project->path().name());
                  }
                };
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Enable(
                  get_project() != nullptr && get_project()->IsShown());
              })},

         {},

         {NewControlId(),
          _("&Auto Sort"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action(
              [=, this](wxCommandEvent& event)
              {
                wex::config("list.SortSync")
                  .set(!wex::config("list.SortSync").get(true));
              })
            .ui(
              [=, this](wxUpdateUIEvent& event)
              {
                event.Check(wex::config("list.SortSync").get(true));
              })}}),
      _("&Project")},

     {menuDebug, _("&Debug")},

     {menuOptions, _("&Options")},

     {new wex::menu(
        {{wxID_ABOUT,
          "",
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              wex::version_info_dialog(
                m_app->version(),
                wex::about_info().website(
                  "http://sourceforge.net/projects/syncped/"))
                .show();
            })},

         {wxID_HELP,
          "",
          wex::data::menu().action(
            [=, this](wxCommandEvent& event)
            {
              wxLaunchDefaultBrowser(
                "http://antonvw.github.io/syncped/v" +
                wex::rfind_before(m_app->version().get(), ".") +
                "/syncped.htm");
            })}}),
      wxGetStockLabel(wxID_HELP)}}));
}
