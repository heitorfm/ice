ICE-1.0.0
sample >= 1.0.0

@app {
    name: "api"
    ports: [8080, 8081]
    server: {
        host: "127.0.0.1",
        port: 8080
    }

    @runtime {
        threads: 4
    }
}
