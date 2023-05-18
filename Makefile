all: rdma_server rdma_client
CC=gcc
LIBS=-libverbs -lrdmacm
CFLAGS=-O2 -Wall

rdma_server.o: rdma_server.c
	$(CC) $(CFLAGS) -c rdma_server.c
rdma_client.o: rdma_client.c
	$(CC) $(CFLAGS) -c rdma_client.c
rdma_common.o: rdma_common.c
	$(CC) $(CFLAGS) -c rdma_common.c
rdma_client_512B.o:rdma_client_512B.c
	$(CC) $(CFLAGS) -c rdma_client_512B.c
rdma_array.o:rdma_array.c
	$(CC) $(CFLAGS) -c rdma_array.c
rdma_msgtest.o:rdma_msgtest.c
	$(CC) $(CFLAGS) -c rdma_msgtest.c

rdma_server: rdma_server.o rdma_common.o
	$(CC) $(CFLAGS) rdma_server.o rdma_common.o -o rdma_server $(LIBS)

rdma_client: rdma_client.o rdma_common.o
	$(CC) $(CFLAGS) rdma_client.o rdma_common.o -o rdma_client $(LIBS)

rdma512B: rdma_client_512B.o rdma_common.o
	$(CC) $(CFLAGS) rdma_client_512B.o rdma_common.o -o rdma512B $(LIBS)
array: rdma_array.o rdma_common.o
	$(CC) $(CFLAGS) rdma_array.o rdma_common.o -o array $(LIBS)
msgtest: rdma_msgtest.o rdma_common.o
	$(CC) $(CFLAGS) rdma_msgtest.o rdma_common.o -o msgtest $(LIBS)
clean:
	rm -rf *.o rdma_server rdma_client *~
	rm -rf array rdma512B msgtest
