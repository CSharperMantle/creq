set(src_lib_files 
    creq.c
)

set(src_lib_headers
    creq.h
    ${PROJECT_SOURCE_DIR}/lib/c-vector/cvector.h
)

include_directories("${PROJECT_SOURCE_DIR}/lib/c-vector")

add_library(creq STATIC ${src_lib_files})
target_link_libraries(creq c_vector)

install(TARGETS creq DESTINATION lib)
install(FILES ${src_lib_headers} DESTINATION include)
