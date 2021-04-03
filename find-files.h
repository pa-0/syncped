////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.h
// Purpose:   Declaration of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

class frame;

class find_files : public wex::item_dialog
{
public:
  find_files();

  void set_root(frame* f);

private:
  wex::path m_root;
};
