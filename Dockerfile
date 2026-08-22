FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    g++ make cmake \
    libjsoncpp-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Client/ ./Client/
COPY netlayer/ ./netlayer/
COPY protocal/ ./protocal/
COPY server/Router.cc server/Router.h ./server/
COPY BusinessLogic/ ./BusinessLogic/
RUN cd Client && rm -rf build && cmake -B build . && make -C build

ENTRYPOINT ["/app/Client/build/chat_client"]
