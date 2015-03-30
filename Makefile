CC=gcc
BIN=tweeter
SRC=main.c
CFLAGS=-std=c99 -O3
LFLAGS=-lpthread
TEST_DIR=out

all: $(BIN)

tweeter: $(SRC)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(BIN) $(SRC)

test: \
	$(TEST_DIR)/1.txt \
	$(TEST_DIR)/2.txt \
	$(TEST_DIR)/3.txt \
	$(TEST_DIR)/4.txt \
	$(TEST_DIR)/5.txt \
	$(TEST_DIR)/6.txt \
	$(TEST_DIR)/7.txt \
	$(TEST_DIR)/8.txt

# zip project directory using the scheme
# yourFirstName-yourLastName.zip
zip:
	git archive --format zip master -o Abdullah-Badahdah.zip

$(TEST_DIR)/%.txt: $(BIN) $(TEST_DIR)
	./$(BIN) $* > $@

$(TEST_DIR):; @mkdir $(TEST_DIR)

clean:; @rm $(BIN)
