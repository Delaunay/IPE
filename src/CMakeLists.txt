# better way ?
INCLUDE_DIRECTORIES(.)

# Source files
# ====================================
#           (Glob is not recommended)
#   file(GLOB APL_SRC *.cc)
#   file(GLOB_RECURSE APL_SRC *.cc)

SET(PROJECT_TEST_HDS
    ipe.h
    logger.h
)

SET(PROJECT_TEST_SRC
    ipe.cpp
    logger.cpp
)

# main library (prevent recompilation when building tests)
ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_TEST_SRC} ${PROJECT_TEST_HDS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} spdlog::spdlog)

#  main executable
# ==========================

ADD_EXECUTABLE(main main.cpp)
TARGET_LINK_LIBRARIES(main spdlog::spdlog ${PROJECT_NAME})
