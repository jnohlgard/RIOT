
if (CPU_CORE STREQUAL "cortex-m0")
  set(CPU_ARCH "armv6m")
elseif (CPU_CORE STREQUAL "cortex-m0plus")
  set(CPU_ARCH "armv6m")
elseif (CPU_CORE STREQUAL "cortex-m23")
  set(CPU_ARCH "armv8m")
elseif (CPU_CORE STREQUAL "cortex-m3")
  set(CPU_ARCH "armv7m")
elseif (CPU_CORE STREQUAL "cortex-m33")
  set(CPU_ARCH "armv8m")
elseif (CPU_CORE STREQUAL "cortex-m4")
  set(CPU_ARCH "armv7m")
elseif (CPU_CORE STREQUAL "cortex-m4f")
  set(CPU_ARCH "armv7m")
elseif (CPU_CORE STREQUAL "cortex-m7")
  set(CPU_ARCH "armv7m")
else()
  message(FATAL_ERROR "Unkwnown cortexm core: \"${CPU_CORE}\"")
endif()
