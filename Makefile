CC=gcc
CFLAGS=-Wall -Wextra -g -std=gnu2x
LDFLAGS=-ldl -lpthread

PARSER_OBJS=market_data_processor_single.o nx_parsers.o
ALG_OBJS=alg.o

PARSER=market_data_processor
ALG=strategy_processor

all: $(PARSER) $(ALG)

$(PARSER): $(PARSER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(ALG): $(ALG_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PARSER) $(ALG) *.o

run-parser:
	./$(PARSER)

run-alg:
	./$(ALG)

.PHONY: all clean run-parser run-alg 