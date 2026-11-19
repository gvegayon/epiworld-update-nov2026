# epiworld 1.0: an update (2026-11-19)

Slides (Quarto, revealjs) introducing the epiworld 1.0 API: contact layers,
natural histories (stages and statuses), and several diseases per model.
About 30 minutes.

- `index.qmd`: the slides. Comments in the file (`<!-- -->`) hold notes and
  sources; there are no speaker notes.
- `examples/*.cpp`: the runs behind the plots of the "New capabilities"
  part, built against `vendor/epiworld.hpp` (the `v1-layers` amalgam; the
  commit is in `vendor/VERSION`). Each writes CSVs to `results/`.
- `styles.scss`: the theme.

## Building

```sh
make results   # compile and run the examples (in the epiworld devcontainer
               # image, localhost/epiworld-dev, if podman or docker is there)
make slides    # quarto render index.qmd -> index.html
make all       # both
```

The slides need R with ggplot2 and data.table.

## Devcontainer

`.devcontainer/` defines the environment: g++ for the examples, R with
epiworldR 1.0 (the `EPIWORLDR_REF` build argument, `v1-layers` for now),
Quarto and a headless Chromium for the diagrams. Open the folder in VS Code
("Reopen in Container"), or build the image the Makefile uses:

```sh
make image     # podman/docker build -> localhost/epiworld-dev
```

`make results` runs the examples in that image. To render the slides in it as
well (no Chrome needed on the host):

```sh
podman run --rm -v "$PWD":"$PWD" -w "$PWD" localhost/epiworld-dev quarto render index.qmd
```

To update epiworld, replace `vendor/epiworld.hpp` with the amalgam
(`./epiworld.hpp`) of a newer `v1-layers` commit, update `vendor/VERSION`, and
run `make all`.
