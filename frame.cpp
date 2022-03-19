////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <thread>
#include <wx/imaglist.h>
#include <wx/stockitem.h>

#include "app.h"
#include "defs.h"
#include "editors.h"
#include "find-files.h"
#include "frame.h"

BEGIN_EVENT_TABLE(frame, decorated_frame)
EVT_MENU(wxID_DELETE, frame::on_command)
EVT_MENU(wxID_JUMP_TO, frame::on_command)
EVT_MENU(wxID_SELECTALL, frame::on_command)
EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, frame::on_command)
EVT_MENU_RANGE(wxID_SAVE, wxID_CLOSE_ALL, frame::on_command)
EVT_MENU_RANGE(
  ID_EDIT_MACRO_PLAYBACK,
  ID_EDIT_MACRO_STOP_RECORD,
  frame::on_command)
EVT_MENU_RANGE(ID_SPLIT, ID_SPLIT_VERTICALLY, frame::on_command)
EVT_UPDATE_UI(wex::ID_ALL_CLOSE, frame::on_update_ui)
EVT_UPDATE_UI(wex::ID_ALL_SAVE, frame::on_update_ui)
EVT_UPDATE_UI(wxID_EXECUTE, frame::on_update_ui)
EVT_UPDATE_UI(wxID_JUMP_TO, frame::on_update_ui)
EVT_UPDATE_UI(wxID_PRINT, frame::on_update_ui)
EVT_UPDATE_UI(wxID_PREVIEW, frame::on_update_ui)
EVT_UPDATE_UI(wxID_REPLACE, frame::on_update_ui)
EVT_UPDATE_UI(wxID_UNDO, frame::on_update_ui)
EVT_UPDATE_UI(wxID_REDO, frame::on_update_ui)
EVT_UPDATE_UI(wxID_SAVE, frame::on_update_ui)
EVT_UPDATE_UI(wxID_STOP, frame::on_update_ui)
EVT_UPDATE_UI(wex::ID_EDIT_CONTROL_CHAR, frame::on_update_ui)
EVT_UPDATE_UI(ID_EDIT_MACRO, frame::on_update_ui)
EVT_UPDATE_UI(ID_EDIT_MACRO_MENU, frame::on_update_ui)
EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, frame::on_update_ui)
EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, frame::on_update_ui)
EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, frame::on_update_ui)
EVT_UPDATE_UI(wex::del::ID_PROJECT_SAVE, frame::on_update_ui)
// Some wxID's are shared between stc and listview, so
// enable / disable is more complex, not yet done
// for the range wxID_CUT wxID_SELECTALL
EVT_UPDATE_UI_RANGE(
  wex::ID_EDIT_FIND_NEXT,
  wex::ID_EDIT_FIND_PREVIOUS,
  frame::on_update_ui)
END_EVENT_TABLE()

