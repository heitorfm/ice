ICE-1.0.0
realm >= 1.0.0

@server {
    host: "127.0.0.1"
    port: 8080
    ssl: true
    timeout: 30
}

@database {
    type: "postgresql"
    host: "db.example.com"
    port: 5432
    name: "myapp"
    max_connections: 100
}

@logging {
    level: "info"
    file: "/var/log/app.log"
    max_size: "10MB"
    rotate: true
}