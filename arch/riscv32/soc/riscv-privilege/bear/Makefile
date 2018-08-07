soc-cflags := -I/$(srctree)/arch/$(ARCH)/soc/$(SOC_FAMILY)/common/
soc-cflags += -march=rv32i -mabi=ilp32

obj-y += vector.o

soc_ld_include := -I$(srctree)/arch/$(ARCH)/soc/$(SOC_FAMILY)/common/ \
		  -I$(srctree)/arch/$(ARCH)/soc/$(SOC_PATH)/
EXTRA_LINKER_CMD_OPT += $(soc_ld_include)
