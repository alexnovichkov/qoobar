TEMPLATE = lib
CONFIG *= plugin
CONFIG *= warn_on
CONFIG *= debug_and_release

greaterThan(QT_MAJOR_VERSION, 4) {
  QT *= widgets
  QT *= concurrent
  DEFINES *= HAVE_QT5
}

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
    PLUGINS_PATH = /usr/lib/qoobar/plugins
    target.path = $$PLUGINS_PATH
    INSTALLS += target
}
}
