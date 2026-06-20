ICE-1.0.0
sample >= 1.0.0

@global {
    value: "from-global"
}

@config {
    value: "from-parent"
    server: {
        host: "127.0.0.1",
        nested: {
            port: 8080
        }
    }

    @child {
        local: "from-child"
        sameScope: ${local}
        parentScope: ${value}
        globalScope: ${global.value}
        host: ${server.host}
        nestedPort: ${server.nested.port}
        missingValue: ${notFound}
    }
}
