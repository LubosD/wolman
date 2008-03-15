TEMPLATE = app
TARGET = wolman
LIBS += -lpcap -lnet
QT += network

#INSTALLS += target

# Input
HEADERS += wolman.h MainDlg.h MachineDlg.h \
 NetIface.h \
 NetListener.h
FORMS += MainDlg.ui MachineDlg.ui
SOURCES += wolman.cpp MainDlg.cpp \
 NetIface.cpp \
 NetListener.cpp \
 MachineDlg.cpp

RESOURCES += gfx/resources.qrc

