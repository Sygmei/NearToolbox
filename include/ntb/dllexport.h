#pragma once

// Define EXPORTED for any platform
#ifdef _WIN32
#ifdef WIN_EXPORT
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif
#elif __APPLE__
#define EXPORTED __attribute__((visibility("default")))
#else
#define EXPORTED
#endif