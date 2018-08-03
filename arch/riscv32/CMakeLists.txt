add_subdirectory(soc)
add_subdirectory(core)

set_property(GLOBAL PROPERTY PROPERTY_OUTPUT_FORMAT elf32-littleriscv)

zephyr_compile_options(
  -march=rv32i
  -mabi=ilp32
  #-mcmodel=medany
)