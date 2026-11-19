# Summarizes the epiworld-benchmark results for the slides: median simulation
# time and memory by engine, scenario and population size, and the model lines
# of every engine in every scenario. Reads a git ref of the benchmark
# repository (its v1-layers branch has the epiworld 1.0 runs) without checking
# it out, and uses the benchmark's own definitions (report/common.R).
#
#   Rscript bench/summarize.R [benchmark repo] [git ref]
#
# Writes results/benchmark_runs.csv, results/benchmark_lines.csv and
# results/benchmark_meta.csv.
library(data.table)

args  <- commandArgs(trailingOnly = TRUE)
repo  <- if (length(args) > 0) args[1] else "~/Documents/software/epiworld-benchmark"
ref   <- if (length(args) > 1) args[2] else "origin/v1-layers"
repo  <- normalizePath(repo, mustWork = TRUE)

# The ref's files, in a temporary folder
root <- file.path(tempdir(), "epiworld-benchmark")
dir.create(root, showWarnings = FALSE)
scenarios <- sprintf("scenario_%02d", 0:4)
paths     <- c(
  "report/common.R",
  "results/results.csv",
  file.path(scenarios, "code_regions.yml"),
  file.path(scenarios, "runners")
)
status <- system2(
  "sh",
  c(
    "-c",
    shQuote(sprintf(
      "git -C %s archive %s %s | tar -x -C %s",
      shQuote(repo), shQuote(ref), paste(shQuote(paths), collapse = " "),
      shQuote(root)
    ))
  )
)
if (status != 0)
  stop("Could not read ", paths, " from ", ref, " in ", repo)

source(file.path(root, "report/common.R"))

# At the v1-layers tip, code_regions.yml still anchors the epiworld-v1 runner
# at `Model<> model(n);`, but the runner now says `Model model(n);` (1.0 has no
# template parameter); fix the anchor in the temporary copy only
for (s in scenarios) {
  f <- file.path(root, s, "code_regions.yml")
  writeLines(
    gsub("Model<> model\\(n\\);", "Model model\\(n\\);", readLines(f), fixed = TRUE),
    f
  )
}

# Runs: simulation and build time, memory (the benchmark's measures)
# Byte counts as doubles: integer64 breaks pmax() in memory_measures()
full <- fread(file.path(root, "results/results.csv"), integer64 = "double")
mem  <- as.data.table(memory_measures(as.data.frame(full)))
runs <- data.table(
  engine   = full$engine,
  scenario = full$scenario,
  n        = full$n,
  simulate = full$simulate_seconds,
  build    = full$setup_seconds - full$read_seconds,
  peak     = mem$peak,
  footprint = mem$footprint
)
runs <- runs[, .(
  replicates = .N,
  simulate   = median(simulate),
  build      = median(build),
  peak       = median(peak, na.rm = TRUE),
  footprint  = median(footprint, na.rm = TRUE)
), by = .(engine, scenario, n)]
setorder(runs, scenario, n, simulate)
fwrite(runs, "results/benchmark_runs.csv")

# Model lines, per engine and scenario
lines <- rbindlist(lapply(scenarios, function(s) {
  e <- code_effort(s, root)
  data.table(
    engine   = e$engine,
    language = e$language,
    scenario = s,
    lines    = e$model_lines
  )
}))
fwrite(lines, "results/benchmark_lines.csv")

# Where the numbers come from
sha  <- system2("git", c("-C", shQuote(repo), "rev-parse", "--short", ref), stdout = TRUE)
meta <- unique(full[, .(engine, engine_version)])
meta[, `:=`(ref = ref, commit = sha)]
fwrite(meta, "results/benchmark_meta.csv")

cat(
  sprintf(
    "Benchmark %s (%s): %d engines, %d runs\n",
    ref, sha, uniqueN(full$engine), nrow(full)
  )
)
