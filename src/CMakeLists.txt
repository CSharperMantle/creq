set(src_files 
    creq.c
)
set(src_header_path "${PROJECT_SOURCE_DIR}/include")
set(src_headers
    ${src_header_path}/creq.h
    ${src_header_path}/cvector.h
)

# Target: CREQ core library
add_library(creq STATIC ${src_headers} ${src_files})
target_compile_features(creq PUBLIC c_std_11)
target_include_directories(creq PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${include_dest}>
    $<INSTALL_INTERFACE:${lib_dest}>
)

install(TARGETS creq EXPORT creq DESTINATION "${lib_dest}")
install(FILES creq-config.cmake DESTINATION "${main_lib_dest}")
install(EXPORT creq DESTINATION "${lib_dest}")
install(FILES ${src_headers} DESTINATION "${include_dest}")
