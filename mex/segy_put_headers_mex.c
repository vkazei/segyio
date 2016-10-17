#include <errno.h>
#include <string.h>

#include <segyio/segy.h>
#include "segyutil.h"

#include "matrix.h"
#include "mex.h"

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[]) {

    char* msg1;
    char* msg2;
    int err;

    FILE* fp = segyfopen( prhs[ 0 ], "r+" );
    double* headers = mxGetPr( prhs[ 1 ] );
    int field = mxGetScalar( prhs[ 2 ] );

    struct segy_file_format fmt = filefmt( fp );

    char traceheader[ SEGY_TRACE_HEADER_SIZE ];
    /* 
     * check that the field is valid and writing it won't return an error. by
     * checking it here we don't have to do it in the write loop
     */
    err = segy_set_field( traceheader, field, 0 );

    if( err != 0 ) {
        msg1 = "segy:put_headers:invalid_field";
        msg2 = "Invalid field value/header offset";
        goto cleanup;
    }

    double* itr = headers;
    for( size_t i = 0; i < fmt.traces; ++i, ++itr ) {
        err = segy_traceheader( fp, i, traceheader, fmt.trace0, fmt.trace_bsize );
        const int val = *itr;

        if( err != 0 ) {
            msg1 = "segy:put_headers:os";
            msg2 = strerror( errno );
            goto cleanup;
        }

        segy_set_field( traceheader, field, val );
        err = segy_write_traceheader( fp, i, traceheader, fmt.trace0, fmt.trace_bsize );
        if( err != 0 ) {
            msg1 = "segy:put_headers:os";
            msg2 = strerror( errno );
            goto cleanup;
        }
    }

    fclose( fp );
    return;

cleanup:
    fclose( fp );
    mexErrMsgIdAndTxt( msg1, msg2 );
}