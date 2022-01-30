TEMPLATE = lib
CONFIG *= plugin
CONFIG *= warn_on
CONFIG *= debug_and_release

QT *= widgets
QT *= concurrent

INCLUDEPATH += ../../qoobar_app
DEPENDPATH += ../../qoobar_app

BUILD_DIR =
CONFIG(release, debug|release):BUILD_DIR = ../../../release
CONFIG(debug, debug|release):BUILD_DIR = ../../../debug

DESTDIR = $${BUILD_DIR}/plugins
mac {
    target.path = $${BUILD_DIR}/qoobar.app/Contents/PlugIns
    INSTALLS += target
}

unix {
!mac {
    PLUGINS_PATH = $${INSTALL_ROOT}/lib/qoobar/plugins
    target.path = $$PLUGINS_PATH
    INSTALLS += target
}
}
