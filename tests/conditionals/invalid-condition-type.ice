ICE-1.0.0
sample >= 1.0.0

@build {
    flags: ["-Wall"]
    IF (${flags}) {
        enabled: true
    }
}
