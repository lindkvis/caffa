@echo off
echo Generate CA key:
openssl genrsa -out ca.key 4096

echo Generate CA certificate:
openssl req -new -x509 -days 365 -key ca.key -out ca.crt -subj  "/C=NO/ST=Trondelag/L=Trondheim/O=YourCompany/OU=YourApp/CN=localhost"

echo Generate server key:
openssl genrsa -out server.key 4096

echo Generate server signing request:
openssl req -new -key server.key -out server.csr -subj  "/C=NO/ST=Trondelag/L=Trondheim/O=YourCompany/OU=YourApp/CN=localhost"

echo Self-sign server certificate:
openssl x509 -req -days 365 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt

echo Generate client key
openssl genrsa -out client.key 4096

echo Generate client signing request:
openssl req -new -key client.key -out client.csr -subj  "/C=NO/ST=Trondelag/L=Trondheim/O=YourCompany/OU=YourApp/CN=localhost"

echo Self-sign client certificate:
openssl x509 -req -days 365 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt

