## Building

Clone the source code, then:

```
mkdir -p '/path/to/tmp/build/dir'
qmake -o '/path/to/tmp/build/dir' -r -Wall -Wlogic -Wparser CONFIG+=debug_and_release '/path/to/repo/clone'
make -C '/path/to/tmp/build/dir' all
make -C '/path/to/tmp/build/dir' check
```

## Debugging

For basic debugging, use the `-d` or `--debug` flags.

```
$ ./src/release/flow -d ...
```

To see the web view rendered, either natively (on your local Windows/OSX/Linux desktop) or remotely (eg via X11
forwarding over SSH), use the `--show` option.  Otherwise the view will be rendered offscreen only.

```
$ ./flow --show ...
```

To enable QtWebEngine's Chromium debugging, set the `QTWEBENGINE_CHROMIUM_FLAGS` environment variable.

```
QTWEBENGINE_CHROMIUM_FLAGS="--enable-logging --log-level=0 --v=1"
```

And for advanced debugging, you can enable Chromium's developer tools interface via the `QTWEBENGINE_REMOTE_DEBUGGING`
environment variable.

```
QTWEBENGINE_REMOTE_DEBUGGING=9001
```

Then visit http://127.0.0.1:9001/ in a Chromium-based browser (such as Google Chrome).

All of the above debug options have be combined too.

```
QTWEBENGINE_CHROMIUM_FLAGS="--enable-logging --log-level=0 --v=1" \
QTWEBENGINE_REMOTE_DEBUGGING=9001 \
QT_LOGGING_RULES="*=true"
./src/release/flow -d ...
```
