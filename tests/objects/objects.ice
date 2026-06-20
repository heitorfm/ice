ICE-1.0.0
sample >= 1.0.0

@objects {
    basePort: 8080
    empty: {}
    single: {name: "api"}
    nested: {
        server: {
            host: "127.0.0.1",
            port: ${basePort} + 1
        },
        protocols: ["http", "https"]
    }
}
