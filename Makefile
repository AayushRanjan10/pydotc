SRCS = src/lexer.c src/parser.c src/eval.c src/main.c
OBJS = $(SRCS:.c=.o)
HEADERS = src/lexer.h src/parser.h src/eval.h

pydotc: $(OBJS)
	gcc $^ -o $@ -lm
src/%.o: src/%.c $(HEADERS)
	gcc -c $< -o $@ 
clean:
	rm -f src/*.o pydotc