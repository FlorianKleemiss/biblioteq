include(biblioteq-source.pro)

greaterThan(QT_MAJOR_VERSION, 4) {
cache()
}

purge.commands = del *~ && del *\\*~

CONFIG		+= qt release thread warn_on windows
DEFINES		+= BIBLIOTEQ_LINKED_WITH_POPPLER \
                   BIBLIOTEQ_POPPLER_VERSION_DEFINED
LANGUAGE	= C++
QT		+= network sql
QT		-= webkit

greaterThan(QT_MAJOR_VERSION, 4) {
QT              += printsupport widgets
}

QMAKE_CLEAN	+= BiblioteQ.exe
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-align \
                          -Wcast-qual \
                          -Wdouble-promotion \
                          -Wextra \
                          -Wformat=2 \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstrict-overflow=5 \
                          -fwrapv \
                          -mtune=generic \
                          -pedantic \
                          -pie
QMAKE_DISTCLEAN += -r temp

greaterThan(QT_MAJOR_VERSION, 4) {
QMAKE_CXXFLAGS_RELEASE += -std=c++11
QMAKE_DISTCLEAN += .qmake.cache .qmake.stash
}

QMAKE_EXTRA_TARGETS = purge

ICON		= Icons\\book.png
INCLUDEPATH	+= Include.win32 \
                   Include.win32\\poppler\\cpp \
                   Include.win32\\poppler\\qt5 \
                   Source \
                   temp
LIBS		+= -L"." \
                   -L"Libraries.win32\\poppler" \
		   -L"Libraries.win32\\sqlite3" \
                   -L"Libraries.win32\\yaz" \
                   -lpoppler \
                   -lpoppler-qt5 \
                   -lsqlite3 \
                   -lyaz5
RC_FILE		= biblioteq.win.rc
RESOURCES	= Icons\\icons.qrc
PROJECTNAME	= BiblioteQ
TARGET		= BiblioteQ
TEMPLATE        = app
