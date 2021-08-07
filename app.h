////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'app'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

class app : public wex::app
{
public:
  auto& data() { return m_data; }

  auto& get_files() const { return m_files; }
  auto& get_output() const { return m_output; }
  auto& get_scriptout() const { return m_scriptout; }
  auto  get_split() const { return m_split; }
  auto& get_tag() const { return m_tag; }

  auto is_debug() const { return m_is_debug; }
  auto is_echo() const { return m_is_echo; }
  auto is_output() const { return m_is_output; }
  auto is_project() const { return m_is_project; }
  auto is_stdin() const { return m_is_stdin; }

  void reset();

  const wex::version_info version() const;

private:
#ifdef __WXOSX__
  void MacOpenFiles(const wxArrayString& fileNames) override;
#endif

  bool OnInit() override;

  void show_locale();

  std::string m_output, m_scriptout, m_tag;

  std::vector<wex::path> m_files;

  bool m_is_echo{false}, m_is_debug{false}, m_is_output{false},
    m_is_project{false}, m_is_stdin{false}, m_keep{false};

  int m_split{-1};

  wex::data::stc m_data;
};
