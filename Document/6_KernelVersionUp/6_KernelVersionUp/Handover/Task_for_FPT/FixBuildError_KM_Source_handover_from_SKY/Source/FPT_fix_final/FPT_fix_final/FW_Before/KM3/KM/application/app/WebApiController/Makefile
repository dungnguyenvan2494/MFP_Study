##############################################
# target object Path
##############################################
export BASEPATH	= $(shell pwd)

ifeq ($(BASETYPE),hw)
	export OBJPATH	= $(BASEPATH)/.objs-hw/$(MACHINE)
else
	export OBJPATH	= $(BASEPATH)/.objs-sim/$(MACHINE)
endif

##############################################
# target module name.
##############################################
PROGPATH		= lib
PROGRAM			= $(PROGPATH)/libWAP.so

##############################################
# target build file.
##############################################
SRCS			 = WAP_MainController.cpp
SRCS			+= WAP_MainControllerShmem.cpp
SRCS			+= WAP_RestApi.cpp
SRCS			+= WAP_SubController.cpp
SRCS			+= WAP_WebApi.cpp
SRCS			+= WAP_access_tokens.cpp
SRCS			+= WAP_jobs.cpp
SRCS			+= WAP_MultiJobs.cpp
SRCS			+= WAP_JobsObserver.cpp
SRCS			+= WAP_BaseJob.cpp
SRCS			+= WAP_AppRequestsController.cpp
SRCS			+= WAP_PowerTask.cpp
SRCS			+= WAP_Power.cpp
SRCS			+= WAP_HddInfoObserver.cpp
SRCS			+= WAP_SystemStatusObserver.cpp
SRCS			+= WAP_LdapAddressSearch.cpp
SRCS			+= WAP_LdapSearchStatusMgr.cpp
SRCS			+= WAP_LdapConfirmStart.cpp
SRCS			+= WAP_LdapConfirmCancel.cpp
SRCS			+= WAP_LdapSearchStart.cpp
SRCS			+= WAP_LdapSearchCancel.cpp
SRCS			+= WAP_LdapSearchResult.cpp
SRCS			+= WAP_plugintrace.cpp
SRCS			+= WAP_NewDetailCode.cpp
SRCS			+= WAP_AsyncResponse.cpp
SRCS			+= WAP_AsyncResponder.cpp
SRCS			+= WAP_MfpInfo.cpp
SRCS			+= WAP_Box.cpp
SRCS			+= WAP_UserBox.cpp
SRCS			+= WAP_UserBoxDocument.cpp
SRCS			+= WAP_MessageHandler.cpp
SRCS			+= WAP_BoxObserver.cpp
ifeq ($(BUILD_TASK_MANAGER), YES)
SRCS			+= WAP_TaskTuningSettings.cpp
SRCS			+= WAP_TaskTuningFiles.cpp
SRCS			+= WAP_TaskTuningOperations.cpp
endif
SRCS			+= WAP_CommonLib.cpp
SRCS			+= WAP_BoxCommonLib.cpp
SRCS			+= WAP_GetThumbnail.cpp
SRCS			+= WAP_GetDocumentList.cpp
SRCS			+= WAP_DeleteDocument.cpp
SRCS			+= WAP_GetPreview.cpp
SRCS			+= WAP_GetPreviewNew.cpp
SRCS			+= WAP_Private.cpp
SRCS			+= WAP_CommonTables.cpp
SRCS			+= WAP_Database.cpp
SRCS			+= WAP_UserdbRole.cpp
SRCS			+= WAP_WebAPIAuthentication.cpp
SRCS			+= WAP_Authentication.cpp
SRCS			+= WAP_LoginSwitch.cpp
SRCS			+= WAP_ProfileNotifyCallback.cpp
SRCS			+= WAP_Secret.cpp
SRCS			+= WAP_AuthDeviceController.cpp
SRCS			+= WAP_AuthDeviceReader.cpp
SRCS			+= WAP_AuthDeviceInfo.cpp
SRCS			+= WAP_AuthDeviceUserInfo.cpp
SRCS			+= WAP_AuthDeviceReaderConfig.cpp
SRCS			+= WAP_AuthDeviceCardStatus.cpp
SRCS			+= WAP_AuthDeviceEventReNotify.cpp
SRCS			+= WAP_GetUsbFileList.cpp
SRCS			+= WAP_DeleteUsbDocument.cpp
SRCS			+= WAP_ScreenContentsController.cpp
SRCS			+= WAP_EditDataRWController.cpp
SRCS			+= WAP_Private_EditDataRWController.cpp
SRCS			+= WAP_MarketPlaceDataController.cpp
SRCS			+= WAP_CurrentSetting.cpp
SRCS			+= WAP_ApplicationManagementData.cpp
SRCS			+= WAP_ApplicationManagementRelaxedDataController.cpp
SRCS			+= WAP_Addresses.cpp
SRCS			+= WAP_DomainChecks.cpp
SRCS			+= WAP_EmailReceiving.cpp
SRCS			+= WAP_HostChecks.cpp
SRCS			+= WAP_Logging.cpp
SRCS			+= WAP_OapExtApp.cpp
SRCS			+= WAP_SystemStatus.cpp
SRCS			+= WAP_ApplicationsController.cpp
SRCS			+= WAP_RegisterLauncher.cpp
SRCS			+= WAP_InstallApplication.cpp
SRCS			+= WAP_ContentsAccess.cpp
SRCS			+= WAP_PostItSetting.cpp
SRCS			+= WAP_LdapServerInfo.cpp
SRCS			+= WAP_ColorCopyRestrictionPassCheck.cpp
SRCS			+= WAP_CheckPinCodeAuthentication.cpp
SRCS			+= WAP_AuthDeviceUserCardidLink.cpp
SRCS			+= WAP_AuthDeviceCardStatusPKI.cpp
SRCS			+= WAP_MemoryRxBox.cpp
SRCS			+= WAP_LineCommandController.cpp
SRCS			+= WAP_LineLogController.cpp
SRCS			+= WAP_Reboot.cpp
SRCS			+= WAP_RinneController.cpp
SRCS			+= WAP_FilesInfomation.cpp
SRCS            	+= WAP_OperateCount.cpp

