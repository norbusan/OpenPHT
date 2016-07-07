add_subdirectory(Encoders)
add_subdirectory(Engines/ActiveAE)
add_subdirectory(Interfaces)
add_subdirectory(Sinks)
add_subdirectory(Utils)

add_sources(AEFactory.cpp AEFactory.h)
add_sources(AEResampleFactory.cpp AEResampleFactory.h)
add_sources(AESinkFactory.cpp AESinkFactory.h)
