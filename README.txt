PortMIDI is a cross platform (Windows, macOS, Linux, and BSDs
which support alsalib) library for interfacing with operating
systems' MIDI I/O APIs. It was started by Ross Bencina and
Phil Burk, then maintained by Roger B. Dannenberg, and now
maintained by the Mixxx DJ Software team.

---------------------- BUILDING ----------------------------
PortMidi has no dependencies on Windows and macOS other than
the operating system APIs. On Linux and BSD, the ALSA library
development headers must be installed. On Debian and derived
distributions, that is the libasound2-dev package. On Fedora
and derived distributions, that is the alsa-lib-devel package.

PortMidi uses the CMake build system. To build it, run:

$ cmake -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install/to -S . -B build
$ cmake --build build --parallel number-of-cpu-cores
$ cmake --install build

---------- some notes on the design of PortMidi ----------

POINTERS VS DEVICE NUMBERS

When you open a MIDI port, PortMidi allocates a structure to
maintain the state of the open device. Since every device is
also listed in a table, you might think it would be simpler to
use the table index rather than a pointer to identify a device.
This would also help with error checking (it's hard to make
sure a pointer is valid). PortMidi's design parallels that of
PortAudio.

ERROR HANDLING

Error handling turned out to be much more complicated than expected.
PortMidi functions return error codes that the caller can check.
In addition, errors may occur asynchronously due to MIDI input.
However, for Windows, there are virtually no errors that can
occur if the code is correct and not passing bogus values. One
exception is an error that the system is out of memory, but my
guess is that one is unlikely to recover gracefully from that.
Therefore, all errors in callbacks are guarded by assert(), which
means not guarded at all in release configurations.

Ordinarily, the caller checks for an error code. If the error is
system-dependent, pmHostError is returned and the caller can
call Pm_GetHostErrorText to get a text description of the error.

Host error codes are system-specific and are recorded in the
system-specific data allocated for each open MIDI port.
However, if an error occurs on open or close,
we cannot store the error with the device because there will be
no device data (assuming PortMidi cleans up after devices that
are not open). For open and close, we will convert the error
to text, copy it to a global string, and set pm_hosterror, a
global flag.

Similarly, whenever a Read or Write operation returns pmHostError,
the corresponding error string is copied to a global string
and pm_hosterror is set. This makes getting error strings
simple and uniform, although it does cost a string copy and some
overhead even if the user does not want to look at the error data.

The system-specific Read, Write, Poll, etc. implementations should
check for asynchronous errors and return immediately if one is
found so that these get reported. This happens in the Mac OS X
code, where lots of things are happening in callbacks, but again,
in Windows, there are no error codes recorded in callbacks.

DEBUGGING

Debugging messages can be enabled by passing `-DDEBUGMESSAGES=ON`
to the CMake configure step.

If you are building a console application for research, we suggest
compiling with the option PM_CHECK_ERRORS. This will insert a
check for error return values at the end of each PortMidi
function. If an error is encountered, a text message is printed
using printf(), the user is asked to type ENTER, and then exit(-1)
is called to clean up and terminate the program.

You should not use PM_CHECK_ERRORS if printf() does not work
(e.g. this is not a console application under Windows, or there
is no visible console on some other OS), and you should not use
PM_CHECK_ERRORS if you intend to recover from errors rather than
abruptly terminate the program.
