#
# system-wide default macro definitions (do not modify).
#
include $(MAKECONFIG)
#
# list of source files (list your *.c source files).
#

SRCS=	CCBLK_Main.c 

MDEFS=-D_USE_STAT -D_LOCAL_DB -D_HANGUP_ALARM
MINCLUDES=-I$(TELCOBASE_HOME)/include
MCFLAGS=-g -W -Wall

#
# executable file name for compiled objects.
#
EXENAME=ccblk
#
# module-specific library list for loading.
#
ifeq "$(TGT_SYS_OS_TYPE)" "LINUX_OS"
MLIBDIRS=-L$(TELCOBASE_HOME)/lib 
MLIBS= -ltelcobase
MLDFLAGS=
endif


#
# system-wide default make rules (do not modify)
#
include $(MAKEEXERULES)
