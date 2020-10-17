////////////////////////////////////////////////////////////////////////////////
// Name:      editors.h
// Purpose:   Declaration of editors class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/notebook.h>

class editors : public wex::notebook
{
public:
  editors(const wex::data::window& data);

  bool is_split() const { return m_split; };
  void reset() { m_split = false; };

private:
  bool m_split{false};
};
