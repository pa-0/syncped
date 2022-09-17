////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.h
// Purpose:   Declaration of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

class frame;

class find_files : public wex::item_dialog
{
public:
  // Constructor.
  explicit find_files(wex::frame* f);

  // Destroys window, saves data.
  bool Destroy() override;

  // Sets root path to search for, using toplevel vcs dir of
  // current page on frame.
  void set_root();

private:
  void run();

  wex::frame*    m_frame;
  wex::listview* m_listview;
  wex::path      m_root;

  wxComboBox* m_combobox;
};
