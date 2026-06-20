ICE-1.0.0
sample >= 1.0.0

@global {
    suffix: "global"
}

@app {
    name: "api"
    port: 8080
    enabled: true
    nothing: null
    message: "service ${name} on ${port}"
    boolText: "${enabled}"
    nullText: "${nothing}"
    globalText: "${global.suffix}"
}
