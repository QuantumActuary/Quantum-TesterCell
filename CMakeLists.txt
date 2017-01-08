cmake_minimum_required (VERSION 3.6)
project (TesterCell)
set(CMAKE_INSTALL_PREFIX /Applications/Quantum.app/contents/resources/.kivy)
set(CMAKE_CXX_COMPILER /usr/local/llvm/bin/clang++)
set(CMAKE_CC_COMPILER /usr/local/llvm/bin/clang)
set(PYTHON_INCLUDE_DIRS /Applications/Quantum.app/Contents/Frameworks/python/3.6.0/include/python3.6m)
set(PYTHON_LIBRARIES /Applications/Quantum.app/Contents/Frameworks/python/3.6.0/lib)

add_definitions(-std=c++1y)

# Find Python and set PYTHON_INCLUDE_DIRS
# find_package( PythonLibs 3.6 REQUIRED )

include_directories(
    .
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INSTALL_PREFIX}/include/Quantum
    ${PYTHON_INCLUDE_DIRS}
    /usr/local/gtest/include
    /usr/local/cellar/boost/1.63.0/include
)

LINK_DIRECTORIES(
    ${PYTHON_LIBRARIES}
    /usr/local/gtest/lib
    /Applications/Quantum.app/Contents/Resources/.kivy/lib
)

add_library(${PROJECT_NAME} SHARED
    testercell.cpp
    TesterCell/tester.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    boost_python3-mt
    python3.6m
    QuantumCell
    gtest
)

install(TARGETS ${PROJECT_NAME} DESTINATION extensions/plugins/tests)
install(FILES testercell.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
install(FILES testercell_config.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
install(FILES TesterCell/tester.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ../post-build.sh . lib${PROJECT_NAME}.dylib)