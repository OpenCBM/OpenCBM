#if (_MSC_VER >= 1200) // MSVC 6 or newer

 #pragma pack(push, packonoff, 1)

#elif (__GNUC__ >= 3)

 #pragma pack(1)

#else

 #pragma error "Unknown compiler!"

#endif
