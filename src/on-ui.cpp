////////////////////////////////////////////////////////////////////////////////
// Name:      on-ui.cpp
// Purpose:   Implementation of class frame::on_update_ui
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "defs.h"
#include "editors.h"
#include "frame.h"

void frame::on_update_ui(wxUpdateUIEvent& event)
{
  if (IsBeingDeleted())
  {
    return;
  }

  switch (event.GetId())
  {
    case wxID_EXECUTE:
      event.Enable(
        !is_closing() && !m_process->data().exe().empty() &&
        !m_process->is_running());
      break;

    case wxID_STOP:
      event.Enable(m_process->is_running());
      break;

    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (get_stc() != nullptr && get_stc()->GetLength() > 0) ||
        (get_listview() != nullptr && get_listview()->GetItemCount() > 0));
      break;

    case wex::ID_ALL_CLOSE:
    case wex::ID_ALL_SAVE:
      event.Enable(m_editors->GetPageCount() > 2);
      break;

    case wex::del::ID_PROJECT_SAVE:
      event.Enable(
        get_project() != nullptr && get_project()->is_contents_changed());
      break;

    default:
    {
      if (auto* editor = dynamic_cast<wex::stc*>(get_stc()); editor != nullptr)
      {
        switch (event.Enable(true); event.GetId())
        {
          case wxID_FIND:
          case wxID_JUMP_TO:
          case wxID_REPLACE:
          case wxID_CUT:
          case wxID_COPY:
          case wxID_PASTE:
          case wxID_CLEAR:
          case wxID_DUPLICATE:
          case wxID_SELECTALL:
          case wex::ID_EDIT_FIND_NEXT:
          case wex::ID_EDIT_FIND_PREVIOUS:
            event.Enable(editor->GetLength() > 0);
            break;

          case ID_EDIT_MACRO:
            event.Enable(
              editor->get_vi().is_active() &&
              !editor->get_vi().get_macros().mode().is_recording() &&
              wex::ex::get_macros().path().file_exists());
            break;

          case ID_EDIT_MACRO_MENU:
            event.Enable(editor->get_vi().is_active());
            break;

          case ID_EDIT_MACRO_PLAYBACK:
            event.Enable(
              editor->get_vi().is_active() &&
              editor->get_vi().get_macros().size() > 0 &&
              !editor->get_vi().get_macros().mode().is_recording());
            break;

          case ID_EDIT_MACRO_START_RECORD:
            event.Enable(
              editor->get_vi().is_active() &&
              !editor->get_vi().get_macros().mode().is_recording());
            break;

          case ID_EDIT_MACRO_STOP_RECORD:
            event.Enable(editor->get_vi().get_macros().mode().is_recording());
            break;

          case wxID_SAVE:
            event.Enable(!editor->path().empty() && editor->GetModify());
            break;

          case wxID_REDO:
            event.Enable(editor->CanRedo());
            break;

          case wxID_UNDO:
            event.Enable(editor->CanUndo());
            break;

          case wex::ID_EDIT_CONTROL_CHAR:
            if (
              editor->GetReadOnly() && editor->GetSelectedText().length() != 1)
            {
              event.Enable(false);
            }
            break;

          default:
            assert(0);
        }
      }
      else if (auto* list = dynamic_cast<wex::del::file*>(get_listview());
               list != nullptr && list->IsShown())
      {
        event.Enable(false);

        if (
          event.GetId() > wex::ID_TOOL_LOWEST &&
          event.GetId() < wex::ID_TOOL_HIGHEST)
        {
          event.Enable(list->GetSelectedItemCount() > 0);
        }
        else if (event.GetId() == wxID_FIND)
        {
          event.Enable(list->GetItemCount() > 0);
        }
      }
      else
      {
        event.Enable(false);
      }
    }
  }
}
