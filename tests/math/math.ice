ICE-1.0.0
sample >= 1.0.0

@math {
    basePort: 8080
    server: {port: 9000}
    precedence: 2 + 3 * 4
    grouped: (2 + 3) * 4
    exponent: 2 ** 3
    modulo: 10 % 3
    division: 5 / 2
    refMath: ${basePort} + 1
    objectPathMath: ${server.port} + 1
    arrayMath: [1 + 2, ${basePort} + 2]
    objectMath: {port: ${basePort} + 3}
}
