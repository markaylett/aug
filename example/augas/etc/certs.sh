#!/bin/sh

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
