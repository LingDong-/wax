set(EXEC_NAME "waxc")
set(LIBRARY_NAME "wax")  # gets renamed to "libwax"

set(Header_Files
        "common.h"
        "parser.h"
        "tac.h"
        "text.h"
        "to_c.h"
        "to_cpp.h"
        "to_cs.h"
        "to_java.h"
        "to_json.h"
        "to_lua.h"
        "to_py.h"
        "to_swift.h"
        "to_ts.h"
        "to_wat.h"
)
source_group("Header Files" FILES "${Header_Files}")
source_group("${LIBRARY_NAME} Header Files" FILES "waxc_lib.h")

set(Source_Files
        "common.c"
        "parser.c"
        "tac.c"
        "text.c"
        "to_c.c"
        "to_cpp.c"
        "to_cs.c"
        "to_java.c"
        "to_json.c"
        "to_lua.c"
        "to_py.c"
        "to_swift.c"
        "to_ts.c"
        "to_wat.c"
)
source_group("Source Files" FILES "${Source_Files}")
source_group("${EXEC_NAME} Source Files" FILES "waxc.c")
source_group("${LIBRARY_NAME} Source Files" FILES "waxc_lib.c")

# Build two independent things (library and executable).
# [because of twice-defined `void transpile(int, const char*, const char*, int, int)`]
#
# To build one:
# add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}" "waxc.c")
# add_library("${LIBRARY_NAME}" "${LIBRARY_TYPE_FLAG}" "${Header_Files}" "waxc_lib.h" "${Source_Files}" "waxc_lib.c")
# target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}" "waxc.c")
add_library("${LIBRARY_NAME}" "${LIBRARY_TYPE_FLAG}" "${Header_Files}" "waxc_lib.h" "${Source_Files}" "waxc_lib.c")

set(_targets "${EXEC_NAME}" "${LIBRARY_NAME}")
foreach(_target ${_targets})
    target_include_directories(
            "${_target}"
            PRIVATE
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<INSTALL_INTERFACE:include>"
    )
    set_target_properties(
            "${_target}"
            PROPERTIES
            LINKER_LANGUAGE
            C
    )
endforeach(_target ${_targets})

# install rules
set(installable_libs "${_targets}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
