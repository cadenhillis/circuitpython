
#include<stdio.h>
/****************************************************************************/
/*                                                                          */
/*     NGDC's Geomagnetic Field Modeling software for the IGRF and WMM      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Disclaimer: This program has undergone limited testing. It is        */
/*     being distributed unoffically. The National Geophysical Data         */
/*     Center does not guarantee it's correctness.                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 7.0:                                                         */
/*     - input file format changed to                                       */
/*            -- accept new DGRF2005 coeffs with 0.01 nT precision          */
/*            -- make sure all values are separated by blanks               */
/*            -- swapped n and m: first is degree, second is order          */
/*     - corrected feet to km conversion factor                             */
/*     Thanks to all who provided bug reports and suggested fixes           */
/*                                                                          */
/*                                          Stefan Maus Jan-25-2010         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.1:                                                         */
/*     Included option to read coordinates from a file and output the       */
/*     results to a new file, repeating the input and adding columns        */
/*     for the output                                                       */
/*                                          Stefan Maus Jan-31-2008         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.0:                                                         */
/*     Bug fixes for the interpolation between models. Also added warnings  */
/*     for declination at low H and corrected behaviour at geogr. poles.    */
/*     Placed print-out commands into separate routines to facilitate       */
/*     fine-tuning of the tables                                            */
/*                                          Stefan Maus Aug-24-2004         */
/*                                                                          */
/****************************************************************************/

#include <math.h>
#include <float.h>
#include "vector.h"
//#include "igrfCoeffs.h"
//#include "igrf.h"
//#define DBL_EPSILON 2.2204460492503131E-16
#define PI 3.14159265358979323846

#define MAXDEG 13
#define MAXCOEFF (MAXDEG*(MAXDEG+2))
double mag_coeff[MAXCOEFF];                   //Computed coefficients
//#include "igrfCoeffs.h"
int extrapsh(double date);
int shval3(double flat, double flon, double elev, int nnmax, VEC* dest);

const int igrf_date=2010;
const int igrf_ord=13;
const int sv_ord=8;
const double igrf_coeffs[195]={-29496.5,-1585.9,4945.1,-2396.6,3026.0,-2707.7,1668.6,-575.4,1339.7,-2326.3,-160.5,1231.7,251.7,634.2,-536.8,912.6,809.0,286.4,166.6,-211.2,-357.1,164.4,89.7,-309.2,-231.1,357.2,44.7,200.3,188.9,-141.2,-118.1,-163.1,0.1,-7.7,100.9,72.8,68.6,-20.8,76.0,44.2,-141.4,61.5,-22.9,-66.3,13.1,3.1,-77.9,54.9,80.4,-75.0,-57.8,-4.7,-21.2,45.3,6.6,14.0,24.9,10.4,7.0,1.6,-27.7,4.9,-3.4,24.3,8.2,10.9,-14.5,-20.0,-5.7,11.9,-19.3,-17.4,11.6,16.7,10.9,7.1,-14.1,-10.8,-3.7,1.7,5.4,9.4,-20.5,3.4,11.6,-5.3,12.8,3.1,-7.2,-12.4,-7.4,-0.8,8.0,8.4,2.2,-8.4,-6.1,-10.1,7.0,-2.0,-6.3,2.8,0.9,-0.1,-1.1,4.7,-0.2,4.4,2.5,-7.2,-0.3,-1.0,2.2,-4.0,3.1,-2.0,-1.0,-2.0,-2.8,-8.3,3.0,-1.5,0.1,-2.1,1.7,1.6,-0.6,-0.5,-1.8,0.5,0.9,-0.8,-0.4,0.4,-2.5,1.8,-1.3,0.2,-2.1,0.8,-1.9,3.8,-1.8,-2.1,-0.2,-0.8,0.3,0.3,1.0,2.2,-0.7,-2.5,0.9,0.5,-0.1,0.6,0.5,0.0,-0.4,0.1,-0.4,0.3,0.2,-0.9,-0.8,-0.2,0.0,0.8,-0.2,-0.9,-0.8,0.3,0.3,0.4,1.7,-0.4,-0.6,1.1,-1.2,-0.3,-0.1,0.8,0.5,-0.2,0.1,0.4,0.5,0.0,0.4,0.4,-0.2,-0.3,-0.5,-0.3,-0.8};


