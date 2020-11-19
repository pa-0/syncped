////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.cpp
// Purpose:   Implementation of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

#include "find-files.h"
#include "frame.h"

find_files::find_files()
  : item_dialog(
      {{"find.File",
        wex::item::COMBOBOX,
        std::any(),
        wex::data::item().window(
          wex::data::window().style(wxTE_PROCESS_ENTER))},
       {"find.Max", wex::item::TEXTCTRL_INT, std::string("50")},
       {"find.Matches", wex::data::listview().type(wex::data::listview::FILE)}},
      wex::data::window().title("Find Files").size({400, 400}).button(0))
{
  auto* l = (wex::listview*)find("find.Matches").window();
  auto* c = (wxComboBox*)find("find.File").window();

  assert(c != nullptr && l != nullptr);

  c->SetFocus();
  c->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    reload(true);

    c->SetInsertionPointEnd();

    if (const auto& v(wex::get_all_files(
          m_root,
          wex::data::dir()
            .file_spec("*" + c->GetValue() + "*")
            .max_matches(wex::config("find.Max").get(50))
            .type(wex::data::dir::type_t()
                    .set(wex::data::dir::FILES)
                    .set(wex::data::dir::RECURSIVE))));
        !v.empty())
    {
      l->clear();

      for (const auto& e : v)
      {
        wex::listitem(l, e).insert();
      }

      reload(true);
    }
  });
}

void find_files::set_root(frame* f)
{
  if (auto* editor = f->get_stc(); editor != nullptr)
  {
    m_root = wex::vcs({editor->get_filename()}).toplevel();

    wex::log::verbose("find files root") << m_root.string();
  }
}
