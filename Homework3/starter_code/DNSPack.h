#ifndef __DNS_PACK_H__
#define __DNS_PACK_H__

#include <string.h>
#include "../starter_code/DNSHeader.h"
#include "../starter_code/DNSQuestion.h"

struct DNSPack
{
	struct DNSHeader   DH;
	struct DNSQuestion DQ;
};

#endif