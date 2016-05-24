plex_find_header(shairplay/raop.h ${dependdir}/include)

if(DEFINED HAVE_SHAIRPLAY_RAOP_H)
  CHECK_C_SOURCE_COMPILES("
    #include <shairplay/raop.h>
    int main(int argc, char *argv[])
    {
      static raop_callbacks_t test;
      if(sizeof(test.audio_set_metadata))
        return 0;
      return 0;
    }
  "
  HAVE_RAOP_CALLBACKS_AUDIO_SET_METADATA)
endif()
