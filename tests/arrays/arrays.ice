ICE-1.0.0
sample >= 1.0.0

@arrays {
    basePort: 8080
    empty: []
    strings: ["a", "b"]
    numbers: [1, 2.5]
    mixed: [true, null, "x"]
    objects: [{name: "api"}, {name: "worker"}]
    refs: [${basePort}, ${basePort} + 1]
}
