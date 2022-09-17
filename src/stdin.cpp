////////////////////////////////////////////////////////////////////////////////
// Name:      stdin.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include "editors.h"
#include "frame.h"

void frame::setup_stdin()
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
