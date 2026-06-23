FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        lemon \
        make \
        re2c \
        valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

COPY . .

CMD ["make", "valgrind"]
