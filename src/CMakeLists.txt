set(src_files 
    creq.c
)
set(src_header_path "${PROJECT_SOURCE_DIR}/include")
set(src_headers
    ${src_header_path}/creq.h
)

set(lib_headers
    creq.h
    ${PROJECT_SOURCE_DIR}/lib/c-vector/cvector.h
)

include_directories("${PROJECT_SOURCE_DIR}/lib/c-vector")

# Target: CREQ core library
add_library(creq STATIC ${src_headers} ${src_files})
target_compile_features(creq PUBLIC c_std_11)
target_include_directories(creq PUBLIC ${src_header_path} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(creq c_vector)

install(TARGETS creq DESTINATION lib)
install(FILES ${lib_headers} DESTINATION include)
