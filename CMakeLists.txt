cmake_minimum_required(VERSION 3.10.0)
project(creq
    VERSION 0.1.6.3
    LANGUAGES C 
    DESCRIPTION "An elegant way to generate HTTP/1.x messages in C")

# Compile environment dependencies
find_package(Doxygen)
find_program(CPPCHECK NAMES cppcheck)
if (NOT CPPCHECK STREQUAL "CPPCHECK-NOTFOUND")
    set(CMAKE_C_CPPCHECK, "${CPPCHECK}")
    list(
        APPEND CPPCHECK 
            "--enable=pexirformance,portability,style"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
            "--suppress=*:unity/*"
    )
else()
    message(WARNING "Could NOT find cppcheck (${CPPCHECK})")
endif()

# Project-wide constants
set(include_dest "include/${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
set(main_lib_dest "lib/${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")

# Project file structure
add_subdirectory(lib)
add_subdirectory(src)

# Testing file structure
include(CTest)
add_subdirectory(unity) 
add_subdirectory(test) 

# Warning flags
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# add a target to generate API documentation with Doxygen
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

# Installing targets

include(InstallRequiredSystemLibraries)
set(CPACK_PROJECT_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
set(PROJECT_VERSION_FULL ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_FILE_NAME creq-release)
include(CPack)
