TARGET = tst_$$basename(_PRO_FILE_PWD_)
CONFIG += testcase
QT += testlib
QT -= gui

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
CONFIG += warn_on
win32-msvc*:QMAKE_CXXFLAGS_WARN_ON += /WX
else:       QMAKE_CXXFLAGS_WARN_ON += -Werror

SOURCES += $${TARGET}.cpp

$$(ENABLE_COVERAGE) {
  message(Enabling test coverage reporting [$$basename(_PRO_FILE_)])
  CONFIG += gcov
  QMAKE_CXXFLAGS_RELEASE -= -O1 -O2 -O3
}
