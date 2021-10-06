set(EXEC_NAME "wax")

set(Source_Files
        "waxc.c"
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
        "waxc_lib.c"
)
source_group("Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Source_Files}")
target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")

target_include_directories(
        "${EXEC_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)
set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# install rules
set(installable_libs "${EXEC_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
