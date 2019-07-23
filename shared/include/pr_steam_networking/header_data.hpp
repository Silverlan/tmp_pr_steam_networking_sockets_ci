#ifndef __PR_HEADER_DATA_HPP__
#define __PR_HEADER_DATA_HPP__

#include <cinttypes>

#pragma pack(push,1)
struct HeaderData
{
	uint32_t messageId;
	uint32_t bodySize;
};
#pragma pack(pop)

#endif
