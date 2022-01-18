#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#define API_EXPORT_TIE_SERVER __declspec(dllexport)
#else
	#define API_EXPORT_TIE_SERVER __attribute__ ((visibility ("default")))
#endif

namespace tie::engine
{
    int API_EXPORT_TIE_SERVER run(int argc, char **argv);
}
