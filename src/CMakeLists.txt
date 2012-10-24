add_definitions("-O3")

include_directories(/usr/include/tcl8.5)
include_directories($PROJECT_SOURCE_DIR/src)
add_library(TclFFTW SHARED
    tclfftw.c
)
target_link_libraries(TclFFTW tcl fftw3 m)
install(FILES pkgIndex.tcl DESTINATION lib/TclFFTW)
install(TARGETS TclFFTW LIBRARY DESTINATION lib/TclFFTW ARCHIVE DESTINATION lib/TclFFTW)
