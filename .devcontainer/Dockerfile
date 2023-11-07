FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
clang \
build-essential \
&& \
apt-get clean && \
rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
clangd \
vim \
git \
curl \
zsh \
llvm \
bear \
nginx \
siege \
&& \
apt-get clean && \
rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get upgrade -y && \
apt-get clean && \
rm -rf /var/lib/apt/lists/*

# RUN update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-12 100

RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

RUN chsh -s $(which zsh)

COPY www/ /var/www/html/
