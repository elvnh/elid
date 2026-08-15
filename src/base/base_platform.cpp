#include "base_platform.h"

#if OS_LINUX
#    include "base_linux.cpp"
#else
#    error Platform functions not yet implemented for this OS
#endif
