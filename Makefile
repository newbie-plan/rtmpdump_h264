MAIN := rtmp
SRC := ./*.c
#INC := `pkg-config --cflags librtmp`
INC := -I /usr/local/ssl/include/ -I /usr/local/rtmpdump/include/
#LIB := `pkg-config --libs librtmp`
LIB :=   -lrtmp  -lcrypto  -lssl 

${MAIN} : ${SRC}
	gcc $^ -o $@ ${INC} ${LIB}

clean:
	rm ${MAIN}







