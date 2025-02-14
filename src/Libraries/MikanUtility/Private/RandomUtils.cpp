#include "RandomUtils.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>

namespace RandomUtils
{
	// See https://lowrey.me/guid-generation-in-c-11/
	std::string RandomHexString(const unsigned int length) 
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());
		std::uniform_int_distribution<> randomDistribution(0, 255);

		std::stringstream ss;
		for (auto i = 0; i < length; i++)
		{
			unsigned char rc = (unsigned char)randomDistribution(generator);

			std::stringstream hexstream;
			hexstream << std::hex << int(rc);
			
			auto hex = hexstream.str();
			ss << (hex.length() < 2 ? '0' + hex : hex);
		}

		return ss.str();
	}
}
