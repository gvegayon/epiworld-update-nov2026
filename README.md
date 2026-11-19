# epiworld 1.0: an update (2026-11-19)

Slides (Quarto, revealjs) introducing the epiworld 1.0 API: contact layers,
natural histories (stages and statuses), and several diseases per model.
About 30 minutes.

- `index.qmd`: the slides. Comments in the file (`<!-- -->`) hold notes and
  sources; there are no speaker notes.
  The four examples of the "New capabilities" part are R (epiworldR 1.0), run
  when the slides render: each has a slide with the code, a slide with its
  output (a `summary()` and a plot), and a link to its C++ equivalent in the
  appendix (and a `<go back>` link there).
- `examples/*.cpp`: those four examples in C++, built against
  `vendor/epiworld.hpp` (the `v1-layers` amalgam; the commit is in
  `vendor/VERSION`). Each writes CSVs to `results/`. The appendix shows the
  parts between `// slide-begin` and `// slide-end`, read from these files.
- `bench/summarize.R`: the numbers of the benchmark slides
  (`results/benchmark_*.csv`), from the `v1-layers` branch of
  epiworld-benchmark, with that repository's own definitions of time,
  memory and model lines. `make benchmark` refreshes them.
- `fig/diagrams/`: hand-drawn SVG diagrams (self-contained, white
  background): `epiworld-day.svg` (a run, day by day) and
  `epiworld-infection.svg` (where an infection comes from), also proposed
  for the epiworld docs (UofUEpiBio/epiworld#385); `epiworld-why-1.0.svg`
  (0.18 against 1.0), `epiworld-roadmap.svg` (the phases),
  `epiworld-transitions.svg` (the four kinds of transitions) and
  `epiworld-models.svg` (0.18 models onto 1.0 models plus a layer).
- `styles.scss`: the theme.

## Building

```sh
make results   # compile and run the examples (in the epiworld devcontainer
               # image, localhost/epiworld-dev, if podman or docker is there)
make slides    # quarto render index.qmd -> index.html
make slides-container  # the same, always in the image (an error if none)
make all       # both
```

The slides need R with epiworldR 1.0, ggplot2 and data.table; the
devcontainer image has them. `make slides` renders in the image when
podman or docker is there.

## Devcontainer

`.devcontainer/` defines the environment: g++ for the examples, R with
epiworldR 1.0 (the `EPIWORLDR_REF` build argument, `v1-layers` for now),
Quarto and a headless Chromium for the diagrams. Open the folder in VS Code
("Reopen in Container"), or build the image the Makefile uses:

```sh
make image     # podman/docker build -> localhost/epiworld-dev
```

`make results` runs the examples in that image. `make slides` renders the slides
in it too (no R, Quarto or Chrome needed on the host).

To update epiworld, replace `vendor/epiworld.hpp` with the amalgam
(`./epiworld.hpp`) of a newer `v1-layers` commit, update `vendor/VERSION`, and
run `make all`.
