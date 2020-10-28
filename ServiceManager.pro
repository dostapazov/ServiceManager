QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	qwinservicemanager.cpp


HEADERS += \
	mainwindow.h \
	qwinservicemanager.h

FORMS += \
	mainwindow.ui

CONFIG += embed_manifest_exe

CONFIG(release, debug|release){
	QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:"level='requireAdministrator' "
}
else{
	QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:"level='asInvoker' "
}

#CONFIG(debug, debug|release)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
