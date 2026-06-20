ICE-1.0.0
sample >= 1.0.0

@build {
    src: "src/main.cpp"
    flags: ["-O2", "-Wall"]
    port: 8080
    compile: build->compile(${src}, ${flags}, {port: ${port} + 1})
    emptyCall: build->clean()
}
