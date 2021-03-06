#-------------------------------------------------
#
# LAN Messenger project file
#
#-------------------------------------------------

QT       += core gui network xml webkit
QTPLUGIN += qjpeg qgif

QT += widgets multimedia webkitwidgets xmlpatterns
CONFIG  += c++11
#QMAKE_CXXFLAGS += -std=c++1y

win32: TARGET = lmc
unix: TARGET = lan-messenger
macx: TARGET  = "LAN-Messenger"
TEMPLATE = app

SOURCES += \
    usertreewidget.cpp \
    udpnetwork.cpp \
    transferwindow.cpp \
    transferlistview.cpp \
    tcpnetwork.cpp \
    strings.cpp \
    soundplayer.cpp \
    shared.cpp \
    settingsdialog.cpp \
    settings.cpp \
    network.cpp \
    netstreamer.cpp \
    messagingproc.cpp \
    messaging.cpp \
    message.cpp \
    mainwindow.cpp \
    main.cpp \
    lmc.cpp \
    imagepickeraction.cpp \
    imagepicker.cpp \
    historywindow.cpp \
    history.cpp \
    helpwindow.cpp \
    filemodelview.cpp \
    datagram.cpp \
    broadcastwindow.cpp \
    aboutdialog.cpp \
    xmlmessage.cpp \
    chathelper.cpp \
    messagelog.cpp \
    updatewindow.cpp \
    webnetwork.cpp \
    userinfowindow.cpp \
    chatroomwindow.cpp \
    userselectdialog.cpp \
    subcontrols.cpp \
    filemessagingproc.cpp \
    themedbutton.cpp \
    themedcombobox.cpp \
    thememanager.cpp \
    loggermanager.cpp \
    imageslist.cpp \
    globals.cpp \
    qxttooltip.cpp \
    stdlocation.cpp \
    instantmessagewindow.cpp

HEADERS  += \
    usertreewidget.h \
    uidefinitions.h \
    udpnetwork.h \
    transferwindow.h \
    transferlistview.h \
    tcpnetwork.h \
    strings.h \
    soundplayer.h \
    shared.h \
    settingsdialog.h \
    settings.h \
    resource.h \
    network.h \
    netstreamer.h \
    messaging.h \
    message.h \
    mainwindow.h \
    lmc.h \
    imagepickeraction.h \
    imagepicker.h \
    historywindow.h \
    historytreewidget.h \
    helpwindow.h \
    filemodelview.h \
    chatdefinitions.h \
    broadcastwindow.h \
    history.h \
    stdlocation.h \
    definitions.h \
    datagram.h \
    aboutdialog.h \
    xmlmessage.h \
    chathelper.h \
    messagelog.h \
    updatewindow.h \
    webnetwork.h \
    userinfowindow.h \
    chatroomwindow.h \
    userselectdialog.h \
    subcontrols.h \
    themedbutton.h \
    themedcombobox.h \
    thememanager.h \
    loggermanager.h \
    imageslist.h \
    globals.h \
    qxttooltip.h \
    qxttooltip_p.h \
    instantmessagewindow.h

FORMS += \
    transferwindow.ui \
    settingsdialog.ui \
    mainwindow.ui \
    historywindow.ui \
    helpwindow.ui \
    broadcastwindow.ui \
    aboutdialog.ui \
    updatewindow.ui \
    userinfowindow.ui \
    chatroomwindow.ui \
    userselectdialog.ui \
    instantmessagewindow.ui

TRANSLATIONS += \
        en_US.ts \
        ml_IN.ts \
        fr_FR.ts \
        de_DE.ts \
        tr_TR.ts \
        es_ES.ts \
        ko_KR.ts \
        bg_BG.ts \
        ro_RO.ts \
        ar_SA.ts \
        sl_SI.ts \
        pt_BR.ts \
        ru_RU.ts \
        it_IT.ts

win32: RC_FILE = lmcwin32.rc
macx: ICON = lmc.icns

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
} else {
    DESTDIR = ../release
}

win32: CONFIG(release, debug|release): LIBS += -L$$PWD/../../lmcapp/lib/ -llmcapp2
else:win32: CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lmcapp/lib/ -llmcappd2
unix:!symbian: LIBS += -L$$PWD/../../lmcapp/lib/ -llmcapp

INCLUDEPATH += $$PWD/../../lmcapp/include
DEPENDPATH += $$PWD/../../lmcapp/include

#win32: LIBS += -L$$PWD/../../openssl/lib/ -llibeay32
unix:!symbian: LIBS += -L$$PWD/../../openssl/lib/ -lcrypto

INCLUDEPATH += $$PWD/../../openssl/include
DEPENDPATH += $$PWD/../../openssl/include

win32:LIBS += -liphlpapi
