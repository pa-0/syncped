if (GETTEXT_FOUND)
  if (WIN32)
    set(LOCALE_INSTALL_DIR bin)
  else ()
    set(LOCALE_INSTALL_DIR share/locale/)
  endif ()
  
  gettext_process_po_files(nl_NL ALL 
    INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
    PO_FILES syncped-nl.po)
    
  gettext_process_po_files(fr_FR ALL 
    INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
    PO_FILES syncped-fr.po)
endif ()
