add_library(sse4-${ARCH} MODULE CopyFrame.cpp)
set_property(TARGET sse4-${ARCH} PROPERTY COMPILE_FLAGS "-fPIC -msse4.1")
install(TARGETS sse4-${ARCH} DESTINATION ${LIBPATH}/system COMPONENT RUNTIME)
