////////////////////////////////////////////////////////////////////////////////
// Name:      decorated-frame.h
// Purpose:   Declaration of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

class app;
class editors;

class decorated_frame : public wex::del::frame
{
public:
  decorated_frame(app* app);

protected:
  void add_pane_history();

  editors*            m_editors{nullptr};
  wex::del::dirctrl*  m_dirctrl{nullptr};
  wex::del::listview* m_history{nullptr};

  app* m_app;

  wex::notebook *m_lists{nullptr}, *m_projects{nullptr};

  wex::process* m_process{nullptr};

private:
  bool allow_close(wxWindowID id, wxWindow* page) override;
  void on_notebook(wxWindowID id, wxWindow* page) override;

  const std::string allow_move_ext() const;

  const std::string m_project_wildcard{_("Project Files") + " (*.prj)|*.prj"};

  int m_project_id{1};
};
