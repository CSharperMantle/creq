set(src_exe_files 
    creq.c
    creq_test.c
)

set(src_lib_files 
    creq.c
)

include_directories("${PROJECT_SOURCE_DIR}/lib/c-vector")

add_library(creq STATIC ${src_lib_files})
target_link_libraries(creq c_vector)

add_executable(creq_test ${src_exe_files})
target_link_libraries(creq_test c_vector)

install(TARGETS creq
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
