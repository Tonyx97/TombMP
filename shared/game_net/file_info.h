#pragma once

using FILE_HASH = uint64_t;

struct file_info
{
	std::string rsrc,
				name;

	FILE_HASH hash;
};