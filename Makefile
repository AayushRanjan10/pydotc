pydotc: src/lexer.c src/parser.c src/eval.c src/main.c
	gcc src/lexer.c src/parser.c src/eval.c src/main.c -o pydotc
clean:
	rm -f pydotc