const double igrf_sv[80]={11.4,16.7,-28.8,-11.3,-3.9,-23.0,2.7,-12.9,1.3,-3.9,8.6,-2.9,-2.9,-8.1,-2.1,-1.4,2.0,0.4,-8.9,3.2,4.4,3.6,-2.3,-0.8,-0.5,0.5,0.5,-1.5,1.5,-0.7,0.9,1.3,3.7,1.4,-0.6,-0.3,-0.3,-0.1,-0.3,-2.1,1.9,-0.4,-1.6,-0.5,-0.2,0.8,1.8,0.5,0.2,-0.1,0.6,-0.6,0.3,1.4,-0.2,0.3,-0.1,0.1,-0.8,-0.8,-0.3,0.4,0.2,-0.1,0.1,0.0,-0.5,0.2,0.3,0.5,-0.3,0.4,0.3,0.1,0.2,-0.1,-0.5,0.4,0.2,0.4};



/****************************************************************************/
/*                                                                          */
/*                           Subroutine extrapsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Extrapolates linearly a spherical harmonic model with a              */
/*     rate-of-change model.                                                */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*          igrf_date - date of base model                                  */
/*          igrf_ord  - maximum degree and order of base model              */
/*        igrf_coeffs - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of base model                 */
/*           sv_ord   - maximum degree and order of rate-of-change model    */
/*           igrf_sv  - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of rate-of-change model       */
/*                                                                          */
/*     Output:                                                              */
/*        mag_coeff - Schmidt quasi-normal internal spherical               */
/*                    harmonic coefficients                                 */
/*           nmax   - maximum degree and order of resulting model           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 16, 1988                                                */
/*                                                                          */
/****************************************************************************/


