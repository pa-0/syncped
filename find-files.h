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
  find_files();
  
  // Destructor.
 ~find_files();

  // Sets root path to search for, using toplevel vcs dir of
  // current page on specified frame.
  void set_root(wex::frame* f);

private:
  void run();

  wex::path m_root;

  wex::listview* m_listview{nullptr};
  wxComboBox*    m_combobox{nullptr};
};
