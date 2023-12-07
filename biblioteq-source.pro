FORMS           = \
                  UI/biblioteq_bookcopybrowser.ui \
                  UI/biblioteq_bookinfo.ui \
                  UI/biblioteq_branch_s.ui \
                  UI/biblioteq_copybrowser.ui \
                  UI/biblioteq_customquery.ui \
                  UI/biblioteq_dbenumerations.ui \
                  UI/biblioteq_documentationwindow.ui \
                  UI/biblioteq_errordiag.ui \
                  UI/biblioteq_files.ui \
                  UI/biblioteq_generalmessagediag.ui \
                  UI/biblioteq_import.ui \
                  UI/biblioteq_mainwindow.ui \
                  UI/biblioteq_merge_sqlite_databases.ui \
                  UI/biblioteq_otheroptions.ui \
                  UI/biblioteq_pdfreader.ui \
                  UI/biblioteq_photograph.ui \
                  UI/biblioteq_photographcompare.ui \
                  UI/biblioteq_photographinfo.ui \
                  UI/biblioteq_photographview.ui \
                  UI/biblioteq_sruResults.ui \
                  UI/biblioteq_tracks.ui \
                  UI/biblioteq_z3950results.ui

HEADERS		= Source/biblioteq.h \
                  Source/biblioteq_bgraphicsscene.h \
		  Source/biblioteq_book.h \
		  Source/biblioteq_copy_editor.h \
		  Source/biblioteq_copy_editor_book.h \
                  Source/biblioteq_dbenumerations.h \
                  Source/biblioteq_documentationwindow.h \
                  Source/biblioteq_files.h \
		  Source/biblioteq_filesize_table_item.h \
                  Source/biblioteq_generic_thread.h \
		  Source/biblioteq_hyperlinked_text_edit.h \
                  Source/biblioteq_image_drop_site.h \
                  Source/biblioteq_import.h \
		  Source/biblioteq_item.h \
                  Source/biblioteq_main_table.h \
                  Source/biblioteq_myqstring.h \
                  Source/biblioteq_otheroptions.h \
                  Source/biblioteq_pdfreader.h \
                  Source/biblioteq_photograph_compare.h \
		  Source/biblioteq_photographcollection.h \
                  Source/biblioteq_photograph_view.h \
                  Source/biblioteq_sqlite_merge_databases.h \
		  Source/biblioteq_sruResults.h \
		  Source/biblioteq_z3950results.h

RESOURCES	= Documentation/documentation.qrc \
                  Icons/icons.qrc \
                  Translations/translations.qrc

SOURCES		= Source/biblioteq_a.cc \
                  Source/biblioteq_b.cc \
                  Source/biblioteq_bgraphicsscene.cc \
                  Source/biblioteq_book.cc \
		  Source/biblioteq_c.cc \
                  Source/biblioteq_callnum_table_item.cc \
                  Source/biblioteq_copy_editor.cc \
                  Source/biblioteq_copy_editor_book.cc \
                  Source/biblioteq_dbenumerations.cc \
                  Source/biblioteq_d.cc \
                  Source/biblioteq_documentationwindow.cc \
                  Source/biblioteq_files.cc \
		  Source/biblioteq_filesize_table_item.cc \
                  Source/biblioteq_generic_thread.cc \
                  Source/biblioteq_hyperlinked_text_edit.cc \
                  Source/biblioteq_image_drop_site.cc \
                  Source/biblioteq_import.cc \
                  Source/biblioteq_item.cc \
                  Source/biblioteq_main_table.cc \
                  Source/biblioteq_marc.cc \
                  Source/biblioteq_misc_functions.cc \
                  Source/biblioteq_myqstring.cc \
                  Source/biblioteq_numeric_table_item.cc \
                  Source/biblioteq_open_library.cc \
                  Source/biblioteq_otheroptions.cc \
                  Source/biblioteq_pdfreader.cc \
                  Source/biblioteq_photograph_compare.cc \
                  Source/biblioteq_photographcollection.cc \
                  Source/biblioteq_photograph_view.cc \
                  Source/biblioteq_sqlite_merge_databases.cc \
                  Source/biblioteq_sruResults.cc \
                  Source/biblioteq_z3950results.cc

TRANSLATIONS    = Translations/biblioteq_ar_JO.ts \
		  Translations/biblioteq_cs_CZ.ts \
		  Translations/biblioteq_de_DE.ts \
                  Translations/biblioteq_el_GR.ts \
                  Translations/biblioteq_es_AR.ts \
                  Translations/biblioteq_fr_FR.ts \
                  Translations/biblioteq_he_IL.ts \
		  Translations/biblioteq_hu_HU.ts \
		  Translations/biblioteq_it_IT.ts \
                  Translations/biblioteq_nl_BE.ts \
                  Translations/biblioteq_nl_NL.ts \
		  Translations/biblioteq_pl_PL.ts \
		  Translations/biblioteq_pt_PT.ts \
		  Translations/biblioteq_ru_RU.ts \
                  Translations/biblioteq_zh_CN.ts

UI_DIR          = temp
