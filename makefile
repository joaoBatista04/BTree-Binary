FILES = src/node.c src/btree.c src/main.c
EXECUTABLE = trab2
FLAGS = -lm -pedantic -Wall -g
ENTRY_FILE = in/caso_teste_1.txt
EXIT_FILE = saida.txt

all:
	@ gcc -o $(EXECUTABLE) $(FILES) $(FLAGS)

run: 
	@ ./$(EXECUTABLE) $(ENTRY_FILE) $(EXIT_FILE)

diff:
	@ diff saida.txt saida1.txt

clean:
	@ rm -f trab1 *.txt

val:
	@ valgrind --leak-check=full --show-leak-kinds=all ./$(EXECUTABLE) $(ENTRY_FILE) $(EXIT_FILE)