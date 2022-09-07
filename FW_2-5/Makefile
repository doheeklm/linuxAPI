#
# system-wide default macro definitions (do not modify).
#
include $(MAKECONFIG)
#
# list of source files (list your *.c source files).
#

# SRCS: 컴파일 할 파일들
# MDEFS: define 파일
# MINCLUDES: include 할 경로
# MLDFLAGS: flag
# EXENAME: 실행 파일
# MLIBS: 라이브러리 파일

SRCS=	TAP_Client.c

MDEFS=-D_USE_STAT -D_LOCAL_DB -D_HANGUP_ALARM
MINCLUDES=-I$(TELCOBASE_HOME)/include
MCFLAGS=-g -W -Wall

#
# executable file name for compiled objects.

EXENAME=	cli

# module-specific library list for loading.
#
ifeq "$(TGT_SYS_OS_TYPE)" "LINUX_OS"
MLIBDIRS=-L$(TELCOBASE_HOME)/lib 
MLIBS= -ltelcobase -lmplib -ltap
MLDFLAGS=
endif


#
# system-wide default make rules (do not modify)
#
include $(MAKEEXERULES)
