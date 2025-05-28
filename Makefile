CC=gcc
CFLAGS=-Wall -Wextra -g -std=gnu2x

ALG_OBJS=alg.o socket_provider.o
CLIENT_OBJS=client.o simple_timeseries.o socket_provider.o candle_agg.o
MOCKER_OBJS=mocker.o socket_provider.o

ALG=alg.out
MOCKER=mocker.out
CLIENT=client.out

all: $(MOCKER) $(CLIENT) $(ALG)

$(MOCKER): $(MOCKER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ALG): $(ALG_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.out *.o


.PHONY: all clean