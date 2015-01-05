=============
asetniop-term
=============

An `asetniop`_ keyboard emulator for linux terminals.

.. _asetniop: http://asetniop.com

Build Instructions
==================

``asetniop-term`` is built with ``make``. It accepts the device node of the keyboard as its first argument, e.g.::

    > make
    # ./asetniop.exe /dev/input/by-path/*-kbd

Either run ``asetniop-term`` as root, or make sure your user has read permissions on the device node (the absolute device node, not the symlink).

Debugging
---------

To debug, set the ``DEBUG`` env var to:
 * ``DEBUG`` to show device acquisition
 * ``DEBUG_EVENT`` to show keydown and keyup events
 * ``DEBUG_STATE`` to show the asetniop state

like so::

    > DEBUG=DEBUG_STATE make

Disabling xinput
================

``asetniop-term`` reads keyboard events, but does not prevent them being read by other processes, like X11. To disable the keyboard under X11 (make sure you have some other way to control the system!) - find the device id like so::

    > xinput
    ⎡ Virtual core pointer                      id=2    [master pointer  (3)]
    ⎜ ↳ ...
    ⎣ Virtual core keyboard                     id=3    [master keyboard (2)]
        ↳ ...
        ↳ AT Translated Set 2 keyboard              id=10   [slave  keyboard (3)]

Then disable the device, e.g.::

    > xinput disable 10
