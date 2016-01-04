INCLUDEPATH += . $$PWD

HEADERS += $$PWD/procmaps.h \
           $$PWD/report.h \
           $$PWD/backtrace.h \

SOURCES += $$PWD/procmaps.cpp \
           $$PWD/report.cpp \
           $$PWD/backtrace.cpp \

CONFIG += c++11
LIBS += -lboost_serialization -lunwind
