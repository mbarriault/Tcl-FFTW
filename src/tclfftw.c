#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <complex.h>
#include <fftw3.h>


int fft(ClientData cdata, Tcl_Interp* interp, int objc, struct Tcl_Obj* const objv[]) {
    static fftw_plan* fplan = 0;
    static double* fin = 0;
    static complex* fout = 0;
    static int storedCount = 0;
    if ( objc != 2 ) {
        Tcl_WrongNumArgs(interp, objc, objv, "data");
        return TCL_ERROR;
    }
    Tcl_Obj* list = objv[1];

    int count;
    if ( Tcl_ListObjLength(interp, list, &count) != TCL_OK ) {
        Tcl_AppendResult(interp, "Couldn't get list length");
        return TCL_ERROR;
    }
    int newCount = count/2+1;

    if ( count != storedCount ) {
        storedCount = 0;
        if ( fplan ) {
            fftw_destroy_plan(*fplan);
            fftw_free(fplan);
            fplan = 0;
        }
        if ( fin ) fftw_free(fin);
        fin = 0;
        if ( fout ) fftw_free(fout);
        fout = 0;
    }

    if ( !storedCount ) {
        storedCount = count;
        fin = fftw_malloc(sizeof(double) * count);
        fout = fftw_malloc(sizeof(complex) * newCount);
        fplan = fftw_malloc(sizeof(fftw_plan));
        *fplan = fftw_plan_dft_r2c_1d(count, fin, fout, FFTW_MEASURE);
    }

    for ( size_t i=0; i<count; ++i ) {
        Tcl_Obj* x;
        if ( Tcl_ListObjIndex(interp, list, i, &x) != TCL_OK ) {
            Tcl_AppendResult(interp, "Invalid index");
            return TCL_ERROR;
        }
        if ( Tcl_GetDoubleFromObj(interp, x, &fin[i]) != TCL_OK ) {
            Tcl_AppendResult(interp, "Not a double");
            return TCL_ERROR;
        }
    }

    fftw_execute(*fplan);

    list = Tcl_NewListObj(0, NULL);
    Tcl_Obj* Re;
    Tcl_Obj* Im;
    Tcl_Obj* C;
    for ( size_t i=0; i<newCount; ++i ) {
        Re = Tcl_NewDoubleObj(creal(fout[i]));
        Im = Tcl_NewDoubleObj(cimag(fout[i]));
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

    return TCL_OK;
}

int ffti(ClientData cdata, Tcl_Interp* interp, int objc, struct Tcl_Obj* const objv[]) {
    static fftw_plan* bplan = 0;
    static complex* bin = 0;
    static double* bout = 0;
    static int storedCount = 0;
    if ( objc != 2 ) {
        Tcl_WrongNumArgs(interp, objc, objv, "data");
        return TCL_ERROR;
    }
    Tcl_Obj* list = objv[1];

    int count;
    if ( Tcl_ListObjLength(interp, list, &count) != TCL_OK ) {
        Tcl_AppendResult(interp, "Couldn't get list length");
        return TCL_ERROR;
    }
    int newCount = 2*(count-1);

    if ( newCount != storedCount ) {
        storedCount = 0;
        if ( bplan ) {
            fftw_destroy_plan(*bplan);
            fftw_free(bplan);
            bplan = 0;
        }
        if ( bin ) fftw_free(bin);
        bin = 0;
        if ( bout ) fftw_free(bout);
        bout = 0;
    }

    if ( !storedCount ) {
        storedCount = newCount;
        bin = fftw_malloc(sizeof(complex) * count);
        bout = fftw_malloc(sizeof(double) * newCount);
        bplan = fftw_malloc(sizeof(fftw_plan));
        *bplan = fftw_plan_dft_c2r_1d(newCount, bin, bout, FFTW_MEASURE);
    }

    for ( size_t i=0; i<count; ++i ) {
        Tcl_Obj* x;
        if ( Tcl_ListObjIndex(interp, list, i, &x) != TCL_OK ) {
            Tcl_AppendResult(interp, "Invalid index");
            return TCL_ERROR;
        }
        Tcl_Obj* xRe, * xIm;
        if ( Tcl_ListObjIndex(interp, x, 0, &xRe) != TCL_OK ) {
            Tcl_AppendResult(interp, "Can't get real component");
            return TCL_ERROR;
        }
        if ( Tcl_ListObjIndex(interp, x, 1, &xIm) != TCL_OK ) {
            Tcl_AppendResult(interp, "Can't get imag component");
            return TCL_ERROR;
        }
        double Re, Im;
        if ( Tcl_GetDoubleFromObj(interp, xRe, &Re) != TCL_OK ) {
            Tcl_AppendResult(interp, "Real part not a double");
            return TCL_ERROR;
        }
        if ( Tcl_GetDoubleFromObj(interp, xIm, &Im) != TCL_OK ) {
            Tcl_AppendResult(interp, "Imag part not a double");
            return TCL_ERROR;
        }
        bin[i] = Re + Im * I;
    }

    fftw_execute(*bplan);

    list = Tcl_NewListObj(0, NULL);
    Tcl_Obj* X;
    for ( size_t i=0; i<newCount; ++i ) {
        X = Tcl_NewDoubleObj(bout[i]/newCount);
        if ( Tcl_ListObjAppendElement(interp, list, X) != TCL_OK ) {
            Tcl_AppendResult(interp, "Couldn't append real result");
            return TCL_ERROR;
        }
    }
    Tcl_SetObjResult(interp, list);

    return TCL_OK;
}

int Tclfftw_Init(Tcl_Interp* interp) {
    if ( Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL )
        return TCL_ERROR;
    if ( Tcl_PkgProvide(interp, "fftw", "1.0") == TCL_ERROR )
        return TCL_ERROR;
    Tcl_CreateObjCommand(interp, "fft", fft, NULL, NULL);
    Tcl_CreateObjCommand(interp, "ffti", ffti, NULL, NULL);
    return TCL_OK;
}
