////////////////////////////////////////////////////////////////////////////////
// Name:      find-files.cpp
// Purpose:   Implementation of class find_files
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "find-files.h"

find_files::find_files(wex::frame* f)
  : item_dialog(
      {{_("find.File"),
        wex::item::COMBOBOX,
        std::any(),
        wex::data::item().window(
          wex::data::window().style(wxTE_PROCESS_ENTER))},
       {_("find.Max"), wex::item::TEXTCTRL_INT, std::string("50")},
       {_("find.Matches"),
        wex::data::listview().type(wex::data::listview::FILE)}},
      wex::data::window().title(_("Find Files")).size({400, 400}).button(0))
  , m_listview((wex::listview*)find(_("find.Matches")).window())
  , m_combobox((wxComboBox*)find(_("find.File")).window())
  , m_frame(f)
{
  assert(m_combobox != nullptr && m_listview != nullptr);

  m_combobox->SetFocus();
  m_combobox->Bind(
    wxEVT_TEXT_ENTER,
    [=, this](wxCommandEvent& event)
    {
      run();
    });
}

find_files::~find_files()
{
  reload(true);

  std::list<std::string> filtered;
  const std::string      item(_("find.Matches"));

  for (const auto& v : wex::config(item).get(filtered))
  {
    if (filtered.size() < 5)
    {
      filtered.emplace_back(v);
    }
  }

  wex::config(item).set(filtered);
}

void find_files::run()
{
  set_root();

  reload(true);

  m_combobox->SetInsertionPointEnd();
  m_listview->clear();

  wex::dir(
    m_root,
    wex::data::dir()
      .file_spec("*" + m_combobox->GetValue() + "*")
      .max_matches(wex::config(_("find.Max")).get(50))
      .type(wex::data::dir::type_t()
              .set(wex::data::dir::FILES)
              .set(wex::data::dir::RECURSIVE)),
    m_listview)
    .find_files();
}

void find_files::set_root()
{
  if (auto* editor = m_frame->get_stc(); editor != nullptr)
  {
    m_root = wex::vcs({editor->path()}).toplevel();

    wex::log::trace("find files root") << m_root.string();
  }
}
