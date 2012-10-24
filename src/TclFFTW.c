#include <stdlib.h>
#include <stdio.h>
#include <tcl.h>
#include <complex.h>
#include <fftw3.h>

int fft(ClientData cdata, Tcl_Interp* interp, int objc, struct Tcl_Obj* const objv[]) {
    if ( objc != 2 )
        Tcl_WrongNumArgs(interp, objc, objv, "fft data");
    Tcl_Obj* list = objv[1];

    int count;
    if ( Tcl_ListObjLength(interp, list, &count) != TCL_OK ) {
        Tcl_AppendResult(interp, "Couldn't get list length");
        return TCL_ERROR;
    }

    int newCount = count/2+1;
    double* in = fftw_malloc(sizeof(double) * count);
    fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * newCount);
    
    for ( size_t i=0; i<count; ++i ) {
        Tcl_Obj* x;
        if ( Tcl_ListObjIndex(interp, list, i, &x) != TCL_OK ) {
            Tcl_AppendResult(interp, "Invalid index");
            return TCL_ERROR;
        }
        if ( Tcl_GetDoubleFromObj(interp, x, &in[i]) != TCL_OK ) {
            Tcl_AppendResult(interp, "Not a double");
            return TCL_ERROR;
        }
    }

    fftw_plan plan = fftw_plan_dft_r2c_1d(count, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    
    list = Tcl_NewListObj(0, NULL);
    Tcl_Obj* Re;
    Tcl_Obj* Im;
    Tcl_Obj* C;
    for ( size_t i=0; i<newCount; ++i ) {
        Re = Tcl_NewDoubleObj(creal(out[i]));
        Im = Tcl_NewDoubleObj(cimag(out[i]));
        C = Tcl_NewListObj(0, NULL);
        if ( Tcl_ListObjAppendElement(interp, C, Re) != TCL_OK ) {
            Tcl_AppendResult(interp, "Couldn't append real part");
            return TCL_ERROR;
        }
        if ( Tcl_ListObjAppendElement(interp, C, Im) != TCL_OK ) {
            Tcl_AppendResult(interp, "Couldn't append imaginary part");
            return TCL_ERROR;
        }
        if ( Tcl_ListObjAppendElement(interp, list, C) != TCL_OK ) {
            Tcl_AppendResult(interp, "Couldn't append complex result");
            return TCL_ERROR;
        }
    }
    Tcl_SetObjResult(interp, list);
    
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return TCL_OK;
}

int Tclfftw_Init(Tcl_Interp* interp) {
    if ( Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL )
        return TCL_ERROR;
    if ( Tcl_PkgProvide(interp, "fftw", "1.0") == TCL_ERROR )
        return TCL_ERROR;
    Tcl_CreateObjCommand(interp, "fft", fft, NULL, NULL);
    return TCL_OK;
}