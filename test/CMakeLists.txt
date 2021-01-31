include_directories("${PROJECT_SOURCE_DIR}/src/")
include_directories("${PROJECT_SOURCE_DIR}/lib/c-vector")

add_executable(test_creq_request_app test_creq_request.c)
target_link_libraries(test_creq_request_app creq unity)
add_test(test_creq_request test_creq_request_app)

add_executable(test_creq_response_app test_creq_response.c)
target_link_libraries(test_creq_response_app creq unity)
add_test(test_creq_response test_creq_response_app)
