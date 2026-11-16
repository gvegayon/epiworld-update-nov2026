# Build and run the examples (writing results/*.csv), then render the slides.
#
#   make results   # compile and run every example (in the epiworld devcontainer
#                  # image if podman or docker is available, else with g++)
#   make slides    # quarto render
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

results: $(CSV)

results/%.csv: examples/%.cpp vendor/epiworld.hpp
	mkdir -p results build
	$(RUN) sh -c '$(CXX) $(CXXFLAGS) $< -o build/$* && ./build/$*'

# The diagrams are rendered to images with a Chromium-based browser
QUARTO_CHROMIUM ?= $(wildcard /Applications/Google Chrome.app/Contents/MacOS/Google Chrome)
export QUARTO_CHROMIUM

slides: $(CSV)
	quarto render index.qmd

clean:
	rm -rf build results/*.csv index.html index_files

.PHONY: all results slides clean
