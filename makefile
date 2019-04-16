CC=gcc

build: prog.c
	@$(CC) -o diskwiper prog.c

diskwiper: prog.c
	@$(CC) -o diskwiper prog.c

install: diskwiper
	@echo "Copying Files."
	@cp diskwiper /usr/bin/diskwiper
	@echo "Setting Permitions."
	@chmod 755 /usr/bin/diskwiper
	@echo "Done."

clean:
	@rm -f diskwiper
	@echo "Done"

