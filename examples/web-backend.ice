ICE-0.0.1
productionApp >= 1.0.0

# Larger backend example with shared values, nested objects, arrays, and interpolation.
@backend {
    name: "orders-api"
    version: "2.4.0"
    environment: "staging"
    region: "us-east-1"
    serviceUrl: "https://api-staging.example.com"

    # The public URL is copied from a section-level property.
    @http {
        host: "0.0.0.0"
        port: 8080
        publicUrl: "${serviceUrl}"
        compression: true
        requestTimeoutMs: 25000
        gracefulShutdownMs: 15000
    }

    # Database connection and pool sizing.
    @database {
        driver: "postgres"
        host: "db-staging.internal"
        port: 5432
        database: "orders"
        user: "orders_app"
        ssl: true
        pool: {
            min: 4,
            max: 24,
            idleTimeoutMs: 30000,
            connectTimeoutMs: 5000
        }
    }

    # Cache keys are grouped in an object so consumers can read them by name.
    @cache {
        provider: "redis"
        host: "redis-staging.internal"
        port: 6379
        database: 2
        ttlSeconds: 900
        keys: {
            order: "orders/:id",
            customer: "customers/:id",
            inventory: "inventory/:sku"
        }
    }

    # Queue URLs are represented as an array of objects.
    @queue {
        provider: "sqs"
        awsRegion: "${region}"
        visibilityTimeoutSeconds: 45
        queues: [
            {
                name: "orders-created",
                url: "https://sqs.us-east-1.amazonaws.com/123/orders-created"
            },
            {
                name: "orders-paid",
                url: "https://sqs.us-east-1.amazonaws.com/123/orders-paid"
            }
        ]
    }

    # Authentication metadata used by the service middleware.
    @auth {
        issuer: "https://auth-staging.example.com"
        audience: "orders-api"
        jwksUrl: "https://auth-staging.example.com/.well-known/jwks.json"
        tokenLeewaySeconds: 30
        roles: ["admin", "support", "service"]
    }

    # Feature toggles can mix booleans and resolved math values.
    @features {
        asyncPaymentCapture: true
        newCheckoutFlow: true
        auditTrail: true
        recommendations: false
        maxItemsPerOrder: 100 + 50
    }
}
