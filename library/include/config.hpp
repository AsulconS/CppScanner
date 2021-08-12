#if !defined(CONFIG_HPP)
#define CONFIG_HPP

#if defined(__SHARED_LIB)
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

#endif // CONFIG_HPP
