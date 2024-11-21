# cxx-libs

[![CI](https://github.com/elsteveogrande/cxx-libs/actions/workflows/ci.yml/badge.svg)](https://github.com/elsteveogrande/cxx-libs/actions/workflows/ci.yml)

Simple, modern C++ libraries.

## Requirements

* Clang (tested in CI with 18)
* GNU Make

## Make and test

There is no process to build or install this project, as these sources are header-only.
However to run the included unit tests, simply run:

```
make testA    # -fsanitize=address
make testM    # -fsanitize=memory
make testT    # -fsanitize=thread
make testU    # -fsanitize=undefined

# Or just say `make test` which is the same as running: `make testA testT testU`.
# `testM` is left out of `make test` because it won't work on OSX.
# However all four sanitizers are used in CI.
make test
```
