##############################################
# target object Path
##############################################
export BASEPATH	= $(shell pwd)

ifeq ($(BASETYPE),hw)
	export OBJPATH	= $(BASEPATH)/.objs-hw/$(MACHINE)
	export OBJWAPIPATH	= $(BASEPATH)/../.objs-hw/$(MACHINE)
else
	export OBJPATH	= $(BASEPATH)/.objs-sim/$(MACHINE)
	export OBJWAPIPATH	= $(BASEPATH)/../.objs-sim/$(MACHINE)
endif

##############################################
# target module name.
##############################################
PROGPATH		= ./../lib
PROGRAM			= $(PROGPATH)/libWAP_Solution.so

##############################################
# target build file.
##############################################
SRCS			 = WAP_MainController_Solution.cpp
SRCS			+= WAP_RestApi_Solution.cpp
#SRCS			+= ../WAP_SubController.cpp
SRCS			+= WAP_WebApi_Solution.cpp
SRCS			+= WAP_SolutionTop.cpp
SRCS			+= WAP_AppRunning.cpp

OBJS			 = $(OBJPATH)/*.o
OBJS			+= $(OBJWAPIPATH)/WAP_SubController.o

HEADERS = $(wildcard *.h)

.PHONY: mkobjsdir nginx
all: mkobjsdir

#include ../Rule_api.makes
include ../Rule.makes
include ../Rule.common

C++FLAGS += -DTORNADO

ifeq ($(BASETYPE),all)
	C++FLAGS += -DSIMULATOR -DFUM_LINUX
endif

CFLAGS		+= -Werror=all -funwind-tables -Wno-error=stringop-truncation -Wno-error=catch-value
C++FLAGS	+= -Werror=all -funwind-tables -Wno-error=stringop-truncation -Wno-error=catch-value

#C++FLAGS += -Wall -Wextra -pedantic -Wendif-labels -Wfloat-equal -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
C++FLAGS += -I ../../iws/include \
			-I ../../iws/deprecated/nativeif \
			-I ../../../mfp/system/include \
			-I ../../../divlib/ \
			-I ../../../divlib/Common/DIV_Marshaling \
			-I ../../../divlib/Common/DIV_connect \
			-I ../../../divlib/Common/DIV_Proxy \
			-I ../../../divlib/Common/DIV_Stub \
			-I ../../../divlib/Common/DIV_BaseLibrary \
			-I ../../../divlib/Common/DIV_Common \
			-I ../../../divlib/client/Proxy \
			-I ../../../divlib/client/Proxy/lib/aes/SW/ \
			-I ../../../divlib/client/Proxy/nicfum \
			-I ../../../divlib/client/Proxy/system/include \
			-I ../../../divlib/client/Proxy/system/jsc/include \
			-I ../../../divlib/client/Proxy/system/doc \
			-I ../../../divlib/client/Proxy/system/igl \
			-I ../../../divlib/client/Proxy/system/include \
			-I ../../../divlib/client/Proxy/system/jlg \
			-I ../../../divlib/client/Proxy/system/job \
			-I ../../../divlib/client/Proxy/system/nvd \
			-I ../../../divlib/client/Proxy/system/prt \
			-I ../../../divlib/client/Proxy/system/pwr \
			-I ../../../divlib/client/Proxy/system/scn \
			-I ../../../divlib/client/Proxy/system/sys \
			-I ../../../divlib/client/Proxy/system/sys/sysg \
			-I ../../../divlib/client/Proxy/system/sys/OsWrapper \
			-I ../../../divlib/client/Proxy/system/sys/Timer \
			-I ../../../divlib/client/Proxy/system/uac \
			-I ../../../divlib/client/Proxy/system/ats \
			-I ../../../divlib/client/Proxy/system/fax \
			-I ../../../divlib/client/Proxy/kbd/kbdg \
			-I ../../../divlib/client/Proxy/browser/browserg \
			-I ../../../divlib/server/Proxy/panel/pcgcom/pcgcomg \
			-I ../../../divlib/server/Proxy/panel/ppcg/ppcgg \
			-I ../../../divlib/server/Proxy/panel/plibl/plibl_CLCD \
			-I ../../../divlib/server/Proxy/panel/pnlmngr/pnlmngrg \
			-I ../../../divlib/server/Proxy/panel/pdp/pdp_CLCD \
			-I ../../../divlib/server/Proxy/panel/pdp/pdpg \
			-I ../../../divlib/server/Proxy/panel/ppcg/ppcg_CLCD \
			-I ../../../divlib/server/Proxy/panel/plibl/plibl_CLCD \
			-I ../../../api/jobspec/inc \
			-I ../../../network/include \
			-I ../../../divlib/Common/blkapi \
			-I ../../../api/include \
			-I ../../../divlib/client/Stub \
			-I ../../../mfp \
			-I$(QTDIR)/include \
			-I$(QTDIR)/include/QtWidgets \
			-I$(QTDIR)/include/QtCore \
			-I$(QTDIR)/include/QtGui \
			-I$(TOPPATH)/shlib/LogLib \
			-I$(TOPPATH)/shlib/RecoveryLib \
			-I$(TOPPATH)/application/mfp/system/sys/OsWrapper \
			-I$(TOPPATH)/application/mfp \
			-I$(TOPPATH)/application/mfp/Linux/Platform \
			-I$(TOPPATH)/application/mfp/system \
			-I$(TOPPATH)/application/divlib/client/Proxy/system/job/jobg

clean:
	-@$(RM) $(OBJPATH)/*.o
	-@$(RM) $(OBJPATH)/*.so

##############################################
# link
##############################################
#all: $(PROGRAM) nginx

all: $(PROGRAM) depend-one
$(PROGRAM): $(ALLOBJS)
	@echo "Creating $(PROGRAM)..."
	$(GCC) -o $(PROGRAM) $(OBJS) -g -Wl,-E -shared

#nginx: $(PROGRAM)
#	@echo "Creating $@..."
#	$(MAKE) -C $@

mkobjsdir:
	@mkdir -p $(PROGPATH)
	@mkdir -p $(OBJPATH)

$(ALLOBJS): $(HEADERS)

TOPPATH_DEPEND=$(shell echo $(TOPPATH) | sed -e 's/\//\\\//g')
QTDIR_DEPEND=$(shell echo $(QTDIR) | sed -e 's/\//\\\//g')

depend: $(SUBDIRS) depend-one

depend-one: 
ifneq ($(strip $(ALLSRCS)),)
	@ $(CC) -MM $(C++FLAGS) $(SRCS) > $(DEPEND_INC).tmp
	@ sed "s/.*\.o[ :]/$$\(OBJPATH\)\/\0/g" $(DEPEND_INC).tmp > $(DEPEND_INC).tmp2
	@ sed "s/$(TOPPATH_DEPEND)/\$$\(TOPPATH\)/g" $(DEPEND_INC).tmp2 > $(DEPEND_INC).tmp3
	@ sed "s/$(QTDIR_DEPEND)/\$$\(QTDIR\)/g" $(DEPEND_INC).tmp3 > $(DEPEND_INC)
	-@ $(RM) -f $(DEPEND_INC).tmp
	-@ $(RM) -f $(DEPEND_INC).tmp2
	-@ $(RM) -f $(DEPEND_INC).tmp3
endif

depclean: sub-depclean
ifneq ($(strip $(ALLSRCS)),)
	-@ $(RM) -f $(DEPEND_INC).tmp $(DEPEND_INC)
endif

sub-depclean:
ifneq ($(strip $(SUBDIRS)),)
	-@for i in $(SUBDIRS) ;do $(MAKE) -C $$i depclean ; done
endif

-include $(DEPEND_INC)
