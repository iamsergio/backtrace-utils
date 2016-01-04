INCLUDEPATH += .

HEADERS += $$PWD/backtracecache.h \
           $$PWD/backtraceview.h \
           $$PWD/backtracemodel.h \
           $$PWD/backtraceproxymodel.h \
           $$PWD/mainwindowbase.h

SOURCES += $$PWD/backtracecache.cpp \
           $$PWD/backtraceview.cpp \
           $$PWD/backtracemodel.cpp \
           $$PWD/backtraceproxymodel.cpp \
           $$PWD/mainwindowbase.cpp

CONFIG += c++11

FORMS += $$PWD/mainwindow.ui
