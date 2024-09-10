set(FHECO_SOURCE_FILES ${FHECO_SOURCE_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/ciphertext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compiler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ops_overloads.cpp
    ${CMAKE_CURRENT_LIST_DIR}/plaintext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/expression.cpp
    ${CMAKE_CURRENT_LIST_DIR}/input.cpp
    ${CMAKE_CURRENT_LIST_DIR}/var.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compute.cpp
)

install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/ciphertext.hpp
        ${CMAKE_CURRENT_LIST_DIR}/common.hpp
        ${CMAKE_CURRENT_LIST_DIR}/compiler.hpp
        ${CMAKE_CURRENT_LIST_DIR}/ops_overloads.hpp
        ${CMAKE_CURRENT_LIST_DIR}/plaintext.hpp
        ${CMAKE_CURRENT_LIST_DIR}/expression.hpp
        ${CMAKE_CURRENT_LIST_DIR}/input.hpp
        ${CMAKE_CURRENT_LIST_DIR}/var.hpp
        ${CMAKE_CURRENT_LIST_DIR}/compute.hpp
        ${CMAKE_CURRENT_LIST_DIR}/tensor.hpp

    DESTINATION
        ${FHECO_INCLUDES_INSTALL_DIR}/dsl
)

set(FHECO_SOURCE_FILES ${FHECO_SOURCE_FILES} PARENT_SCOPE)
