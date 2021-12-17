include_directories("${PROJECT_SOURCE_DIR}/src/")

# Target: tests for requests
add_executable(test_creq_request_app test_creq_request.c)
target_compile_features(test_creq_request_app PUBLIC c_std_11)
target_link_libraries(test_creq_request_app creq unity)
add_test(test_creq_request test_creq_request_app)

# Target: tests for requests
add_executable(test_creq_response_app test_creq_response.c)
target_compile_features(test_creq_response_app PUBLIC c_std_11)
target_link_libraries(test_creq_response_app creq unity)
add_test(test_creq_response test_creq_response_app)