frame::frame(app* app)
  : decorated_frame(app)
  , m_find_files(new find_files(this))
{
  if (!m_app->get_tag().empty())
  {
    wex::ctags::find(m_app->get_tag());
  }
  else if (m_app->get_files().empty())
  {
    bool project_opened = false;

    if (
      wex::config("show.Projects").get(false) &&
      !get_project_history()[0].empty())
    {
      open_file(
        get_project_history()[0],
        wex::data::stc().flags(
          wex::data::stc::window_t().set(wex::data::stc::WIN_IS_PROJECT)));
      project_opened = true;
    }

    if (const int count = wex::config("recent.OpenFiles").get(0); count > 0)
    {
      wex::open_files(
        this,
        file_history().get_history_files(count),
        m_app->data());
    }
    else
    {
      if (project_opened)
      {
        pane_maximize("PROJECTS");
      }
      else
      {
        auto* page = new wex::stc(
          std::string(),
          wex::data::stc(m_app->data())
            .window(wex::data::window().parent(m_editors)));

        page->get_file().file_new(wex::path("no name"));

        m_editors->add_page(
          wex::data::notebook().page(page).key("no name").select());

        pane_show("FILES");
      }
    }
  }
  else
  {
    if (m_app->is_debug())
    {
      m_files = m_app->get_files();
      const auto& p(m_files.back());

      if (const auto l(wex::lexers::get()->find_by_filename(p.filename()));
          !l.is_ok())
      {
        get_debug()->execute("file " + p.string());
        m_files.pop_back();
      }
      else
      {
        wex::open_files(
          this,
          m_app->get_files(),
          m_app->data(),
          wex::data::dir::type_t().set(wex::data::dir::FILES));
      }
    }
    else if (m_app->is_project())
    {
      wex::open_files(
        this,
        m_app->get_files(),
        wex::data::stc().flags(
          wex::data::stc::window_t().set(wex::data::stc::WIN_IS_PROJECT)));
    }
    else
    {
      wex::open_files(
        this,
        m_app->get_files(),
        m_app->data(),
        wex::data::dir::type_t().set(wex::data::dir::FILES));
    }
  }

  statustext(wex::lexers::get()->theme(), "PaneTheme");

  if (
    m_editors->GetPageCount() > 0 &&
    !m_app->data().flags().test(wex::data::stc::WIN_EX))
  {
    m_editors->GetPage(m_editors->GetPageCount() - 1)->SetFocus();
  }

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
      }}}); // realize

  get_find_toolbar()->add_find();

  get_options_toolbar()->add_checkboxes_standard();

  Bind(
    wxEVT_AUINOTEBOOK_BG_DCLICK,
    [=, this](wxAuiNotebookEvent& event)
    {
      file_history().popup_menu(this, wex::ID_CLEAR_FILES);
    },
    ID_NOTEBOOK_EDITORS);

  Bind(
    wxEVT_AUINOTEBOOK_BG_DCLICK,
    [=, this](wxAuiNotebookEvent& event)
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

      if (m_app->data().control().command().empty())
      {
        delete m_process;
      }
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
     {[=, this](wxCommandEvent& event)
      {
        shift_double_click();
      },
      ID_FIND_FILE}});

  wex::bind(this).ui(
    {{[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(
          !file_history().empty() && m_editors->GetPageCount() > 0 &&
          m_browse_index < file_history().size() - 1);
      },
      wxID_FORWARD},
     {[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(
          !file_history().empty() && m_editors->GetPageCount() > 0 &&
          m_browse_index > 0);
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

  if (m_files.empty())
  {
    m_app->reset();
  }

  if (m_app->is_stdin())
  {
    std::thread v(
      [&]
      {
        std::string text;

        while (!std::cin.fail())
        {
          const int c(std::cin.get());

          if (text.empty() || text.back() != WXK_ESCAPE)
          {
            text.push_back(c);
          }

          if (c == '\n')
          {
            if (auto* stc(((wex::stc*)m_editors->GetCurrentPage()));
                stc != nullptr)
            {
              wxCommandEvent event(wxEVT_MENU, wex::id::stc::vi_command);
              event.SetString(
                text == "\n" && !stc->get_vi().mode().is_insert() ? "j" : text);
              wxPostEvent(stc, event);
            }

            text.clear();
          }
        }
      });

    v.detach();
  }
}

wex::del::listview*
frame::activate(wex::data::listview::type_t type, const wex::lexer* lexer)
{
  if (type == wex::data::listview::FILE)
  {
    return get_project();
  }
  else
  {
    pane_show("OUTPUT");

    const std::string name =
      wex::data::listview().type(type).type_description() +
      (lexer != nullptr ? " " + lexer->display_lexer() : std::string());
    auto* list = (wex::del::listview*)m_lists->page_by_key(name);

    if (list == nullptr && type != wex::data::listview::FILE)
    {
      list = new wex::del::listview(
        wex::data::listview(wex::data::window().parent(m_lists))
          .type(type)
          .lexer(lexer));

      m_lists->add_page(wex::data::notebook().page(list).key(name).select());
    }

    return list;
  }
}

void frame::debug_exe(const wex::path& p)
{
  if (!m_files.empty())
  {
    wex::open_files(
      this,
      m_files,
      m_app->data(),
      wex::data::dir::type_t().set(wex::data::dir::FILES));

    m_files.clear();
    m_app->reset();
  }
}

bool frame::exec_ex_command(wex::ex_command& command)
{
  if (command.command() == ":")
    return false;

  bool handled = false;

  try
  {
    if (m_editors->GetPageCount() > 0)
    {
      if (boost::algorithm::trim_copy(command.command()) == ":n")
      {
        if (m_editors->GetSelection() == m_editors->GetPageCount() - 1)
          return false;

        m_editors->AdvanceSelection();
        handled = true;
      }
      else if (boost::algorithm::trim_copy(command.command()) == ":prev")
      {
        if (m_editors->GetSelection() == 0)
          return false;

        m_editors->AdvanceSelection(false);
        handled = true;
      }

      if (handled && wex::ex::get_macros().mode().is_playback())
      {
        command.set(((wex::stc*)m_editors->GetPage(m_editors->GetSelection()))
                      ->get_vi()
                      .get_command());
      }
    }
  }
  catch (std::exception& e)
  {
    wex::log(e) << command.command();
  }

  return handled;
}

wex::process* frame::get_process(const std::string& command)
{
  if (!m_app->is_debug())
    return nullptr;

  delete m_process;
  m_process = new wex::process;
  m_process->async_system(wex::process_data(command));

  return m_process;
}

wex::del::file* frame::get_project()
{
  if (!m_projects->IsShown() || m_projects->GetPageCount() == 0)
  {
    return nullptr;
  }
  else
  {
    return (wex::del::file*)m_projects->GetPage(m_projects->GetSelection());
  }
}

bool frame::is_open(const wex::path& filename)
{
  return m_editors->page_index_by_key(filename.string()) != wxNOT_FOUND;
}

void frame::on_command(wxCommandEvent& event)
{
  switch (auto* editor = dynamic_cast<wex::stc*>(get_stc()); event.GetId())
  {
    // edit commands
    // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
    // as wxID_ABOUT etc. is used here and not in the editor.
    // That causes appl to hang.
    case wxID_UNDO:
    case wxID_REDO:
    case wxID_CLEAR:
    case wxID_COPY:
    case wxID_CUT:
    case wxID_DELETE:
    case wxID_JUMP_TO:
    case wxID_PASTE:
    case wxID_SELECTALL:
      if (editor != nullptr)
      {
        wxPostEvent(editor, event);
      }
      else if (get_listview() != nullptr)
      {
        wxPostEvent(get_listview(), event);
      }
      break;

    case wxID_SAVE:
      if (editor != nullptr)
      {
        save(editor);
      }
      break;

    case wxID_SAVEAS:
      if (editor != nullptr)
      {
        saveas(editor, event.GetString());
      }
      break;

    case ID_EDIT_MACRO_PLAYBACK:
      if (editor != nullptr)
        editor->get_vi().get_macros().mode().transition(
          "@",
          &editor->get_vi(),
          true);
      break;

    case ID_EDIT_MACRO_START_RECORD:
    case ID_EDIT_MACRO_STOP_RECORD:
      if (editor != nullptr)
        editor->get_vi().get_macros().mode().transition(
          "q",
          &editor->get_vi(),
          true);
      break;

    case ID_SPLIT:
    case ID_SPLIT_HORIZONTALLY:
    case ID_SPLIT_VERTICALLY:
      if (editor == nullptr)
      {
        wex::log::status("No valid focus");
      }
      else
      {
        auto* stc = new wex::stc(
          editor->path(),
          wex::data::stc().window(wex::data::window().parent(m_editors)));
        editor->sync(false);
        stc->sync(false);
        stc->get_vi().copy(&editor->get_vi());

        wex::data::notebook nd;

        if (editor->path().file_exists())
        {
          nd.bitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
            wex::get_iconid(editor->path())));
        }
        else if (!editor->get_lexer().scintilla_lexer().empty())
        {
          stc->get_lexer().set(editor->get_lexer().scintilla_lexer());
        }

        // key should be unique
        nd.key("split" + std::to_string(m_split_id++));

        // Place new page before page for editor.
        m_editors->insert_page(nd.index(m_editors->GetPageIndex(editor))
                                 .page(stc)
                                 .caption(editor->path().filename())
                                 .select());

        stc->SetDocPointer(editor->GetDocPointer());

        if (event.GetId() == ID_SPLIT_HORIZONTALLY)
        {
          m_editors->split(nd.key(), wxBOTTOM);
          m_editors->set_selection(editor->path().string());
        }
        else if (event.GetId() == ID_SPLIT_VERTICALLY)
        {
          m_editors->split(nd.key(), wxRIGHT);
          m_editors->set_selection(editor->path().string());
        }
      }
      break;

    default:
      assert(0);
      break;
  }
}

void frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  switch (dialogid)
  {
    case wxID_PREFERENCES:
      if (event.GetId() != wxID_CANCEL)
      {
        m_editors->for_each<wex::stc>(wex::ID_ALL_CONFIG_GET);

        wex::process::get_shell()->config_get();

        m_statusbar->pane_show(
          "PaneMacro",
          wex::config(_("stc.vi mode")).get(true));
      }
      break;

    case ID_OPTION_LIST:
      if (event.GetId() != wxID_CANCEL)
      {
        update_listviews();
      }
      break;

    case ID_OPTION_TAB:
      if (event.GetId() != wxID_CANCEL)
      {
        m_editors->config_get();
        m_lists->config_get();
        m_projects->config_get();

        if (m_history != nullptr)
        {
          m_history->config_get();
        }
      }
      break;

    default:
      decorated_frame::on_command_item_dialog(dialogid, event);
  }
}

