CC = gcc -DDEBUG #컴파일러
CFLAGS = -W -Wall -g #컴파일 옵션 세팅

t = add
o = add.c

all : $(t) #타겟 파일이 여러 개일때

$(t) : $(o)
	$(CC) -ggdb -O0 -o $@ $^ -lpthread

# $@: 현재 타겟의 이름
# $^: 현재 타겟의 종속 항목 리스트

$(o) :

clean :
	rm *.o $(t)
