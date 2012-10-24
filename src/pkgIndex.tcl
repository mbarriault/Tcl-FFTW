proc platform {} {
    global tcl_platform
    set plat [lindex $tcl_platform(os) 0]
    return "$plat"
}

switch [platform] {
    Linux {
        set ext so
    }
    Darwin {
        set ext dylib
    }
}

package ifneeded fftw 1.0 [list load [file join $dir libTclFFTW.$ext] tclfftw]
