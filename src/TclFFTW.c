#include <stdlib.h>
#include <tcl.h>
#include <complex.h>
#include <fftw3.h>

int fft(ClientData cdata, Tcl_Interp* interp, int objc, struct Tcl_Obj* const objv[]) {
    int len;
    const char* listStr = Tcl_GetStringFromObj(objv[1], &len);
    Tcl_Obj* list = Tcl_GetVar2Ex(interp, listStr, (char*)NULL, TCL_LEAVE_ERR_MSG);
    int count;
    Tcl_Obj** items;
    Tcl_ListObjGetElements(interp, list, &count, &items);

    int newCount = count/2+1;
    double* data = fftw_malloc(sizeof(data) * count);
    fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * newCount);
    
    fftw_plan plan = fftw_plan_dft_r2c_1d(count, data, out, FFTW_ESTIMATE);
    fftw_execute(plan);
    
    Tcl_Obj** newItems = malloc(sizeof(Tcl_Obj*) * newCount);
    Tcl_Obj** newItem = malloc(sizeof(Tcl_Obj*) * 2);
    for ( size_t i=0; i<newCount; ++i ) {
        Tcl_SetDoubleObj(newItem[0], out[0]);
        Tcl_SetDoubleObj(newItem[1], out[1]);
        newItems[i] = Tcl_NewListObj(2, newItem);
    }
    list = Tcl_NewListObj(newCount, newItems);
    Tcl_SetObjResult(interp, list);
    free(newItems);
    free(newItem);
    
    fftw_destroy_plan(plan);
    fftw_free(data);
    fftw_free(out);
}

int Tclfftw_Init(Tcl_Interp* interp) {
    if ( Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL )
        return TCL_ERROR;
    if ( Tcl_PkgProvide(interp, "fftw", "1.0") == TCL_ERROR )
        return TCL_ERROR;
    Tcl_CreateObjCommand(interp, "fft", fft, NULL, NULL);
    return TCL_OK;
}