////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "decorated-frame.h"

class app;
class find_files;

class frame : public decorated_frame
{
public:
  explicit frame(app* app);

  void show_vcs();

private:
  // All overrides.

  wex::del::listview* activate(
    wex::data::listview::type_t type,
    const wex::lexer*           lexer = nullptr) override;

  void debug_exe(const wex::path& p) override;

  bool exec_ex_command(wex::ex_command& command) override;

  wex::process* get_process(const std::string& command) override;

  wex::del::file* get_project() override;

  bool is_open(const wex::path& filename) override;

  void on_command_item_dialog(wxWindowID dialogid, const wxCommandEvent& event)
    override;

  wex::factory::stc* open_file(
    const wex::path&      filename,
    const wex::data::stc& data = wex::data::stc()) override;

  wex::factory::stc* open_file(
    const wex::path&      filename,
    const std::string&    text,
    const wex::data::stc& data = wex::data::stc()) override;

  wex::factory::stc* open_file_vcs(
    const wex::path&      filename,
    wex::vcs_entry&       vcs,
    const wex::data::stc& data = wex::data::stc()) override;

  void open_file_same_page(const wex::path& p) override;

  bool print_ex(wex::syntax::stc* stc, const std::string& text) override;

  wex::stc* restore_page(const std::string& key) override;

  bool save_current_page(const std::string& key) override;

  void shift_double_click() override;

  void statusbar_clicked(const std::string& pane) override;

  void sync_all() override;

  void sync_close_all(wxWindowID id) override;

  // All others.

  void app_handle();
  void bind();
  void on_command(wxCommandEvent& event);
  void on_update_ui(wxUpdateUIEvent& event);

  wex::factory::stc* open_file_blame(
    const wex::path&      filename,
    wex::vcs_entry&       vcs,
    const wex::data::stc& data = wex::data::stc());
  void save(wex::stc* stc);
  bool saveas(wex::file* f, const std::string& name);
  void saveas(wex::stc* stc, const std::string& name);
  void setup_stdin();
  void toolbar_handle();
  void update_listviews();

  bool m_maximized{false};

  int m_split_id{1};

  std::string m_saved_page;

  std::vector<wex::path> m_files;

  find_files* m_find_files{nullptr};

  DECLARE_EVENT_TABLE()
};
