ICE-1.0.0
realm >= 1.0.0

# Build configuration showing pipelines and callable descriptors.
@build {
    project: "ice-demo"
    mode: "release"
    platform: "linux"
    sourceDir: "src"
    outputDir: "dist"
    compilerFlags: ["-Wall", "-std=c++17"]

    # Conditional blocks are resolved before references, math, and the public document.
    IF (${mode} == "release") {
        optimization: "speed"
        compilerFlags: ["-O2", "-Wall", "-std=c++17"]
    }

    IF (${mode} == "debug") {
        optimization: "none"
        compilerFlags: ["-g", "-O0", "-Wall", "-std=c++17"]
    }

    IF (${platform} == "linux") {
        linkerFlags: ["-pthread"]
    }

    # Pipelines describe ordered host operations; the parser does not execute them.
    clean: "dist" |> fs->remove() |> fs->mkdir()

    # Function calls are preserved as descriptors for the build tool runtime.
    compile: build->compile(
        "src/main.cpp",
        "dist/app",
        ${compilerFlags}
    )

    # The output binary is transformed before being archived.
    package: "dist/app" |> build->strip() |> build->archive("dist/app.tar")

    # Execution metadata for the host build system.
    @steps {
        order: ["clean", "compile", "package"]
        parallel: false
        retries: 1
    }

    # Files produced by the build.
    @artifacts {
        binary: "dist/app"
        archive: "dist/app.tar"
        checksum: "dist/app.sha256"
    }
}
