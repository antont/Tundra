// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenNIPlugin_OpenNIAPI_h
#define incl_OpenNIPlugin_OpenNIAPI_h

#if defined (_WINDOWS)
#if defined(OPENNI_PLUGIN_EXPORTS) 
#define OPENNI_PLUGIN_API __declspec(dllexport)
#else
#define OPENNI_PLUGIN_API __declspec(dllimport) 
#endif
#else
#define OPENNI_PLUGIN_API
#endif

#endif // incl_OpenNIAPI_h
