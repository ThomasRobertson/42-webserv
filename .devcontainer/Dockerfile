FROM mcr.microsoft.com/devcontainers/cpp:1-debian-11

RUN apt update && apt upgrade -y
RUN apt install nginx -y

COPY www/ /var/www/html/

EXPOSE 4243
