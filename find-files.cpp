////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.cpp
// Purpose:   Implementation of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "find-files.h"
#include "frame.h"

find_files::find_files()
  : item_dialog(
      {{_("find.File"),
        wex::item::COMBOBOX,
        std::any(),
        wex::data::item().window(
          wex::data::window().style(wxTE_PROCESS_ENTER))},
       {_("find.Max"), wex::item::TEXTCTRL_INT, std::string("50")},
       {_("find.Matches"), wex::data::listview().type(wex::data::listview::FILE)}},
      wex::data::window().title(_("Find Files")).size({400, 400}).button(0))
{
  auto* lv = (wex::listview*)find(_("find.Matches")).window();
  auto* cb = (wxComboBox*)find(_("find.File")).window();

  assert(cb != nullptr && lv != nullptr);

  cb->SetFocus();
  cb->Bind(
    wxEVT_TEXT_ENTER,
    [=, this](wxCommandEvent& event)
    {
      reload(true);

      cb->SetInsertionPointEnd();
      lv->clear();

      if (const auto& v(wex::get_all_files(
            m_root,
            wex::data::dir()
              .file_spec("*" + cb->GetValue() + "*")
              .max_matches(wex::config(_("find.Max")).get(50))
              .type(wex::data::dir::type_t()
                      .set(wex::data::dir::FILES)
                      .set(wex::data::dir::RECURSIVE))));
          !v.empty())
      {
        for (const auto& e : v)
        {
          wex::listitem(lv, e).insert();
        }

        reload(true);
      }
    });
}

void find_files::set_root(wex::frame* f)
{
  if (auto* editor = f->get_stc(); editor != nullptr)
  {
    m_root = wex::vcs({editor->path()}).toplevel();

    wex::log::trace("find files root") << m_root.string();
  }
}
