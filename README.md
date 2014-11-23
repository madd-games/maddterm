maddterm
========

The Madd Games Terminal Library (`maddtech`) is a UNIX library written in C for easy creation and manipulation of virtual terminals. It emulates the `rxvt` terminal, and allows you to start a process in such a terminal, and control it by passing events and reading the terminal matrix. It has no input or render system of its own, and therefore allows you to implement them.

It stores the terminal matrix in a location in RAM that you specified, in a VGA-framebuffer-like format.
