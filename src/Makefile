KP_DIR = ../__KernelPatch_lib

CC = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld

INCLUDE_DIRS := . include patch/include linux/include kpm/include linux/arch/arm64/include linux/tools/arch/arm64/include

INCLUDE_FLAGS := $(foreach dir,$(INCLUDE_DIRS),-I$(KP_DIR)/kernel/$(dir))

C_FILES := mod_main.c
C_FILES += control/control.c
C_FILES += komm/komm.c
C_FILES += redirect/redirect.c
C_FILES += feature/feature.c
C_FILES += data/shared_data.c
C_FILES += data/kstring.c
C_FILES += data/parser.c
C_FILES += ptrace/ptrace_hide.c

OBJS := $(C_FILES:.c=.o)

CFLAGS := -g -fno-pie -fno-stack-protector -fno-omit-frame-pointer -fno-common
LDFLAGS := -r

INCLUDE_FLAGS += -I. -Icontrol -Ikomm -Iredirect -Ifeature -Idata -Iptrace

all: mod_main.kpm

mod_main.kpm: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c -O2 -o $@ $<

.PHONY: clean
clean:
	rm -rf *.kpm
	find . -name "*.o" | xargs rm -f