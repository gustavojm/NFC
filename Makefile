######## Build options ########

verbose = 0

######## Build setup ########

# SRCROOT should always be the current directory
SRCROOT         = $(CURDIR)

# .o directory
ODIR            = obj

# Source VPATHS
VPATH           += $(SRCROOT)/freertos/src
VPATH	        += $(SRCROOT)/freertos/src/portable/MemMang
VPATH	        += $(SRCROOT)/freertos/src/portable/GCC/POSIX
VPATH			+= $(SRCROOT)/NFC/src

# FreeRTOS Objects
C_FILES			+= croutine.c
C_FILES			+= event_groups.c
C_FILES			+= list.c
C_FILES			+= queue.c
C_FILES			+= tasks.c
C_FILES			+= timers.c

# portable Objects
C_FILES			+= heap_4.c
C_FILES			+= port.c

INCLUDES        += -I$(SRCROOT)/NFC
# Main Object

C_FILES			+= main.c
C_FILES			+= ad2s1210.c
C_FILES			+= pole.c
C_FILES			+= lift.c
C_FILES			+= pid.c

# Include Paths
INCLUDES        += -I$(SRCROOT)/freertos/inc
INCLUDES        += -I$(SRCROOT)/freertos/src/portable/GCC/POSIX/
INCLUDES        += -I$(SRCROOT)/Demo/Common/include
INCLUDES        += -I$(SRCROOT)/NFC/src
INCLUDES        += -I$(SRCROOT)/NFC/inc

# Generate OBJS names
OBJS = $(patsubst %.c,%.o,$(C_FILES))

######## C Flags ########

# Warnings
#CWARNS += -W
#CWARNS += -Wall
CWARNS += -Werror
CWARNS += -Wextra
CWARNS += -Wformat
CWARNS += -Wmissing-braces
CWARNS += -Wno-cast-align
CWARNS += -Wparentheses
CWARNS += -Wshadow
CWARNS += -Wno-sign-compare
CWARNS += -Wswitch
CWARNS += -Wuninitialized
CWARNS += -Wunknown-pragmas
CWARNS += -Wunused-function
CWARNS += -Wunused-label
#CWARNS += -Wunused-parameter
#CWARNS += -Wunused-value
#CWARNS += -Wunused-variable
#CWARNS += -Wmissing-prototypes

#CWARNS += -Wno-unused-function

CFLAGS += -m32
CFLAGS += -DDEBUG=1

#Incluir math library functions
LINKFLAGS += -lm

#CFLAGS += -g -DUSE_STDIO=1 -D__GCC_POSIX__=1
CFLAGS += -g -UUSE_STDIO -D__GCC_POSIX__=1
ifneq ($(shell uname), Darwin)
CFLAGS += -pthread
endif

# MAX_NUMBER_OF_TASKS = max pthreads used in the POSIX port. 
# Default value is 64 (_POSIX_THREAD_THREADS_MAX), the minimum number required by POSIX.
CFLAGS += -DMAX_NUMBER_OF_TASKS=300

CFLAGS += $(INCLUDES) $(CWARNS) -O2

######## Makefile targets ########

# Rules
.PHONY : all
all: FreeRTOS-Basic


# Fix to place .o files in ODIR
_OBJS = $(patsubst %,$(ODIR)/%,$(OBJS))

# Target that allows compiling just one file by typing make obj/filename.o
$(ODIR)/%.o: %.c
	@mkdir -p $(dir $@)
# If verbose, print gcc execution, else hide
ifeq ($(verbose),1)
	@echo ">> Compiling $<"
	$(CC) $(CFLAGS) -c -o $@ $<
else
	@echo ">> Compiling $(notdir $<)"
	@$(CC) $(CFLAGS) -c -o $@ $<
endif


# Target that allows to obtain preproccessed ouput by typing make obj/filename.i
$(ODIR)/%.i: %.c
	@mkdir -p $(dir $@)
# If verbose, print gcc execution, else hide
ifeq ($(verbose),1)
	@echo ">> PreProcessing $<"
	$(CC) $(CFLAGS) -E -o $@ $<
else
	@echo ">> PreProcessing $(notdir $<)"
	@$(CC) $(CFLAGS) -E -o $@ $<
endif

# Target that allows to obtain assembled ouput by typing make obj/filename.s
$(ODIR)/%.s: %.c
	@mkdir -p $(dir $@)
# If verbose, print gcc execution, else hide
ifeq ($(verbose),1)
	@echo ">> PreProcessing $<"
	$(CC) $(CFLAGS) -S -o $@ $<
else
	@echo ">> PreProcessing $(notdir $<)"
	@$(CC) $(CFLAGS) -S -o $@ $<
endif



FreeRTOS-Basic: $(_OBJS)
	@echo ">> Linking $@..."
ifeq ($(verbose),1)
	$(CC) $(CFLAGS) $^ $(LINKFLAGS) $(LIBS) -o $@
else
	@$(CC) $(CFLAGS) $^ $(LINKFLAGS) $(LIBS) -o $@
endif

	@echo "-------------------------"
	@echo "BUILD COMPLETE: $@"
	@echo "-------------------------"

.PHONY : clean
clean:
	@-rm -rf $(ODIR) FreeRTOS-Sim
	@echo "--------------"
	@echo "CLEAN COMPLETE"
	@echo "--------------"


.PHONY: valgrind
valgrind: FreeRTOS-Sim
	valgrind.bin --tool=memcheck --leak-check=full --show-reachable=yes --track-fds=yes ./FreeRTOS-Sim
