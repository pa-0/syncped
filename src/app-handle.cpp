////////////////////////////////////////////////////////////////////////////////
// Name:      app-handle.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"
#include "editors.h"
#include "frame.h"

void frame::app_handle()
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
      else if (!m_app->data().flags().test(wex::data::stc::WIN_EX))
      {
        auto* page = new wex::stc(
          wex::path("no name"),
          wex::data::stc(m_app->data())
            .window(wex::data::window().parent(m_editors)));

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
}
