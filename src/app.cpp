////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class app
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(app);

#ifdef __WXOSX__
void app::MacOpenFiles(const wxArrayString& fileNames)
{
  auto* frame = dynamic_cast<::frame*>(GetTopWindow());
  wex::open_files(frame, wex::to_vector_path(fileNames).get(), m_data);
}
#endif

bool app::OnInit()
{
  SetAppName("syncped");

  bool        list_lexers{false}, show_locale{false};
  std::string ctags_file;

  wex::data::cmdline cmdl(argc, argv);

  if (bool exit = false;
      !wex::cmdline(
         {// --- boolean options ---
          {{"end,+", "start at end any file opened"},
           [&](const std::any& s)
           {
             m_data.control(wex::data::control().command("G"));
           }},

          {{"debug,d",
            "use debug mode, opens last specified file as debug target"},
           [&](bool on)
           {
             m_is_debug = on;
           }},

          {{"ex", "ex mode"},
           [&](bool on)
           {
             if (!on)
               return;
             m_data.flags(
               wex::data::stc::window_t().set(wex::data::stc::WIN_EX),
               wex::data::control::OR);
           }},

          {{"stdin,E", "use stdin"},
           [&](bool on)
           {
             m_is_stdin = on;
           }},

          {{"hex,H", "hex mode"},
           [&](bool on)
           {
             if (!on)
               return;
             m_data.flags(
               wex::data::stc::window_t().set(wex::data::stc::WIN_HEX),
               wex::data::control::OR);
           }},

          {{"info,i", "show versions"},
           [&](bool on)
           {
             if (on)
             {
               std::cout << version().get() << " using\n"
                         << wex::external_libraries().str();
               exit = true;
             }
           }},

          {{"keep,k", "keep vi options: +, c, s"},
           [&](bool on)
           {
             m_keep = on;
           }},

          {{"list-lexers,L", "show list of lexers"},
           [&](bool on)
           {
             list_lexers = on;
           }},

          {{"locale,l", "show locale"},
           [&](bool on)
           {
             show_locale = on;
           }},

          {{"no-config,n", "do not save json config on exit"},
           [&](bool on)
           {
             if (on)
             {
               wex::config::discard();
             }
           }},

          {{"project,p", "open specified files as projects"},
           [&](bool on)
           {
             m_is_project = on;
           }},

          {{"splithor,o", "split tabs horizontally"},
           [&](bool on)
           {
             if (on)
               m_split = wxBOTTOM;
           }},

          {{"splitver,O", "split tabs vertically"},
           [&](bool on)
           {
             if (on)
               m_split = wxRIGHT;
           }},

          {{"readonly,R", "readonly mode"},
           [&](bool on)
           {
             if (on)
               m_data.flags(
                 wex::data::stc::window_t().set(wex::data::stc::WIN_READ_ONLY),
                 wex::data::control::OR);
           }}},

         {// --- options with arguments ---
          {{"command,c", "vi command"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              m_data.control(
                wex::data::control().command(std::any_cast<std::string>(s)));
            }}},

          {{"config,j", "json config file"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              wex::config::set_path(wex::path(std::any_cast<std::string>(s)));
            }}},

          {{"tag,t", "start at tag"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              m_tag = std::any_cast<std::string>(s);
            }}},

          {{"tagfile,u", "use tagfile"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              // do not open here, to ensure app::OnInit is done (logging)
              ctags_file = std::any_cast<std::string>(s);
            }}},

          {{"scriptin,s", "script in (:so <arg> applied on any file opened)"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              m_data.control(wex::data::control().command(
                ":so " + std::any_cast<std::string>(s)));
            }}}},

         {{"files",
           "input file[:line number][:column number]\n"
           "or project files is -p was specified\n"
           "or executable file if -d was specified and last file has no known "
           "extension"},
          [&](const std::vector<std::string>& v)
          {
            std::transform(
              v.begin(),
              v.end(),
              std::back_inserter(m_files),
              [](const auto& i)
              {
                return wex::path(i);
              });
          }})
         .parse(cmdl) ||
      exit || !wex::del::app::OnInit())
  {
    return false;
  }

  if (list_lexers)
  {
    // code cannot be part of lambda, as OnInit is required
    for (const auto& l : wex::lexers::get()->get_lexers())
    {
      if (!l.display_lexer().empty())
        std::cout << l.display_lexer() << "\n";
    }

    return false;
  }

  if (show_locale)
  {
    // code cannot be part of lambda, as OnInit is required
    switch (get_language())
    {
      case wxLANGUAGE_UNKNOWN:
        // error already reported
        break;

      case wxLANGUAGE_DEFAULT:
        std::cout << "default locale\n";
        break;

      default:
        if (!get_catalog_dir().empty())
        {
          std::cout << "catalog dir: " << get_catalog_dir() << "\n";
        }
    }

    return false;
  }

  if (!ctags_file.empty())
  {
    wex::ctags::open(ctags_file);
  }

  auto* f = new frame(this);

  if (!f->is_closing())
  {
    f->Show();
    f->show_vcs();
  }

  return !f->is_closing();
}

void app::reset()
{
  if (!m_keep)
  {
    m_data.control(wex::data::control().command(""));
  }

  m_is_project = false;
  m_split      = -1;
  m_tag.clear();
}
