#pragma once
#include "serializer.h"
#include "../libs/refresh/archive/lib/archive_input.h"
#include "../libs/refresh/archive/lib/archive_output.h"
#include "version.h"
#include <vector>

struct mdc_file_version
{
	//initially we used the same for soft and format
	//but because of a bug in dist2_3d (https://github.com/refresh-bio/mdcompress/commit/96e88621982d550dbf5c239f5675c31de53bd13d)
	//we need to make old archives incompatible with new software versions (and thus we need to separate format version from software version)
	//then we changed mind ale decided to also increase soft version nnumber to 2.0.0, but lets not reuse these defines and just set this here
	int format_major = 2;
	int format_minor = 0;

	int produced_by_major = MDCOMPRESS_VERSION_MAJOR;
	int produced_by_minor = MDCOMPRESS_VERSION_MINOR;
	int produced_by_patch = MDCOMPRESS_VERSION_PATCH;

	//currently require this but this may change if format versioning will differ from software versioning
	//and this is happening because of the bug mentioned above (we need to make old archives incompatible with new software versions)
	static constexpr int MAX_SUPPORTED_FORMAT_MAJOR = 2;
	static constexpr int MIN_SUPPORTED_FORMAT_MAJOR = 2;

	bool load(refresh::archive_input& ar_in)
	{
		std::vector<uint8_t> packed;
		uint64_t metadata;

		int id = ar_in.get_stream_id("format-ver");
		if (id == -1)
			return false;

		if (!ar_in.get_part(id, 0, packed, metadata))
			return false;

		Serializer serializer;
		serializer.set_data(packed);
		format_major = serializer.load32i();
		format_minor = serializer.load32i();

		id = ar_in.get_stream_id("soft-ver");
		if (id == -1)
			return false;

		if (!ar_in.get_part(id, 0, packed, metadata))
			return false;

		serializer.set_data(packed);
		produced_by_major = serializer.load32i();
		produced_by_minor = serializer.load32i();
		produced_by_patch = serializer.load32i();

		return true;
	}

	bool is_supported(std::string& err_msg) const
	{
		if (format_major > MAX_SUPPORTED_FORMAT_MAJOR)
		{
			err_msg = "mdc file format is newer than this software - please update software";
			return false;
		}
		if (format_major < MIN_SUPPORTED_FORMAT_MAJOR)
		{
			if (format_major == 1)
				err_msg = "mdc file format (version " + as_string() + ") is incompatible with this software version. "
				"This file was produced with a version containing a known bug in distance computation. "
				"To decompress this file use mdcompress 1.0.0, however it is recommended to recompress from the original trajectory using the current version.";
			//TODO: when needed extend this code with if else (format_major == 2), etc.
			else //In general we should always cover all cases such that the else below is never executed, I'm leaving it here just in case we forget to update this code when we increase the major version number in the future
				err_msg = "mdc file format (version " + as_string() + ") is incompatible with this software version. ";
			return false;
		}

		return true;
	}

	bool store(refresh::archive_output& ar_out) const
	{
		Serializer serializer;
		serializer.append32i(format_major);
		serializer.append32i(format_minor);
		ar_out.add_part(ar_out.register_stream("format-ver"), serializer.get_data());

		serializer.clear();
		serializer.append32i(produced_by_major);
		serializer.append32i(produced_by_minor);
		serializer.append32i(produced_by_patch);
		ar_out.add_part(ar_out.register_stream("soft-ver"), serializer.get_data());

		return true;
	}

	std::string as_string() const
	{
		return std::to_string(format_major) + "." + std::to_string(format_minor);
	}
};
