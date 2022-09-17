////////////////////////////////////////////////////////////////////////////////
// Name:      on-command.cpp
// Purpose:   Implementation of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "defs.h"
#include "editors.h"
#include "frame.h"

void frame::on_command(wxCommandEvent& event)
{
  switch (auto* editor = dynamic_cast<wex::stc*>(get_stc()); event.GetId())
  {
    // edit commands
    // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
    // as wxID_ABOUT etc. is used here and not in the editor.
    // That causes appl to hang.
    case wxID_UNDO:
    case wxID_REDO:
    case wxID_CLEAR:
    case wxID_COPY:
    case wxID_CUT:
    case wxID_DELETE:
    case wxID_JUMP_TO:
    case wxID_PASTE:
    case wxID_SELECTALL:
      if (editor != nullptr)
      {
        wxPostEvent(editor, event);
      }
      else if (get_listview() != nullptr)
      {
        wxPostEvent(get_listview(), event);
      }
      else if (auto* fs = dynamic_cast<wex::factory::stc*>(get_stc());
               fs != nullptr)
      {
        wxPostEvent(fs, event);
      }
      break;

    case wxID_SAVE:
      if (editor != nullptr)
      {
        save(editor);
      }
      break;

    case wxID_SAVEAS:
      if (editor != nullptr)
      {
        saveas(editor, event.GetString());
      }
      break;

    case ID_EDIT_MACRO_PLAYBACK:
      if (editor != nullptr)
        editor->get_vi().get_macros().mode().transition(
          "@",
          &editor->get_vi(),
          true);
      break;

    case ID_EDIT_MACRO_START_RECORD:
    case ID_EDIT_MACRO_STOP_RECORD:
      if (editor != nullptr)
        editor->get_vi().get_macros().mode().transition(
          "q",
          &editor->get_vi(),
          true);
      break;

    case ID_SPLIT:
    case ID_SPLIT_HORIZONTALLY:
    case ID_SPLIT_VERTICALLY:
      if (editor == nullptr)
      {
        wex::log::status("No valid focus");
      }
      else
      {
        auto* stc = new wex::stc(
          editor->path(),
          wex::data::stc().window(wex::data::window().parent(m_editors)));
        editor->sync(false);
        stc->sync(false);
        stc->get_vi().copy(&editor->get_vi());

        wex::data::notebook nd;

        if (editor->path().file_exists())
        {
          nd.bitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
            wxFileIconsTable::file));
        }
        else if (!editor->get_lexer().scintilla_lexer().empty())
        {
          stc->get_lexer().set(editor->get_lexer().scintilla_lexer());
        }

        // key should be unique
        nd.key("split" + std::to_string(m_split_id++));

        // Place new page before page for editor.
        m_editors->insert_page(nd.index(m_editors->GetPageIndex(editor))
                                 .page(stc)
                                 .caption(editor->path().filename())
                                 .select());

        stc->SetDocPointer(editor->GetDocPointer());

        if (event.GetId() == ID_SPLIT_HORIZONTALLY)
        {
          m_editors->split(nd.key(), wxBOTTOM);
          m_editors->set_selection(editor->path().string());
        }
        else if (event.GetId() == ID_SPLIT_VERTICALLY)
        {
          m_editors->split(nd.key(), wxRIGHT);
          m_editors->set_selection(editor->path().string());
        }
      }
      break;

    default:
      assert(0);
      break;
  }
}

void frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  switch (dialogid)
  {
    case wxID_PREFERENCES:
      if (event.GetId() != wxID_CANCEL)
      {
        m_editors->for_each<wex::stc>(wex::ID_ALL_CONFIG_GET);

        wex::process::get_shell()->config_get();

        m_statusbar->pane_show(
          "PaneMacro",
          wex::config(_("stc.vi mode")).get(true));
      }
      break;

    case ID_FIND_FILE_DIALOG:
      break;

    case ID_OPTION_LIST:
      if (event.GetId() != wxID_CANCEL)
      {
        update_listviews();
      }
      break;

    case ID_OPTION_TAB:
      if (event.GetId() != wxID_CANCEL)
      {
        m_editors->config_get();
        m_lists->config_get();
        m_projects->config_get();

        if (m_history != nullptr)
        {
          m_history->config_get();
        }
      }
      break;

    default:
      decorated_frame::on_command_item_dialog(dialogid, event);
  }
}
