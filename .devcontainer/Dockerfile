FROM mcr.microsoft.com/vscode/devcontainers/cpp:ubuntu

RUN apt update && apt upgrade -y
RUN apt install clangd-12 bear -y
RUN update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-12 100
RUN apt install nginx -y

COPY www/ /var/www/html/

EXPOSE 4243