HEADERS = $(wildcard *.h)

.PHONY: mkobjsdir nginx Solution
all: mkobjsdir

#include ../Rule_api.makes
include ./Rule.makes
include ./Rule.common

C++FLAGS += -DTORNADO

ifeq ($(BASETYPE),all)
	C++FLAGS += -DSIMULATOR -DFUM_LINUX
endif

CFLAGS		+= -Werror=all -funwind-tables
C++FLAGS	+= -Werror=all -funwind-tables

C++FLAGS += -I ../iws/include \
			-I ../iws/deprecated/nativeif \
			-I ../../mfp/system/include \
			-I ../../divlib/ \
			-I ../../divlib/Common/DIV_Marshaling \
			-I ../../divlib/Common/DIV_connect \
			-I ../../divlib/Common/DIV_Proxy \
			-I ../../divlib/Common/DIV_Stub \
			-I ../../divlib/Common/DIV_BaseLibrary \
			-I ../../divlib/Common/DIV_Common \
			-I ../../divlib/client/Proxy \
			-I ../../divlib/client/Proxy/lib/aes/SW/ \
			-I ../../divlib/client/Proxy/nicfum \
			-I ../../divlib/client/Proxy/system/include \
			-I ../../divlib/client/Proxy/system/jsc/include \
			-I ../../divlib/client/Proxy/system/doc \
			-I ../../divlib/client/Proxy/system/igl \
			-I ../../divlib/client/Proxy/system/include \
			-I ../../divlib/client/Proxy/system/jlg \
			-I ../../divlib/client/Proxy/system/job \
			-I ../../divlib/client/Proxy/system/nvd \
			-I ../../divlib/client/Proxy/system/prt \
			-I ../../divlib/client/Proxy/system/pwr \
			-I ../../divlib/client/Proxy/system/scn \
			-I ../../divlib/client/Proxy/system/sys \
			-I ../../divlib/client/Proxy/system/sys/sysg \
			-I ../../divlib/client/Proxy/system/sys/OsWrapper \
			-I ../../divlib/client/Proxy/system/sys/Timer \
			-I ../../divlib/client/Proxy/system/uac \
			-I ../../divlib/client/Proxy/system/ats \
			-I ../../divlib/client/Proxy/system/fax \
			-I ../../divlib/client/Proxy/system/opl \
			-I ../../divlib/client/Proxy/kbd/kbdg \
			-I ../../divlib/client/Proxy/browser/browserg \
			-I ../../divlib/server/Proxy/panel/pcgcom/pcgcomg \
			-I ../../divlib/server/Proxy/panel/ppcg/ppcgg \
			-I ../../divlib/server/Proxy/panel/plibl/plibl_CLCD \
			-I ../../divlib/server/Proxy/panel/pnlmngr/pnlmngrg \
			-I ../../divlib/server/Proxy/panel/pdp/pdp_CLCD \
			-I ../../divlib/server/Proxy/panel/pdp/pdpg \
			-I ../../divlib/server/Proxy/panel/ppcg/ppcg_CLCD \
			-I ../../divlib/server/Proxy/panel/plibl/plibl_CLCD \
			-I ../../api/jobspec/inc \
			-I ../../network/include \
			-I ../../divlib/Common/blkapi \
			-I ../../api/include \
			-I ../../divlib/client/Stub \
			-I ../../mfp \
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


##############################################
# link
##############################################
all: $(PROGRAM) Solution
all: $(PROGRAM) iws nginx

$(PROGRAM): $(ALLOBJS)
	@echo "Creating $(PROGRAM)..."
	$(GCC) -o $(PROGRAM) $(OBJPATH)/*.o -g -Wl,-E -shared

Solution: $(PROGRAM) depend-one
	@echo "Creating $@..."
	$(MAKE) -C $@

iws: $(PROGRAM) depend-one
	@echo "Creating iws $@..."
	$(MAKE) -C $@
	
nginx: $(PROGRAM) iws Solution depend-one
	@echo "Creating $@..."
	$(MAKE) -C $@

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
