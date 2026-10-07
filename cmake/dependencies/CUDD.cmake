# CUDD is provided by the environment as an installed CMake package (README, "CUDD").
# DaedaluX locates it and consumes it: it does not fetch, configure or build it.
find_package(cudd 4.0.0 CONFIG REQUIRED)
