CC=gcc
CFLAGS=-Wall -Wextra -g -std=gnu2x

# PARSER_OBJS=market_data_processor_single.o nx_parsers.o config_utils.o print_utils.o
ALG_OBJS=alg.o socket_provider.o

# PARSER=market_data_processor
ALG=strategy_processor
MOCKER=mocker

all: $(ALG) $(MOCKER)

# $(PARSER): $(PARSER_OBJS)
	# $(CC) $(CFLAGS) -o $@ $^

$(ALG): $(ALG_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MOCKER): mocker.o socket_provider.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(ALG) $(MOCKER) *.o


.PHONY: all clean run-parser run-alg 