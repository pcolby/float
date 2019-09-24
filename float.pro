TEMPLATE = subdirs
SUBDIRS += src test

$$(ENABLE_COVERAGE) {
  message(Enabling test coverage reporting [$$basename(_PRO_FILE_)])
  coverage.commands += lcov --capture --directory . --output-file coverage.info &&
  coverage.commands += lcov --remove coverage.info \
                       $$shell_quote(*/test/*) \
                       $$shell_quote(/usr/*) \
                       $$shell_quote(*/QtCore/*) \
                       $$shell_quote(*/QtTest/*) \
                       --output-file coverage.info &&
  coverage.commands += genhtml --demangle-cpp --output-directory coverage_html coverage.info
  QMAKE_CLEAN += coverage.info coverage_html/*.html
  QMAKE_EXTRA_TARGETS += coverage
}
