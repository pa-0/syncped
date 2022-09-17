////////////////////////////////////////////////////////////////////////////////
// Name:      ascii-table.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "decorated-frame.h"

void decorated_frame::build_ascii_table()
{
  auto* stc = dynamic_cast<wex::stc*>(
    open_file(wex::path("Ascii table"), wex::data::stc()));

  // Do not show an edge, eol whitespace for ascii table.
  stc->SetEdgeMode(wxSTC_EDGE_NONE);
  stc->SetViewEOL(false);
  stc->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  stc->SetTabWidth(5);

  for (int i = 1; i <= 255; i++)
  {
    switch (i)
    {
      case 9:
        stc->add_text(wxString::Format("%3d\tTAB", i));
        break;
      case 10:
        stc->add_text(wxString::Format("%3d\tLF", i));
        break;
      case 13:
        stc->add_text(wxString::Format("%3d\tCR", i));
        break;
      default:
        stc->add_text(wxString::Format("%3d\t%c", i, (wxUniChar)i));
    }
    stc->add_text((i % 5 == 0) ? stc->eol() : "\t");
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  stc->SetReadOnly(true);
}
