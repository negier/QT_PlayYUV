include(openglwindow.pri)

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target
QT += opengl network

HEADERS +=

CONFIG += c++11
