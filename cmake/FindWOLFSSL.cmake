FIND_PATH(WOLFSSL_INCLUDE_DIR
    NAMES wolfssl/ssl.h
)

FIND_LIBRARY(WOLFSSL_LIBRARY
    NAMES wolfssl
)

set(WOLFSSL_INCLUDE_DIRS ${WOLFSSL_INCLUDE_DIR})
set(WOLFSSL_LIBRARIES ${WOLFSSL_LIBRARY})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WOLFSSL DEFAULT_MSG WOLFSSL_LIBRARY WOLFSSL_INCLUDE_DIR)
mark_as_advanced(WOLFSSL_LIBRARY WOLFSSL_INCLUDE_DIR)