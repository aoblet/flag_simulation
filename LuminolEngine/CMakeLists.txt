include_directories(include)
file(GLOB_RECURSE SRC_FILES *.cpp *.hpp)
add_library(LuminolEngine ${SRC_FILES})
