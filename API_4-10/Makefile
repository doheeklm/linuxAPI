########################################
# To Compile: make t=sample o=sample.c #
########################################

CC = gcc -DDEBUG #컴파일러
CFLAGS = -W -Wall -g #컴파일 옵션 세팅
# -D[매크로]
# -D[매크로]=[매크로값]
# -U[매크로]: 소스파일 내에 #undef 옵션을 추가한 것과 동일함
# -Wall: 모든 모호한 코딩에 대해 경고를 보내는 옵션
# -W: 합법적이지만 모호한 코딩에 대해 경고를 보내는 옵션
# -W -Wall: 아주 사소한 모호성에 대해서도 경고

#t = TARGET
t = #최종 타겟 파일 # ex) shm_rcv

#o = OBJECTS
o = # ex) shm_rcv.c
#명령어 작성할때 make t=(타겟파일) o=(옵젝파일들) 작성하면 됨

all : $(t) #타겟 파일이 여러 개일때

$(t) : $(o)
                $(CC) -ggdb -O0 -o $@ $^ -lpthread
# gcc -o test_exe main.o function.o와 동일
# $(CC): gcc
# $@: 현재 타겟의 이름
# $^: 현재 타겟의 종속 항목 리스트
# -lm: math 라이브러리 사용
# -lpthread: thread 사용

$(o) :

clean : #더미타겟
                rm *.o $(t)
