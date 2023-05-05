CC=g++
TARGET=rdma

client:
	$(CC) -o client rdma_client.c -libverbs

server:
	$(CC) -o server rdma_server.c -libverbs
