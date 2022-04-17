
CC = clang
CXX = clang++
AS = clang
LD = ld.lld
NM = llvm-nm
OBJCOPY = objcopy
LLVM = 1

EXTRA_LDFLAGS += -L ~/Documents/cpp/llvm-project/build-compiler-rt/lib/linux -lclang_rt.builtins-i386

ARCH = i686
TARGET_TRIP = $(ARCH)-dxgmx-elf