void frame::on_update_ui(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case wxID_EXECUTE:
      event.Enable(
        !is_closing() && !m_process->data().exe().empty() &&
        !m_process->is_running());
      break;

    case wxID_STOP:
      event.Enable(m_process->is_running());
      break;

    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (get_stc() != nullptr && get_stc()->GetLength() > 0) ||
        (get_listview() != nullptr && get_listview()->GetItemCount() > 0));
      break;

    case wex::ID_ALL_CLOSE:
    case wex::ID_ALL_SAVE:
      event.Enable(m_editors->GetPageCount() > 2);
      break;

    case wex::del::ID_PROJECT_SAVE:
      event.Enable(
        get_project() != nullptr && get_project()->is_contents_changed());
      break;

    default:
    {
      if (auto* editor = dynamic_cast<wex::stc*>(get_stc()); editor != nullptr)
      {
        switch (event.Enable(true); event.GetId())
        {
          case wxID_FIND:
          case wxID_JUMP_TO:
          case wxID_REPLACE:
          case wxID_CUT:
          case wxID_COPY:
          case wxID_PASTE:
          case wxID_CLEAR:
          case wxID_DUPLICATE:
          case wxID_SELECTALL:
          case wex::ID_EDIT_FIND_NEXT:
          case wex::ID_EDIT_FIND_PREVIOUS:
            event.Enable(editor->GetLength() > 0);
            break;

          case ID_EDIT_MACRO:
            event.Enable(
              editor->get_vi().is_active() &&
              !editor->get_vi().get_macros().mode().is_recording() &&
              wex::ex::get_macros().path().file_exists());
            break;

          case ID_EDIT_MACRO_MENU:
            event.Enable(editor->get_vi().is_active());
            break;

          case ID_EDIT_MACRO_PLAYBACK:
            event.Enable(
              editor->get_vi().is_active() &&
              editor->get_vi().get_macros().size() > 0 &&
              !editor->get_vi().get_macros().mode().is_recording());
            break;

          case ID_EDIT_MACRO_START_RECORD:
            event.Enable(
              editor->get_vi().is_active() &&
              !editor->get_vi().get_macros().mode().is_recording());
            break;

          case ID_EDIT_MACRO_STOP_RECORD:
            event.Enable(editor->get_vi().get_macros().mode().is_recording());
            break;

          case wxID_SAVE:
            event.Enable(!editor->path().empty() && editor->GetModify());
            break;

          case wxID_REDO:
            event.Enable(editor->CanRedo());
            break;

          case wxID_UNDO:
            event.Enable(editor->CanUndo());
            break;

          case wex::ID_EDIT_CONTROL_CHAR:
            if (
              editor->GetReadOnly() && editor->GetSelectedText().length() != 1)
            {
              event.Enable(false);
            }
            break;

          default:
            assert(0);
        }
      }
      else if (auto* list = (wex::del::file*)get_listview();
               list != nullptr && list->IsShown())
      {
        event.Enable(false);

        if (
          event.GetId() > wex::ID_TOOL_LOWEST &&
          event.GetId() < wex::ID_TOOL_HIGHEST)
        {
          event.Enable(list->GetSelectedItemCount() > 0);
        }
        else if (event.GetId() == wxID_FIND)
        {
          event.Enable(list->GetItemCount() > 0);
        }
      }
      else
      {
        event.Enable(false);
      }
    }
  }
}

