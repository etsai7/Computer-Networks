#ifndef __DNS_HEADER_H__
#define __DNS_HEADER_H__

struct DNSHeader
{
	ushort ID;
	int QR;
	int OPCODE;
	int AA;
	int TC;
	int RD;
	int RA;
	char Z;
	char RCODE;
	ushort QDCOUNT;
	ushort ANCOUNT;
	ushort NSCOUNT;
	ushort ARCOUNT;
};

#endif