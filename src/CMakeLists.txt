set(src_exe_files 
    creq.c
    creq_test.c)

set(src_lib_files 
    creq.c)

add_executable(creq_test ${src_exe_files})
add_library(creq STATIC ${src_lib_files})