wex::stc* frame::open_file(
  const wex::path&      filename,
  wex::vcs_entry&       vcs,
  const wex::data::stc& data)
{
  if (vcs.get_command().is_blame())
  {
    if (auto* page = (wex::stc*)m_editors->set_selection(filename.string());
        page != nullptr)
    {
      if (page->show_blame(&vcs))
        return page;
    }
  }

  const auto unique = vcs.get_command().get_command() + " " + vcs.get_flags();

  wex::data::notebook nd;
  nd.select()
    .key(filename.string() + " " + unique)
    .page((wex::stc*)m_editors->set_selection(nd.key()));

  if (nd.page() == nullptr)
  {
    nd.page(new wex::stc(
      vcs.std_out(),
      wex::data::stc(data).window(wex::data::window().parent(m_editors).name(
        filename.filename() + " " + unique))));

    wex::vcs_command_stc(
      vcs.get_command(),
      wex::path_lexer(filename).lexer(),
      (wex::stc*)nd.page());

    if (const int index = m_editors->page_index_by_key(filename.string());
        index != -1)
    {
      // Place new page before the one used for vcs.
      m_editors->insert_page(nd.index(index));
    }
    else
    {
      // Just add at the end.
      m_editors->add_page(nd);
    }
  }

  return (wex::stc*)nd.page();
}

