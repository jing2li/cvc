#ifndef _IFFTW_H
#define _IFFTW_H

/* FFTW3: fftw_init_threads()/fftw_plan_with_nthreads() are declared
 * directly in fftw3.h, no separate threads header is needed. */
#ifdef HAVE_MPI
#  include <fftw3-mpi.h>
#else
#  include <fftw3.h>
#endif

#endif
