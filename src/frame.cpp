////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
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
  app_handle();

  statustext(wex::lexers::get()->theme(), "PaneTheme");

  if (
    m_editors->GetPageCount() > 0 &&
    !m_app->data().flags().test(wex::data::stc::WIN_EX))
  {
    m_editors->GetPage(m_editors->GetPageCount() - 1)->SetFocus();
  }

  toolbar_handle();
  bind();

  get_find_toolbar()->add_find();
  get_options_toolbar()->add_checkboxes_standard();

  if (m_files.empty())
  {
    m_app->reset();
  }

  if (m_app->is_stdin())
  {
    setup_stdin();
  }

  if (m_app->data().flags().test(wex::data::stc::WIN_EX))
  {
    if (auto* stc(((wex::stc*)m_editors->GetCurrentPage())); stc != nullptr)
    {
      show_ex_bar(SHOW_BAR, stc);
    }
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

    const auto name =
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
  {
    return false;
  }

  bool handled = false;

  try
  {
    if (m_editors->GetPageCount() > 0)
    {
      if (boost::algorithm::trim_copy(command.command()) == ":n")
      {
        if (m_editors->GetSelection() == m_editors->GetPageCount() - 1)
        {
          return false;
        }

        m_editors->AdvanceSelection();
        handled = true;
      }
      else if (boost::algorithm::trim_copy(command.command()) == ":prev")
      {
        if (m_editors->GetSelection() == 0)
        {
          return false;
        }

        m_editors->AdvanceSelection(false);
        handled = true;
      }

      if (handled && wex::ex::get_macros().mode().is_playback())
      {
        command.set_stc(
          ((wex::stc*)m_editors->GetPage(m_editors->GetSelection())));
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
  {
    return nullptr;
  }

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

bool frame::next_page()
{
  if (m_editors->GetPageCount() < 2)
  {
    return false;
  }

  if (m_editors->GetSelection() == m_editors->GetPageCount() - 1)
  {
    m_editors->SetSelection(0);
  }
  else
  {
    m_editors->AdvanceSelection();
  }

  return true;
}

wex::factory::stc* frame::open_file_vcs(
  const wex::path&      filename,
  wex::vcs_entry&       vcs,
  const wex::data::stc& data)
{
  if (
    vcs.data().exe().find(" blame ") != std::string::npos ||
    vcs.data().args_str().find("blame") != std::string::npos ||
    vcs.get_command().is_blame())
  {
    return open_file_blame(filename, vcs, data);
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

wex::factory::stc* frame::open_file(
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

wex::factory::stc*
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
            wxFileIconsTable::file)));
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
      {
        wf.set(wex::data::stc::WIN_HEX);
      }
      if (m_app->is_debug())
      {
        mf.set(wex::data::stc::MENU_DEBUG);
      }

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
      nd.key(filename.string());

      notebook->add_page(
        nd.page(editor)
          .caption(filename.filename())
          .select()
          .bitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
            wxFileIconsTable::file)));

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
        vcs_blame(editor);
      }
    }
    else
    {
      wex::data::stc(data).set_stc(editor).inject();
    }

    if (editor->is_visual())
    {
      editor->SetFocus();
    }

    return editor;
  }

  return (wex::stc*)page;
}

wex::factory::stc* frame::open_file_blame(
  const wex::path&      filename,
  wex::vcs_entry&       vcs,
  const wex::data::stc& data)
{
  if (auto* page = (wex::stc*)m_editors->set_selection(filename.string());
      page != nullptr)
  {
    vcs_blame_show(&vcs, page);
    return page;
  }
  else
  {
    page = new wex::stc(
      std::string(),
      wex::data::stc(data).window(
        wex::data::window().parent(m_editors).name(filename.string())));

    page->get_lexer().set(wex::path_lexer(filename).lexer(), true);

    m_editors->add_page(wex::data::notebook()
                          .page(page)
                          .key(vcs.data().exe())
                          .caption(vcs.get_blame().caption())
                          .select());

    vcs_blame_show(&vcs, page);
    page->EmptyUndoBuffer();
    page->SetSavePoint();
    page->inject(data.control());
    page->config_get();
    return page;
  }
}

void frame::open_file_same_page(const wex::path& p)
{
  if (auto* page = (wex::stc*)m_editors->GetPage(m_editors->GetSelection());
      page != nullptr)
  {
    m_editors->set_page_text(
      m_editors->key_by_page(page),
      p.string(),
      p.filename());
    page->open(p, wex::data::stc().recent(false));
    page->get_lexer().set(wex::path_lexer(p).lexer().display_lexer(), true);
    page->properties_message();
  }
}

bool frame::print_ex(wex::syntax::stc* stc, const std::string& text)
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
  {
    return;
  }

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
            wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS)));
        dlg.ShowModal() != wxID_OK || !f->file_save(wex::path(dlg.GetPath())))
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
  if (wex::vcs vcs; vcs.use() && wex::vcs::size() > 0 &&
                    m_editors->GetPageCount() > 0 &&
                    // stdin mode (-E) and get_branch do no work together,
                    // boost system locks until enter is given
                    !m_app->is_stdin())
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
  {
    return;
  }

  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      SetTitle(wxTheApp->GetAppDisplayName());

      for (int i = 0; i < get_statusbar()->GetFieldsCount(); i++)
      {
        get_statusbar()->SetStatusText(std::string(), i);
      }

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
