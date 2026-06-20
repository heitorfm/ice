ICE-0.0.1
sampleHttpServer >= 1.0.0

# HTTP server configuration with nested sections and object references.
@http {
    name: "public-api"
    environment: "production"

    # server1 derives its port from server0 using a typed reference inside math.
    server0: {port: 8080}
    server1: {port: ${server0.port} + 1}

    # Runtime listener settings.
    @server {
        host: "0.0.0.0"
        port: 8080
        workers: 4
        requestTimeoutMs: 30000
        keepAlive: true
    }

    # TLS assets and allowed protocol versions.
    @tls {
        enabled: true
        certificate: "/etc/ice/certs/server.crt"
        privateKey: "/etc/ice/certs/server.key"
        protocols: ["TLSv1.2", "TLSv1.3"]
    }

    # Request limits and rate limiting policy.
    @limits {
        maxBodyMb: 25
        maxHeaders: 64
        rateLimit: {
            enabled: true,
            requests: 1000,
            windowSeconds: 60
        }
    }

    # Logging output for access and application events.
    @logging {
        level: "info"
        accessLog: true
        format: "json"
    }
}
