ICE-1.0.0
sample >= 1.0.0

@build {
    target: "dist/app"
    clean: "dist" |> fs->remove() |> fs->mkdir()
    package: ${target} |> build->strip() |> build->archive("dist/app.tar")
    nested: {
        task: "dist/app" |> build->checksum()
    }
    tasks: ["dist/app" |> build->strip()]
}
