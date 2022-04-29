#ifndef FRANUTILS_FILESYSTEM_H
#define FRANUTILS_FILESYSTEM_H

#define FRANUTILS_MODDIR 1
#include "FranUtils.hpp"

// Because of Valve's broken include order, we can't just include
// Filesystem header here. So we should inform the devs

#ifdef _FILESYSTEM_

namespace FranUtils::Filesystem
{
	// Check if a file exists in mod dir or fallback dir.
	inline bool Exists(const std::string& file)
	{
		if (std::filesystem::exists(FranUtils::GetModDirectory() + file))
			return true;// Mod dir file
		else if (std::filesystem::exists(std::filesystem::current_path().string() + "//" + FranUtils::Globals::GetFallbackDir() + "//" + file))
			return true;// Fallback dir file
		else 
			return false;
	}

	// Check if a file exists and open if it does.
	// Returns false if the file doesn't exist
	inline bool OpenInputFile(const std::string& file, std::ifstream& stream)
	{
		// Mod dir file
		if (std::filesystem::exists(FranUtils::GetModDirectory() + file))
		{
			stream.open(FranUtils::GetModDirectory() + file);
			return true;
		}
		// Fallback dir file
		else if (std::filesystem::exists(std::filesystem::current_path().string() + "//" + FranUtils::Globals::GetFallbackDir() + "//" + file))
		{
			stream.open(std::filesystem::current_path().string() + "//" + FranUtils::Globals::GetFallbackDir() + "//" + file);
			return true;
		}

		return false;
	}
};

#else

#error "You are trying to use FranUtils.Filesystem but you didn't include standard library filesystem header. Please include it in an appropriate place."

#endif // _FILESYSTEM_

#endif