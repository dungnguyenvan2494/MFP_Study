##############################################
# target object Path
##############################################
export BASEPATH	= $(shell pwd)

ifeq ($(BASETYPE),hw)
	export OBJPATH	= ../$(BASEPATH)/.objs-hw/$(MACHINE)
else
	export OBJPATH	= ../$(BASEPATH)/.objs-sim/$(MACHINE)
endif

##############################################
# target module name.
##############################################
PROGRAM			= ../lib/libiwsapp.so

##############################################
# target build file.
##############################################
OBJS = $(SRCS:%.cpp=%.o)

LIBPRUGINSYS_SO		= ../lib/libiwsweb.so
LIBPRUGINSYS_OBJS	= IWS_apic.o

HEADERS = $(wildcard *.h)

#include ../Rule_api.makes
include ../Rule.makes
include ../Rule.common

#C++FLAGS += -Wall -Wextra -pedantic -Wendif-labels -Wfloat-equal -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
C++FLAGS += -Wno-error=catch-value
C++FLAGS += -I ../../iws/include \
			-I ../../iws/deprecated/nativeif \
			-I ../../../mfp/system/include \
			-I ../../../divlib/ \
			-I ../../../divlib/client/Proxy \
			-I ../../../divlib/client/Proxy/system/include \
			-I ../../../divlib/client/Proxy/system/nvd \
			-I ../../../api/include \
			-I ../../../mfp


IWSXMLLIB	=	../../iws/lib/libiwsplugsys.a

##############################################
# link
##############################################
all:$(LIBPRUGINSYS_SO) 

%.o:%.cpp
	$(CC) $(C++FLAGS) -c -o $@ $<

$(LIBPRUGINSYS_SO):$(LIBPRUGINSYS_OBJS)
	$(CC) -shared -Wl,-soname, $^ -o $(LIBPRUGINSYS_SO) $(IWSXMLLIB)
