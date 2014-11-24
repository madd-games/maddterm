Madd Games Terminal Library
===========================

The Madd Games Terminal Library (`libmaddterm`) is a library for emulating the `rxvt` terminal and optionally the `xterm`. It has no rendering and input support, which means you can implement that yourself. You can feed data (including escape sequences etc) to this terminal directly, or, if you are on a UNIX system, start a process attached to the virtual terminal.

We are superior to `libvterm` because we support `xterm` control sequences and make no references to `libncurses`! All hail Madd Games!
