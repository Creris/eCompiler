#ifndef _JH_HEADER_ERROR_
#define _JH_HEADER_ERROR_

#include <iostream>

namespace jh{
	inline std::ostream& error()
	{
		return std::cerr;
	}
}

#endif	//_JH_HEADER_ERROR_