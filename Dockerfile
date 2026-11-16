FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y clang && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY . .

RUN sh tools/build_server.sh

ENV PORT=8080
EXPOSE 8080

CMD ["sh", "-c", "./build/playme_server $PORT"]
