QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    admin_mainwindow.cpp \
    adminreportdialog.cpp \
    billsdialog.cpp \
    bubblelabel.cpp \
    clickablelabel.cpp \
    dashboardcard.cpp \
    feecardwidget.cpp \
    historylistwidget.cpp \
    imageslot.cpp \
    leftbubble.cpp \
    loginswitch.cpp \
    main.cpp \
    mainwindow.cpp \
    noticedialog.cpp \
    registerdialog.cpp \
    reportitemwidget.cpp \
    reportwidget.cpp \
    resultdialog.cpp \
    retrievedialog.cpp \
    rightbubble.cpp \
    socket.cpp \
    widget.cpp

HEADERS += \
    admin_mainwindow.h \
    adminreportdialog.h \
    billsdialog.h \
    bubblelabel.h \
    clickablelabel.h \
    dashboardcard.h \
    feecardwidget.h \
    global.h \
    historylistwidget.h \
    imageslot.h \
    leftbubble.h \
    loginswitch.h \
    mainwindow.h \
    noticedialog.h \
    registerdialog.h \
    reportitemwidget.h \
    reportwidget.h \
    resultdialog.h \
    retrievedialog.h \
    rightbubble.h \
    socket.h \
    widget.h

FORMS += \
    admin_mainwindow.ui \
    adminreportdialog.ui \
    billsdialog.ui \
    feecardwidget.ui \
    historylistwidget.ui \
    leftbubble.ui \
    mainwindow.ui \
    noticedialog.ui \
    registerdialog.ui \
    reportitemwidget.ui \
    reportwidget.ui \
    resultdialog.ui \
    retrievedialog.ui \
    rightbubble.ui \
    widget.ui

TRANSLATIONS +=
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    picture.qrc \
    resources.qrc

DISTFILES +=

