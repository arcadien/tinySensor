FROM php:7.4-apache

RUN apt update
RUN apt install -y gcc-avr avrdude cmake avr-libc git
RUN mkdir /var/lib/tinysensor
RUN cd /var/lib/tinysensor && git clone https://github.com/arcadien/tinySensor.git --recurse-submodules

COPY web/index.html /var/www/html
COPY web/build.php  /var/www/html
