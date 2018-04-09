#ifndef __DNS_PACK_H__
#define __DNS_PACK_H__

#include <string.h>
#include "./DNSHeader.h"
#include "./DNSQuestion.h"
#include "./DNSRecord.h"

struct DNSPack
{
	struct DNSHeader   DHeader;
	struct DNSQuestion DQuestion;
	struct DNSRecord   DRecord;
};

#endif