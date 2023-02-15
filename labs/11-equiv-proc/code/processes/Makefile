BOOTLOADER=my-install

START = staff-start.o

PROGS  = single-step-proc.c

# have a common library for the tests
LIB = libasm.a
LIB_SRC = interrupts.S breakpoint.c

STAFF_LIB_OBJS = $(CS140E_2023_PATH)/libpi/staff-objs/kmalloc.o

# automatically run.
RUN=1

CAN_EMIT=1

EXCLUDE ?= grep -v simple_boot
include $(CS140E_2023_PATH)/libpi/mk/Makefile.template-v2
