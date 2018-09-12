include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/sbc.c)
