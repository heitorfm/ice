ICE-1.0.0
corpApp >= 1.0.0

# Small feature flag document for product rollout configuration.
@features {
    defaultState: false
    rolloutPercent: 25

    # Individual flags override the default state.
    flags: {
        searchV2: true,
        billingPortal: false,
        compactDashboard: true
    }

    # Teams that can edit or approve these flags.
    owners: ["platform", "growth", "billing"]
}
