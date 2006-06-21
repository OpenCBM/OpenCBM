#if (_MSC_VER >= 1200) // MSVC 6 or newer

 #pragma pack(pop, packonoff)

#elif (__GNUC__ >= 3)

 #pragma pack()

#else

 #pragma error "Unknown compiler!"

#endif
