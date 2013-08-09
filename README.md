mrtgdf
======

A caching disk usage monitor that outputs in the format required by
MRTG.  SNMP doesn't seem to be able to provide a stable identity for
removable disks, hence this program.

Installation
------------

If you got it from git:

    autoreconf -is

In any case:

    fakeroot debian/rules clean build binary
    sudo dpkg -i ../mrtgdf_*.deb

or:

    ./configure
    make
    sudo make install

Documentation
-------------

man mrtgdf, or UTSL.

Copyright
---------

Copyright © 2013 Richard Kettlewell

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
02110-1301, USA.