wex::stc* frame::open_file(
  const wex::path&       filename,
  wex::factory::process& p,
  const wex::data::stc&  data)
{
  auto* page = new wex::stc(
    std::string(),
    wex::data::stc(data).window(
      wex::data::window().parent(m_editors).name(filename.string())));

  page->get_lexer().set(wex::path_lexer(filename).lexer());

  m_editors->add_page(wex::data::notebook()
                        .page(page)
                        .key(p.data().exe())
                        .caption(p.data().exe())
                        .select());

  page->show_blame(&wex::vcs().entry(), p.std_out());
  page->EmptyUndoBuffer();
  page->SetSavePoint();
  page->inject(data.control());

  return page;
}

wex::stc* frame::open_file(
  const wex::path&      filename,
  const std::string&    text,
  const wex::data::stc& data)
{
  auto* page = (wex::stc*)m_editors->set_selection(filename.string());

  if (page == nullptr)
  {
    page = new wex::stc(
      text,
      wex::data::stc(data).window(
        wex::data::window().parent(m_editors).name(filename.string())));

    page->get_lexer().set(wex::path_lexer(filename).lexer());

    m_editors->add_page(wex::data::notebook()
                          .page(page)
                          .key(filename.string())
                          .caption(filename.filename())
                          .select());
  }
  else
  {
    page->SetText(text);
  }

  return page;
}

