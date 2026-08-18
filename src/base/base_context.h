#pragma once

#if defined(__linux__)
#    define OS_LINUX 1
#elif defined(_WIN32)
#    define OS_WIN32 1
#else
#    error Unsupported operating system
#endif

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || defined (_M_AMD64)
#    define ARCHITECTURE_X64 1
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
#    define ARCHITECTURE_X86 1
#else
#    error Unsupported architecture
#endif

#if defined(__GNUC__)
#    define COMPILER_GCC 1
#elif defined(_MSC_VER)
#    define COMPILER_MSVC 1
#else
#    error Unsupported compiler
#endif

// Define all undefined context macros to 0.
#if !defined(OS_LINUX)
#    define OS_LINUX 0
#endif

#if !defined(OS_WIN32)
#    define OS_WIN32 0
#endif

#if !defined(ARCHITECTURE_X64)
#    define ARCHITECTURE_X64 0
#endif

#if !defined(ARCHITECTURE_X86)
#    define ARCHITECTURE_X86 0
#endif

#if !defined(COMPILER_GCC)
#    define COMPILER_GCC 0
#endif

#if !defined(COMPILER_MSVC)
#    define COMPILER_MSVC 0
#endif

#if !defined(DEBUG_BUILD)
#    define DEBUG_BUILD 0
#endif

#if !defined(POISON_ALLOCATORS)
#    define POISON_ALLOCATORS 0
#endif
