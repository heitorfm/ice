ICE-1.0.0
sample >= 1.0.0

@build {
    mode: "debug"
    platform: "linux"
    debugEnabled: true
    basePort: 8080
    compilerFlags: ["-Wall"]

    IF (${mode} == "debug") {
        debugSymbols: true
        compilerFlags: ["-g", "-O0", "-Wall"]
    }

    IF (${mode} == "release") {
        releaseOnly: ${doesNotExist.value}
    }

    IF (${platform} == "linux" AND ${debugEnabled}) {
        linkerFlags: ["-pthread"]
    }

    IF (${basePort} + 1 == 8081) {
        adminPort: ${basePort} + 1
    }

    compile: build->compile(${compilerFlags})
}
