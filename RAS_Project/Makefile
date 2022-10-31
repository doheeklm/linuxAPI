#
# system-wide default macro definitions (do not modify).
#
include $(MAKECONFIG)
#
# list of source files (list your *.c source files).
#

SRCS= RAS_Main.c			\
	  RAS_Config.c			\
	  RAS_Ipc.c				\
	  RAS_Mmchdlr_Ip.c		\
	  RAS_Mmchdlr_Trc.c		\
	  RAS_Mmchdlr_Usr.c		\
	  RAS_Oammmc.c

MDEFS=-D_USE_STAT -D_LOCAL_DB -D_HANGUP_ALARM
MINCLUDES=-I$(TELCOBASE_HOME)/include
MCFLAGS=-g -W -Wall

#
# executable file name for compiled objects.

EXENAME=	main

# module-specific library list for loading.
#
ifeq "$(TGT_SYS_OS_TYPE)" "LINUX_OS"
MLIBDIRS=-L$(TELCOBASE_HOME)/lib 
MLIBS= -ltelcobase -lstctl -lmplib -ltap -ltelpro -loam
MLDFLAGS=
endif


#
# system-wide default make rules (do not modify)
#
include $(MAKEEXERULES)
