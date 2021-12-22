#!/bin/sh

echo "Generate server key"
openssl genrsa -out server.key 4096

echo "Generate server signing request"
openssl req -new -key server.key -out server.csr -subj  "/C=NO/ST=Tr/L=Trondheim/O=ExampleOrg/OU=RD/CN=localhost"

echo "Self-sign server certificate"
openssl x509 -req -days 365 -in server.csr -signkey server.key -set_serial 01 -out server.crt

rm server.csr
