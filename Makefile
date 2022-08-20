TARGET = 6502-emulator

ASM_TARGET = program.bin

CC = gcc
ASSEMBLER = vasm6502_oldstyle
ASM_FLAGS = -dotdir -Fbin
INCLUDE_DIR = src/6503_emulator
CFLAGS = -I$(INCLUDE_DIR) -F /Library/Frameworks -I$(INCLUDE_DIR)
LIBS = -framework SDL2
C_SOURCES = $(wildcard src/6502_emulator/*.c *.c)
DEPS = $(wildcard $(INCLUDE_DIR)/*.h *.h src/6502_tests/*.h)
OBJ = ${C_SOURCES:.c=.o}
MAIN = src/6502_emulator/main.o


TEST_SOURCES = $(wildcard src/6502_emulator/*.c *.c src/6502_tests/*.c)
TEST_OBJ = ${TEST_SOURCES:.c=.o}

ASM_SOURCES = $(wildcard assembly_sources/videotest.s )

# First rule is the one executed when no parameters are fed to the Makefile

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)


$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test:  $(filter-out $(MAIN),$(TEST_OBJ))
	$(CC) -o $@ $^ $(CFLAGS)

asm: $(ASM_TARGET)

$(ASM_TARGET): $(ASM_SOURCES)
	$(ASSEMBLER) $(ASM_FLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) src/*.bin src/*.o src/*.dis src/*.elf
	$(RM) src/*.bin src/*.o src/*.dis src/*.elf
	$(RM) lib/*.o
	$(RM) $(OBJ)
	$(RM) $(TEST_OBJ)
