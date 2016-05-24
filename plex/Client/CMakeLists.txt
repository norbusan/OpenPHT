find_all_sources(. client_SRC)
find_all_sources(MyPlex myPlex_SRC)

if(NOT TARGET_RPI)
  list(REMOVE_ITEM client_SRC ./PlexTranscoderClientRPi.cpp)
endif(NOT TARGET_RPI)

add_sources(${client_SRC} ${myPlex_SRC})

add_subdirectory(Tests)