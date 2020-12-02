FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive
ENV COCKATRICE_RELEASE_DATE=2020-08-23-Release-2.7.5
ENV COCKATRICE_RELEASE_NAME=Cockatrice-Blessed_Sanctuary-2.7.5-UbuntuBionic.deb

RUN apt-get update -y --no-install-recommends \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        gnupg \
    && curl -LO  https://github.com/Cockatrice/Cockatrice/releases/download/${COCKATRICE_RELEASE_DATE}/${COCKATRICE_RELEASE_NAME} \
    && apt-get install -y \
        libqt5multimedia5-plugins \
        libqt5svg5 \
        libprotobuf10 \
        libqt5core5a \
        libqt5gui5 \
        libqt5multimedia5 \
        libqt5network5 \
        libqt5printsupport5 \
        libqt5sql5 \
        libqt5websockets5 \
        libqt5widgets5 \
    && dpkg -i ./${COCKATRICE_RELEASE_NAME} \
    && apt-get install -f \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

CMD cockatrice