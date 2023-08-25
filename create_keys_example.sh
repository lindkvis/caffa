#!/bin/sh
openssl dhparam -out build/dh.pem 2048
openssl req -newkey rsa:2048 -nodes -keyout build/key.pem -x509 -days 10000 -out build/cert.pem -subj "/C=NO/ST=Trondelag/L=Trondheim/O=YourCompany/OU=YourApp/CN=www.kontur.tech"