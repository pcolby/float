# float

A simple utility for exporting body weight from Fitbit to Polar Flow. Useful it, for example, you
use Fitbit Aria scales, and want Polar to be updated when you do so.

## Usage

```
Usage: float [options]
Export weight from Fitbit to Polar Flow

Options:
  -h, --help                    Displays this help.
  -c, --credentials <filename>  Read credentials from filename
  -d, --debug                   Enable debug output
  --no-color                    Do not color the output
  --show                        Show the web view on screen
  -v, --version                 Displays version information.
```

### Credentials

Of course, the application needs credentials to access both Fitbit and Polar Flow. These may be
provide either by file, or environment variables.

#### Credentials File

The Fitbit and Polar Flow credentials may be given in an ini file, specified by the `-c` option:

```
float -c path/to/credentials.ini
```

Where the ini file looks something like:

```
[Fitbit]
username=user@example.com
password=my$trongPa$$word!!

[Polar]
username=user@example.com
password=myOtherPa$$word##
```

#### Environment Variables

If no `-c` (or `--credentials`) option is given, then application will look for the following
environment variables instead: `FITBIT_USERNAME`, `FITBIT_PASSWORD`, `POLAR_USERNAME`, and
`POLAR_PASSWORD`.

## Building

To build the application from source code, clone the repository, then:

```
mkdir -p '/path/to/tmp/build/dir'
qmake -o '/path/to/tmp/build/dir' -r -Wall -Wlogic -Wparser CONFIG+=debug_and_release '/path/to/repo/clone'
make -C '/path/to/tmp/build/dir' all
make -C '/path/to/tmp/build/dir' check
```

## Debugging

For basic debugging, use the `-d` or `--debug` flags.

```
$ ./float -d ...
```

To see the web view rendered, either natively (on your local Windows/OSX/Linux desktop) or remotely (eg via X11
forwarding over SSH), use the `--show` option.  Otherwise the view will be rendered offscreen only.

```
$ ./float --show ...
```

To enable QtWebEngine's Chromium debugging, set the `QTWEBENGINE_CHROMIUM_FLAGS` environment variable.

```
QTWEBENGINE_CHROMIUM_FLAGS="--enable-logging --log-level=0 --v=1"
```

And for advanced debugging, you can enable Chromium's developer tools interface via the
`QTWEBENGINE_REMOTE_DEBUGGING` environment variable.

```
QTWEBENGINE_REMOTE_DEBUGGING=9001
```

Then visit http://127.0.0.1:9001/ in a Chromium-based browser (such as Google Chrome).

All of the above debug options can be combined too.

```
QTWEBENGINE_CHROMIUM_FLAGS="--enable-logging --log-level=0 --v=1" \
QTWEBENGINE_REMOTE_DEBUGGING=9001 \
QT_LOGGING_RULES="*=true"
./float -d ...
```
