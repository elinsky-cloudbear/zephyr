zephyr_compile_options(
  -march=rv32i
  -mabi=ilp32
  -mcmodel=medany
)

zephyr_sources(
  vector.S
  )
