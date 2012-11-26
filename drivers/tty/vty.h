#ifndef PACKET_H
#define PACKET_H

#define BUF_LEN 100


struct packet
{
	char data[BUF_LEN];
	int destination;
	int source;
	struct semaphore *sem;
};

#endif