wex::stc*
frame::open_file(const wex::path& filename, const wex::data::stc& data)
{
  auto* notebook =
    (data.flags().test(wex::data::stc::WIN_IS_PROJECT) ? m_projects :
                                                         m_editors);

  assert(notebook != nullptr);

  auto* page = notebook->set_selection(filename.string());

  if (data.flags().test(wex::data::stc::WIN_IS_PROJECT))
  {
    if (!pane_is_shown("PROJECTS"))
    {
      pane_show("PROJECTS");
    }

    if (page == nullptr)
    {
      auto* project =
        new wex::del::file(filename, wex::data::window().parent(m_projects));

      notebook->add_page(
        wex::data::notebook()
          .page(project)
          .key(filename.string())
          .caption(filename.name())
          .select()
          .bitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
            wex::get_iconid(filename))));
    }
  }
  else
  {
    if (!pane_is_shown("FILES"))
    {
      if (pane_is_maximized("PROJECTS"))
      {
        pane_restore("PROJECTS");
      }

      pane_show("FILES");
    }

    if (filename == wex::ex::get_macros().path())
    {
      wex::ex::get_macros().save_document();
    }

    auto* editor = (wex::stc*)page;

    if (page == nullptr)
    {
      wex::data::stc::menu_t   mf(m_app->data().menu());
      wex::data::stc::window_t wf(m_app->data().flags());
      wex::data::control       cd(data.control());

      if (!m_app->data().control().command().empty())
      {
        cd.command(m_app->data().control().command());
      }

      if (wex::config("is_hexmode").get(false))
        wf.set(wex::data::stc::WIN_HEX);
      if (m_app->is_debug())
        mf.set(wex::data::stc::MENU_DEBUG);

      editor = new wex::stc(
        filename,
        wex::data::stc(data)
          .control(cd)
          .window(wex::data::window().parent(m_editors))
          .flags(wf, wex::data::control::OR)
          .menu(mf));

      if (m_app->is_debug())
      {
        get_debug()->apply_breakpoints(editor);
      }

      wex::data::notebook nd;
      nd.key(filename.stat().path());

      notebook->add_page(
        nd.page(editor)
          .caption(filename.filename())
          .select()
          .bitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
            wex::get_iconid(filename))));

      if (notebook->GetPageCount() >= 2 && m_app->get_split() != -1)
      {
        notebook->split(nd.key(), m_app->get_split());
      }

      if (pane_is_shown("DIRCTRL"))
      {
        m_dirctrl->expand_and_select_path(wex::path(nd.key()));
      }

      // Do not show an edge for project files opened as text.
      if (filename.extension() == ".prj")
      {
        editor->SetEdgeMode(wxSTC_EDGE_NONE);
      }

      if (wex::config(_("stc.Auto blame")).get(false))
      {
        if (wex::vcs vcs{{filename}}; vcs.execute("blame " + filename.string()))
        {
          editor->show_blame(&vcs.entry());
        }
      }
    }
    else
    {
      wex::data::stc(editor, data).inject();
    }

    if (editor->is_visual())
    {
      editor->SetFocus();
    }

    return editor;
  }

  return (wex::stc*)page;
}

void frame::open_file_same_page(wxCommandEvent& event)
{
  if (auto* page = (wex::stc*)m_editors->GetPage(m_editors->GetSelection());
      page != nullptr && file_history().size() > 1)
  {
    if (event.GetId() == wxID_FORWARD)
    {
      if (m_browse_index < file_history().size() - 1)
      {
        m_browse_index++;
      }
      else if (m_browse_index > file_history().size() - 1)
      {
        m_browse_index = file_history().size() - 1;
        return;
      }
      else
      {
        return;
      }
    }
    else
    {
      if (m_browse_index > 0)
      {
        m_browse_index--;
      }
      else
      {
        return;
      }
    }

    const auto& p(file_history()[m_browse_index]);

    m_editors->set_page_text(
      m_editors->key_by_page(page),
      p.string(),
      p.filename());
    page->open(p, wex::data::stc().recent(false));
    page->get_lexer().set(wex::path_lexer(p).lexer().display_lexer(), true);
    page->properties_message();
  }
}

bool frame::print_ex(wex::factory::stc* stc, const std::string& text)
{
  auto* page = (wex::stc*)m_editors->set_selection("Print");

  if (page == nullptr)
  {
    page = new wex::stc(
      text,
      wex::data::stc().window(
        wex::data::window().name("Print").parent(m_editors)));
    m_editors->add_page(wex::data::notebook().page(page).key("Print").select());
    m_editors->split("Print", wxBOTTOM);
  }
  else
  {
    page->AppendText(text);
    page->DocumentEnd();
    page->SetSavePoint();
  }

  page->get_lexer().set(stc->get_lexer());

  return true;
}

wex::stc* frame::restore_page(const std::string& key)
{
  if (!m_saved_page.empty() && is_open(wex::path(m_saved_page)))
  {
    m_editors->change_selection(m_saved_page);
    return (wex::stc*)m_editors->page_by_key(m_saved_page);
  }

  return nullptr;
}

void frame::save(wex::stc* editor)
{
  if (!editor->IsModified() || !editor->get_file().file_save())
    return;

  set_recent_file(editor->path());

  if (editor->path() == wex::lexers::get()->path())
  {
    if (wex::lexers::get()->load_document())
    {
      m_editors->for_each<wex::stc>(wex::ID_ALL_STC_SET_LEXER);
      update_listviews();

      // As the lexer might have changed, update status bar field as well.
      update_statusbar(editor, "PaneLexer");
    }
  }
  else if (editor->path() == wex::menus::path())
  {
    wex::vcs::load_document();
  }
  else if (editor->path() == wex::ex::get_macros().path())
  {
    wex::ex::get_macros().load_document();
  }
  else if (editor->path() == wex::config::path())
  {
    wex::config::read();
  }
}

