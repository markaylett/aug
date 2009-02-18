#!/bin/sh

# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

set -e

# Country Name (2 letter code) [AU]:GB
# State or Province Name (full name) [Some-State]:Surrey
# Locality Name (eg, city) []:Camberley
# Organization Name (eg, company) [Internet Widgits Pty Ltd]:Dummy Company
# Organizational Unit Name (eg, section) []:IT
# Common Name (eg, YOUR name) []:CA Person
# Email Address []:ca@dummy

echo '-------------------------'
echo 'new certificate authority'
echo '-------------------------'

rm -fR demoCA
CA.pl -newca

# Enter PEM pass phrase: test
# Common Name (eg, YOUR name) []:CA Person
# Email Address []:ca@dummy

echo '--------------------------'
echo 'server certificate request'
echo '--------------------------'

CA.pl -newreq

# Enter PEM pass phrase: test
# Common Name (eg, YOUR name) []:Server System
# Email Address []:server@dummy

echo '-----------------------'
echo 'sign server certificate'
echo '-----------------------'

CA.pl -sign

mv newcert.pem servercert.pem
mv newkey.pem serverkey.pem
mv newreq.pem serverreq.pem

echo '--------------------------'
echo 'client certificate request'
echo '--------------------------'

CA.pl -newreq

# Enter PEM pass phrase: test
# Common Name (eg, YOUR name) []:Client System
# Email Address []:client@dummy

echo '-----------------------'
echo 'sign client certificate'
echo '-----------------------'

CA.pl -sign

mv newcert.pem clientcert.pem
mv newkey.pem clientkey.pem
mv newreq.pem clientreq.pem

cp demoCA/cacert.pem ./