int extrapsh(double date){
  int   nmax;
  int   k, l;
  int   i;
  int   igo=igrf_ord,svo=sv_ord;
  double factor;
  //# of years to extrapolate
  factor = date - igrf_date;
  //make shure that degree is smaller then MAXDEG
  if(igo>MAXDEG){
      igo=MAXDEG;
  }
  if(svo>MAXDEG){
      svo=MAXDEG;
  }
  //check for equal degree
  if (igo == svo){
      k =  igo * (igo + 2);
      nmax = igo;
  }else{
      //check if reference is bigger
      if (igo > svo){
          k = svo * (svo + 2);
          l = igo * (igo + 2);
          //copy extra elements unchanged
          for ( i = k ; i < l; ++i){
              mag_coeff[i] = igrf_coeffs[i];
          }
          //maximum degree of model
          nmax = igo;
      }else{
          k = igo * (igo + 2);
          l = svo * (svo + 2);
          //put in change for extra elements?
          for(i=k;i<l;++i){
            mag_coeff[i] = factor * igrf_sv[i];
          }
          nmax = svo;
        }
    }
    //apply secular variations to model
    for ( i = 0; i < k; ++i){
        mag_coeff[i] = igrf_coeffs[i] + factor * igrf_sv[i];
    }
    //return maximum degree of model and secular variations
    return nmax;
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine shval3                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Calculates field components from spherical harmonic (sh)             */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           latitude  - north latitude, in radians                         */
/*           longitude - east longitude, in radians                         */
/*           elev      - radial distance from earth's center                */
/*           nmax      - maximum degree and order of coefficients           */
/*                                                                          */
/*     Output:                                                              */
/*           x         - northward component                                */
/*           y         - eastward component                                 */
/*           z         - vertically-downward component                      */
/*                                                                          */
/*     based on subroutine 'igrf' by D. R. Barraclough and S. R. C. Malin,  */
/*     report no. 71/1, institute of geological sciences, U.K.              */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Norman W. Peddie                                               */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/

//define buffer size for q and q
#define PQ_BUFFSIZE         32

int shval3(double flat,double flon,double elev,int nmax,VEC *dest){
  const double earths_radius = 6371.2;
  double slat;
  double clat;
  double ratio;
  double aa, bb, cc;
  double rr;
  double fm,fn;
  double sl[MAXDEG];
  double cl[MAXDEG];
  double p[PQ_BUFFSIZE];
  double q[PQ_BUFFSIZE];
  int i,j,k,l,m,n;
  int kw;
  int npq;
  double x,y,z;

  //calculate sin and cos of latitude
  slat = sin(flat);
  clat = cos(flat);
  //prevent divide by zero
  if(clat==0){
    clat=DBL_EPSILON;
  }

  //calculate sin and cos of longitude
  sl[0] = sin(flon);
  cl[0] = cos(flon);
  //initialize coordinates
  x = 0;
  y = 0;
  z = 0;

  //calculate loop iterations
  npq = (nmax * (nmax + 3)) / 2;

  //calculate ratio of earths radius to elevation
  ratio = earths_radius / elev;

  aa = sqrt(3.0);

  //set initial values of p
  p[0] = 2.0 * slat;
  p[1] = 2.0 * clat;
  p[2] = 4.5 * slat * slat - 1.5;
  p[3] = 3.0 * aa * clat * slat;

  //Set initial values of q
  q[0] = -clat;
  q[1] = slat;
  q[2] = -3.0 * clat * slat;
  q[3] = aa * (slat * slat - clat * clat);

  for(k=0,l=1,n=0,m=0,rr=ratio*ratio; k < npq;k++,m++){
      //testing get wrapped idx
      kw=k%PQ_BUFFSIZE;
      if (n <= m){
          m = -1;
          n+= 1;
          //rr = pow(ratio,n+2);
          rr*=ratio;
          fn = n;
      }
      fm = m+1;
      if (k >= 4){
          j = k - n ;
          //wrap j for smaller array
          j=j%PQ_BUFFSIZE;
          if (m+1 == n){
              aa = sqrt(1.0 - 0.5/fm);
              p[kw] = (1.0 + 1.0/fm) * aa * clat * p[j-1];
              q[kw] = aa * (clat * q[j-1] + slat/fm * p[j-1]);
              sl[m] = sl[m-1] * cl[0] + cl[m-1] * sl[0];
              cl[m] = cl[m-1] * cl[0] - sl[m-1] * sl[0];
          }else{
              aa = sqrt(fn*fn - fm*fm);
              bb = sqrt(((fn - 1.0)*(fn-1.0)) - (fm * fm))/aa;
              cc = (2.0 * fn - 1.0)/aa;
              i = k - 2 * n + 1;
              //wrap i for smaller array
              i=i%PQ_BUFFSIZE;
              p[kw] = (fn + 1.0) * (cc * slat/fn * p[j] - bb/(fn - 1.0) * p[i]);
              q[kw] = cc * (slat * q[j] - clat/fn * p[j]) - bb * q[i];
            }
        }
        aa = rr * mag_coeff[l-1];
      if (m == -1){
          x = x + aa * q[kw];
          z = z - aa * p[kw];
          l+= 1;
      }else{
              bb = rr * mag_coeff[l];
              cc = aa * cl[m] + bb * sl[m];
              x = x + cc * q[kw];
              z = z - cc * p[kw];
              if (clat > 0){
                  y = y + (aa * sl[m] - bb * cl[m]) *fm * p[kw]/((fn + 1.0) * clat);
              }else{
                  y = y + (aa * sl[m] - bb * cl[m]) * q[kw] * slat;
              }
              l+= 2;
      }
    }
    //set destination values
    dest->c.x=x;
    dest->c.y=y;
    dest->c.z=z;
    //always returns zero
    return 0;
}



#include "py/obj.h"
#include "py/runtime.h"
//#include "igrf.h"
#include "extmod/ulab/code/ndarray.h"

/*static ndarray_obj_t* copy_to_np_array( uint8_t* src, size_t len, size_t sz)
{

        ndarray_obj_t* ndarray = m_new_obj(ndarray_obj_t);
        
		ndarray->base.type = &ulab_ndarray_type;
		
		ndarray->dtype = NDARRAY_FLOAT;
        ndarray->boolean =  NDARRAY_NUMERIC;
        ndarray->ndim = 1;
        ndarray->len = len;
        ndarray->itemsize = sz;
        ndarray->shape[ULAB_MAX_DIMS - 1] = len;
        ndarray->strides[ULAB_MAX_DIMS - 1] = sz;

        ndarray->array = src ;
		return ndarray;	
}*/
/*static ndarray_obj_t* getNumpyArray(const mp_obj_t arg)
{

	if (mp_obj_is_type(arg,&ulab_ndarray_type)) {
		ndarray_obj_t* arry = MP_OBJ_TO_PTR(arg);
		return ndarray_copy_view(arry);
	}
	//else //mp_raise_TypeError(translate("input must be a dense ndarray hi"));
	uint8_t ndim = 1;
	size_t shape = 1;
	int32_t strides = 1;
	uint8_t dtype = NDARRAY_FLOAT;// 1;
	return ndarray_new_ndarray(ndim, &shape, &strides, dtype);
}*/
static ndarray_obj_t* new_np_array( size_t len, size_t sz)
{

        ndarray_obj_t* ndarray = m_new_obj(ndarray_obj_t);
        
		ndarray->base.type = &ulab_ndarray_type;
		
		ndarray->dtype = NDARRAY_FLOAT;
        ndarray->boolean =  NDARRAY_NUMERIC;
        ndarray->ndim = 1;
        ndarray->len = len;
        ndarray->itemsize = sz;
        ndarray->shape[ULAB_MAX_DIMS - 1] = len;
        ndarray->strides[ULAB_MAX_DIMS - 1] = sz;

        //ndarray->array = src ;
		return ndarray;	
}/*
STATIC mp_obj_array_t *array_new(char typecode, size_t n) {
    if (typecode == 'x') {
        mp_raise_ValueError(MP_ERROR_TEXT("bad typecode"));
    }
    int typecode_size = mp_binary_get_size('@', typecode, NULL);
    mp_obj_array_t *o = m_new_obj(mp_obj_array_t);
    #if MICROPY_PY_BUILTINS_BYTEARRAY && MICROPY_PY_ARRAY
    o->base.type = (typecode == BYTEARRAY_TYPECODE) ? &mp_type_bytearray : &mp_type_array;
    #elif MICROPY_PY_BUILTINS_BYTEARRAY
    o->base.type = &mp_type_bytearray;
    #else
    o->base.type = &mp_type_array;
    #endif
    o->typecode = typecode;
    o->free = 0;
    o->len = n;
    o->items = m_new(byte, typecode_size * o->len);
    return o;
}
*/
int nmax = 13;
static mp_obj_t Igrf_init(mp_obj_t in) {
    nmax=extrapsh(mp_obj_get_int(in));
	return mp_const_none;
}

static mp_obj_t Igrf_get_NED(size_t n_args,const mp_obj_t* args ) {
    nmax=extrapsh(2024);
	if (n_args > 3) {
        mp_raise_ValueError(MP_ERROR_TEXT("args > 3"));
	}
	if (n_args < 3) {
        mp_raise_ValueError(MP_ERROR_TEXT("args < 3"));
	}
	float l = mp_obj_get_float(args[0]);
	float lo = mp_obj_get_float(args[1]);
	float al = mp_obj_get_float(args[2]);
	VEC field;
	shval3(l, lo, al, nmax, &field);
	float ary[] = {field.c.x, field.c.y, field.c.z}	;
	//ndarray_obj_t* val = ndarray_new_ndarray(1, &shape, &strides, NDARRAY_FLOAT);
	ndarray_obj_t* val = new_np_array(3, sizeof(float));
   	val->array = m_new0(uint8_t, 3*sizeof(float)); 
	memcpy(val->array, (uint8_t*)ary, 3*sizeof(float))	;
   	return MP_OBJ_FROM_PTR(val);
   //return mp_obj_new_float(field.c.x); 
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Igrf_get_NED_obj,2, 4, Igrf_get_NED);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(Igrf_init_obj, Igrf_init);

STATIC const mp_rom_map_elem_t Igrf_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_Igrf) },
    { MP_ROM_QSTR(MP_QSTR_get_NED), MP_ROM_PTR(&Igrf_get_NED_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&Igrf_init_obj) },
};
STATIC MP_DEFINE_CONST_DICT(Igrf_module_globals, Igrf_module_globals_table);

const mp_obj_module_t Igrf_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&Igrf_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_Igrf, Igrf_user_cmodule, 1/*MODULE_LARGEMODULE_ENABLED*/);