bool frame::saveas(wex::file* f, const std::string& name)
{
  if (!name.empty())
  {
    if (!f->file_save(wex::path(name)))
    {
      return false;
    }
  }
  else
  {
    if (wex::file_dialog dlg(
          f,
          wex::data::window().style(wxFD_SAVE).parent(this).title(
            wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS).ToStdString()));
        dlg.ShowModal() != wxID_OK ||
        !f->file_save(wex::path(dlg.GetPath().ToStdString())))
    {
      return false;
    }
  }

  return true;
}

void frame::saveas(wex::stc* editor, const std::string& name)
{
  if (editor->get_file().is_contents_changed())
  {
    if (const auto old(editor->get_file().path());
        saveas(&editor->get_file(), name))
    {
      open_file(editor->get_file().path(), wex::data::stc(m_app->data()));
      auto* page = (wex::stc*)m_editors->page_by_key(old.string());
      page->get_file().file_load(old);
    }
  }
  else
  {
    if (wex::file f(editor->get_file()); saveas(&f, name))
    {
      open_file(f.path(), wex::data::stc(m_app->data()));
    }
  }
}

void frame::shift_double_click()
{
  m_find_files->set_root();
  m_find_files->Show();
}

bool frame::save_current_page(const std::string& key)
{
  m_saved_page = m_editors->current_page_key();
  return true;
}

void frame::show_vcs()
{
  if (wex::vcs vcs;
      vcs.use() && wex::vcs::size() > 0 && m_editors->GetPageCount() > 0)
  {
    statustext_vcs(dynamic_cast<wex::stc*>(
      m_editors->GetPage(m_editors->GetPageCount() - 1)));
  }
}

void frame::statusbar_clicked(const std::string& pane)
{
  if (pane == "PaneTheme")
  {
    if (wex::lexers::get()->show_theme_dialog(m_editors))
    {
      m_editors->for_each<wex::stc>(wex::ID_ALL_STC_SET_LEXER_THEME);

      update_listviews();

      wex::lexers::get()->apply_default_style(
        [=, this](const std::string& back)
        {
          m_dirctrl->GetTreeCtrl()->SetBackgroundColour(wxColour(back));
        },
        [=, this](const std::string& fore)
        {
          m_dirctrl->GetTreeCtrl()->SetForegroundColour(wxColour(fore));
        });

      m_statusbar->pane_show("PaneLexer", !wex::lexers::get()->theme().empty());

      statustext(wex::lexers::get()->theme(), "PaneTheme");
    }
  }
  else
  {
    decorated_frame::statusbar_clicked(pane);
  }
}

void frame::sync_all()
{
  m_editors->for_each<wex::stc>(wex::ID_ALL_STC_SYNC);
}

void frame::sync_close_all(wxWindowID id)
{
  decorated_frame::sync_close_all(id);

  if (is_closing())
    return;

  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      SetTitle(wxTheApp->GetAppDisplayName());
      statustext(std::string(), std::string());
      statustext(std::string(), "PaneFileType");
      statustext(std::string(), "PaneInfo");
      statustext(std::string(), "PaneLexer");

      if (pane_is_shown("PROJECTS"))
      {
        pane_maximize("PROJECTS");
      }
      break;

    case ID_NOTEBOOK_LISTS:
      pane_show("OUTPUT", false);
      break;

    case ID_NOTEBOOK_PROJECTS:
      pane_show("PROJECTS", false);
      break;

    default:
      assert(0);
  }
}

void frame::update_listviews()
{
  m_lists->for_each<wex::del::file>(wex::ID_ALL_CONFIG_GET);
  m_projects->for_each<wex::del::file>(wex::ID_ALL_CONFIG_GET);

  if (m_history != nullptr)
  {
    m_history->config_get();
  }

  if (const auto* dlg = wex::stc::get_config_dialog(); dlg != nullptr)
  {
    const auto& item(dlg->find(_("stc.link.Include directory")));

    if (auto* lv = (wex::listview*)item.window(); lv != nullptr)
    {
      lv->config_get();
    }
  }
}
