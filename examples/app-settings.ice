ICE-1.0.0
corpApp >= 1.0.0

#  application settings with math and string interpolation.
@app {
    name: "worker"
    environment: "local"

    # Math expressions are resolved before the public document is built.
    retries: 2 + 1
    tempDir: "/tmp/worker"

    # Runtime knobs for a local worker process.
    @runtime {
        threads: 4
        debug: true
    }

    # Interpolated strings expand tempDir without preserving the source quotes.
    @paths {
        input: "${tempDir}/input"
        output: "${tempDir}/output"
    }
}
