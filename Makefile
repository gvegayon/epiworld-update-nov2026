# Build and run the examples (writing results/*.csv), then render the slides.
#
#   make image     # build the devcontainer image (.devcontainer/Containerfile)
#   make results   # compile and run every example (in the epiworld devcontainer
#                  # image if podman or docker is available, else with g++)
#   make slides    # quarto render (in the image if there is a container runtime)
#   make slides-container  # quarto render, always in the image
#   make benchmark # refresh results/benchmark_*.csv from epiworld-benchmark
#   make all       # both

EXAMPLES := layers durations quarantine two_diseases
CSV      := $(addprefix results/,$(addsuffix .csv,$(EXAMPLES)))

CONTAINER ?= $(shell command -v podman || command -v docker)
IMAGE     ?= localhost/epiworld-dev
CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -O2

ifneq ($(CONTAINER),)
RUN := $(CONTAINER) run --rm -v "$(CURDIR)":"$(CURDIR)" -w "$(CURDIR)" $(IMAGE)
endif

all: results slides

# epiworldR is installed at the commit the branch points to now, so the image
# rebuilds (and records the commit) whenever the branch moves
EPIWORLDR_BRANCH ?= v1-layers

image:
	$(CONTAINER) build -t $(IMAGE) -f .devcontainer/Containerfile \
	  --build-arg EPIWORLDR_REF=$$(git ls-remote https://github.com/UofUEpiBio/epiworldR refs/heads/$(EPIWORLDR_BRANCH) | cut -f1) \
	  .devcontainer

results: $(CSV)

results/%.csv: examples/%.cpp vendor/epiworld.hpp
	mkdir -p results build
	$(RUN) sh -c '$(CXX) $(CXXFLAGS) $< -o build/$* && ./build/$*'

# The diagrams are rendered to images with a Chromium-based browser
CHROME := /Applications/Google Chrome.app/Contents/MacOS/Google Chrome
QUARTO_CHROMIUM ?= $(shell test -x "$(CHROME)" && echo "$(CHROME)")
export QUARTO_CHROMIUM

# The R examples run while the slides render, so they need epiworldR 1.0: in
# the image if there is a container runtime. The C++ in the appendix is read
# from examples/*.cpp.
slides:
	$(RUN) quarto render index.qmd

# Always in the container (an error if there is none, or no image yet)
slides-container:
	@test -n "$(CONTAINER)" || { echo "No podman or docker found" >&2; exit 1; }
	@$(CONTAINER) image inspect $(IMAGE) >/dev/null 2>&1 || \
	  { echo "No image $(IMAGE): run 'make image' first" >&2; exit 1; }
	$(CONTAINER) run --rm -v "$(CURDIR)":"$(CURDIR)" -w "$(CURDIR)" $(IMAGE) \
	  quarto render index.qmd

# The benchmark's numbers (results/benchmark_*.csv), from a git ref of
# epiworld-benchmark, read without checking it out. Runs on the host (R with
# data.table and yaml).
BENCH     ?= $(HOME)/Documents/software/epiworld-benchmark
BENCH_REF ?= origin/v1-layers

benchmark:
	git -C "$(BENCH)" fetch -q origin
	Rscript bench/summarize.R "$(BENCH)" "$(BENCH_REF)"

clean:
	rm -rf build results/*.csv docs

.PHONY: all image results slides slides-container benchmark clean
