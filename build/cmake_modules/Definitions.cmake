add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO)

# OS specify
if (MSYS OR WIN32 OR MINGW OR CYGWIN)
  add_definitions(-D__USE_W32_SOCKETS)
  add_definitions(-D_WIN32_WINNT=0x0501)
endif()

if (BILLING_MAX_SESSION)
  add_definitions(-D__BILLING_MAX_SESSION__=${BILLING_MAX_SESSION})
endif()

if (NOT BILLING_WITHOUT_E1)
  add_definitions(-D__BILLING_WITH_E1__)
  message(STATUS "Billing with E1")
endif()

if (NOT BILLING_WITHOUT_ANTI_CLONE)
  add_definitions(-D__BILLING_WITH_ANTI_CLONE__)
  message(STATUS "Billing with ANTI CLONE")
endif()

