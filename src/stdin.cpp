////////////////////////////////////////////////////////////////////////////////
// Name:      stdin.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
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
          if (auto* stc(dynamic_cast<wex::stc*>(m_editors->GetCurrentPage()));
              stc != nullptr)
          {
            wxCommandEvent event(wxEVT_MENU, wex::id::stc::vi_command);

            std::string corrected(
              text == "\n" && !stc->get_vi().mode().is_insert() ? "j" : text);

            if (!stc->get_vi().mode().is_insert() && corrected.size() > 1)
            {
              // The eol was needed for getting input, but should not be
              // added to the event: that would give extra navigation.
              corrected.pop_back();
            }

            event.SetString(corrected);
            wxPostEvent(stc, event);
          }

          text.clear();
        }
      }
    });

  v.detach();
}
