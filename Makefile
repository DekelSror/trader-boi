CC=gcc
CFLAGS=-Wall -Wextra -g -std=gnu2x

PARSER_OBJS=market_data_processor_single.o nx_parsers.o config_utils.o print_utils.o
ALG_OBJS=alg.o

PARSER=market_data_processor
ALG=strategy_processor
MOCKER=mocker

all: $(PARSER) $(ALG) $(MOCKER)

$(PARSER): $(PARSER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ALG): $(ALG_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MOCKER): mocker.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PARSER) $(ALG) $(MOCKER) *.o

run-parser:
	./$(PARSER)

run-alg:
	./$(ALG)

.PHONY: all clean run-parser run-alg 