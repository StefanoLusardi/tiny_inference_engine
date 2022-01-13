#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#define API_EXPORT __declspec(dllexport)
#else
	#define API_EXPORT __attribute__ ((visibility ("default")))
#endif

namespace tie::engine
{
    int API_EXPORT run(int argc, char **argv);
}
