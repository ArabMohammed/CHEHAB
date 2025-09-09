
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/benchmarks/llm_based_benchmark/he)
add_executable(llm_based_benchmark llm_based_benchmark.cpp)
target_link_libraries(llm_based_benchmark PRIVATE fheco)

# Define the source file and destination file

set(SOURCE_FILE_2 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/he/utils.cpp)
set(DESTINATION_FILE_2 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/he/utils.cpp)
set(SOURCE_FILE_3 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/he/utils.hpp)
set(DESTINATION_FILE_3 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/he/utils.hpp)
set(SOURCE_FILE_4 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/he/CMakeLists.txt)
set(DESTINATION_FILE_4 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/he/CMakeLists.txt) 

set(SOURCE_FILE_5 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/fhe_io_example.txt)
set(DESTINATION_FILE_5 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/fhe_io_example.txt)


set(SOURCE_FILE_7 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/llm_dataset_cleaned.txt)
set(DESTINATION_FILE_7 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/llm_dataset_cleaned.txt)




add_custom_target(copy_my_file_2llm_based_benchmark ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE_2} ${DESTINATION_FILE_2}
    DEPENDS ${SOURCE_FILE}
    COMMENT "Copying ${SOURCE_FILE_2} to ${DESTINATION_FILE_2}"
)
add_custom_target(copy_my_file_3llm_based_benchmark ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE_3} ${DESTINATION_FILE_3}
    DEPENDS ${SOURCE_FILE}
    COMMENT "Copying ${SOURCE_FILE_3} to ${DESTINATION_FILE_3}"
)
add_custom_target(copy_my_file_5llm_based_benchmark ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE_5} ${DESTINATION_FILE_5}
    DEPENDS ${SOURCE_FILE}
    COMMENT "Copying ${SOURCE_FILE_5} to ${DESTINATION_FILE_5}"
)
set(SOURCE_FILE_4 ${CMAKE_SOURCE_DIR}/benchmarks/llm_based_benchmark/he/CMakeLists.txt)
set(DESTINATION_FILE_4 ${CMAKE_SOURCE_DIR}/build/benchmarks/llm_based_benchmark/he/CMakeLists.txt)
add_custom_target(copy_my_file_4llm_based_benchmark ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE_4} ${DESTINATION_FILE_4}
    DEPENDS ${SOURCE_FILE}
    COMMENT "Copying ${SOURCE_FILE_4} to ${DESTINATION_FILE_4}"
)

add_custom_target(copy_my_file_7llm_based_benchmark ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE_7} ${DESTINATION_FILE_7}
    DEPENDS ${SOURCE_FILE}
    COMMENT "Copying ${SOURCE_FILE_7} to ${DESTINATION_FILE_7}"
)