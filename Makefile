build:
	gcc \
	*.c \
	-o mytop \
	-lncurses \
	-g 

run:
	./mytop