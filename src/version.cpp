////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"

const wex::version_info app::version() const
{
  std::string description(_("This program offers a portable text or"
                            "binary editor\n"
                            "with automatic syncing."));
#ifdef __WXMSW__
  description += _("All its config files are read\n"
                   "and saved in the same directory as "
                   "where the executable is.");
#endif
  description += _("\n\nUsing:\n") + wex::external_libraries().str();

  return wex::version_info(
    {"syncped",
     25,
     4,
     0,
     description,
     "(c) 1998-2024, Anton van Wezenbeek. " + _("All rights reserved.")});
}
