//#include "cvc_linalg.h"
#include "kernels.h"
#include "global.h"
#include "../cvc_utils.h"

#define ny 1 

int main ( int argc, char **argv )
{
  // set up MPI cartesian
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  printf("MPI thread support level: %d\n", provided);
  if (provided < MPI_THREAD_FUNNELED){
      MPI_Abort(MPI_COMM_WORLD, 1);
      return 1; // Usually not reached
  }
  int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
  int const proc_dim[4] = {NPROCT, NPROCX, NPROCY, NPROCZ};
  const int period[4] = {0,0,0,0};
  MPI_Cart_create(MPI_COMM_WORLD, 4, proc_dim, period, true, &g_cart_grid);
  MPI_Comm_rank(g_cart_grid, &g_cart_id);
  MPI_Cart_coords(g_cart_grid, g_cart_id, 4, g_proc_coords);

  int const VOLUME = LX * LY * LZ * T; // local volume
  int const RAND = 0;
  const int Lmax = T_global;
  double *p1_0 = (double *)calloc(16 * VOLUME, sizeof(double));
  double *p1_1 = (double *)calloc(16 * VOLUME, sizeof(double));
  //double ** spinor_work = init_2level_dtable ( 2, _GSI( (size_t)(VOLUME+RAND) ));
  double **spinor_work = (double **) malloc(sizeof(double *) * 2);
  for (int i=0; i<2; i++) spinor_work[i] = (double *)malloc(sizeof(double)*_GSI(VOLUME));
  //double *** fwd_y = init_3level_dtable ( 2, 12, _GSI( (size_t)VOLUME ) );
  double *fwd_y = (double *) malloc(sizeof(double) * 2 * 12 * 24 * VOLUME);

  //double ***** P1 = init_5level_dtable ( 2, 4, 4, 4, Lmax );
  double * P1 = (double *) malloc (sizeof(double) * 4 * 4 * 4 * Lmax);

  /* fill fwd_y with test data ... */
  srand(1234);
  for (int i=0; i< 2 * 12 * 24 * VOLUME; i++) {
          fwd_y[i] = rand() * 2. / RAND_MAX - 1; // a random number between -1 and 1
  }

  /* fill p1_1 with test data  */
  for (int i=0; i<16*VOLUME; i++){
    p1_1[i] = rand() * 2. / RAND_MAX - 1; // a random number between -1 and 1
  }

  //compute_pi_0(fwd_y, p1_0, 0, spinor_work, VOLUME);
  //compute_pi(fwd_y, p1_1, 0, VOLUME);

  const int src[4] = {0, 0, 0, 0};
  /* integrate_p1_0(p1_0, P1, 0, src, VOLUME); */
  //integrate_p1(p1_1, P1, 0, src, VOLUME, g_proc_coords, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);

  const int y[4] = {1, 1, 1, 1};
  int y_coord[4 * ny];
  for (int i=0; i<ny; i++) {
    y_coord[i*4 + 0] = i * y[0];
    y_coord[i*4 + 1] = i * y[1];
    y_coord[i*4 + 2] = i * y[2];
    y_coord[i*4 + 3] = i * y[3];
  }

  // set unit
  double const mmuon = 105.6583745 /* MeV */  / 197.3269804 /* MeV fm */;
  double const alat[2] = { 0.1, 0.00013 };  /* fm */
  //double const xunit[2] = { mmuon * alat[0], mmuon * alat[1] };
  double const xunit[2] = { 0.1, 0.1 };

  int const n_p23 = ny * kernel_n * kernel_n_geom * 4 * 4 * 4;
  double *P23 = (double *) malloc(sizeof(double) * n_p23);
  /* fill p1_1 with test data */
  for (int i=0; i<16*VOLUME; i++){
    p1_1[i] = rand() * 2. / RAND_MAX - 1; // a random number between -1 and 1
  }
  struct QED_kernel_temps kqed_t ;
  initialise(&kqed_t);
  //compute_p23_0(p1_1, P23, src, ny, (const int *)y_coord, xunit, VOLUME);
  //compute_p23(p1_1, P23, src, ny, (const int *)y_coord, xunit, kqed_t, VOLUME);

  //compute_2p2_cpu(fwd_y, P1, P23, src, 0, ny, (const int *)y_coord, xunit, kqed_t, VOLUME, g_proc_coords, g_cart_grid, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);

  
  //check_Pi(VOLUME);
  //check_integral(VOLUME, src[0], src[1], src[2], src[3], g_proc_coords, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);
  //check_p23(VOLUME, src, ny, (const int *)y_coord, xunit, g_proc_coords, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);

  //check_Pi_cuda();
  //check_P1_cuda();
  //check_P23_cuda(g_proc_coords, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);

  //check_compute_4pt(VOLUME, g_proc_coords, g_cart_grid, T, LX, LY, LZ, T_global, LX_global, LY_global, LZ_global);

  // debug KQEX
  /* double kerv[6][4][4][4] KQED_ALIGN ;
  double const xv[4] = {0.1, 0.1, 0.1, 10};
  double const yv[4] = {0.1, 0.1, 0.1, 0.1};
  QED_Mkernel_L2(0.4, xv, yv, kqed_t, kerv);
  // find kerv norm
  double *k = kerv[0][0][0];
  double kervnorm = 0;
  for (int i=0; i<384; i++){
    // compute norm
    kervnorm += k[i] * k[i];
  }
  printf("kerv norm = %e\n", kervnorm); */
  double* gauge;
  gauge = (double *) malloc(sizeof(double) * 4 * 3 * 3 * 2 * VOLUME);
  for (int i=0; i<72*VOLUME; i++){
    gauge[i] = rand() * 2. / RAND_MAX - 1; // a random number between -1 and 1
  }
  double **mzz[2] = { NULL, NULL }, **mzzinv[2] = { NULL, NULL };
  cvc::init_clover ( &g_clover, &mzz, &mzzinv, gauge_field_with_phase, g_mu, g_csw );
  cvc::check_residual_clover(fwd_y, fwd_y, gauge, mzz[0], mzzinv[0], 1);

  free(gauge);
  free(p1_0);
  free(p1_1);
  for (int i=0; i<2; i++) free(spinor_work[i]);
  free(spinor_work);
  free(fwd_y);
  free(P1);

  MPI_Finalize();

  return 0;
}

