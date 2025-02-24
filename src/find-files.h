////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.h
// Purpose:   Declaration of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

class frame;

class find_files : public wex::item_dialog
{
public:
  // Constructor.
  explicit find_files(wex::del::frame* f);

  // Destroys window, saves data.
  bool Destroy() override;

  // Sets root path to search for, using toplevel vcs dir of
  // current page on frame.
  void set_root();

private:
  void run(bool is_enter_key);

  wex::del::frame* m_frame;
  wex::listview*   m_listview;
  wex::path        m_root;

  wxComboBox* m_combobox;

  std::string m_value;
};
