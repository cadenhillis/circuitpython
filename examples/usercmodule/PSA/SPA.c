#include "py/obj.h"
#include "py/runtime.h"
#include "SunPos.h"
#include "extmod/ulab/code/ndarray.h"
#include "SPA.h"

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
}

static mp_obj_t getSunPos(size_t n_args, const mp_obj_t* args) {
	struct cTime ct;
	struct cLocation cL;
	struct cSunCoordinates cS;

	ct.iYear = mp_obj_get_int(args[0]);
	ct.iMonth = mp_obj_get_int(args[1]);
	ct.iDay = mp_obj_get_int(args[2]);
	ct.dHours = mp_obj_get_float(args[3]);
	ct.dMinutes = mp_obj_get_float(args[4]);
	ct.dSeconds = mp_obj_get_float(args[5]);

	cL.dLatitude = mp_obj_get_float(args[6]);
	cL.dLongitude = mp_obj_get_float(args[7]);

	sunpos(ct, cL, &cS);

	float cs2[] = {(float)cS.dAzimuth, (float) cS.dZenithAngle};	
	ndarray_obj_t* val = new_np_array(2, sizeof(float));
	val->array = m_new0(uint8_t, 2*sizeof(float));
	memcpy(val->array, (uint8_t*) cs2, 2*sizeof(float));
	
	return MP_OBJ_FROM_PTR(val);
	
}



STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(getSunPos_obj,7, 9, getSunPos);

STATIC const mp_rom_map_elem_t SPA_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_Igrf) },
    { MP_ROM_QSTR(MP_QSTR_getSunPos), MP_ROM_PTR(&getSunPos_obj) },
};
STATIC MP_DEFINE_CONST_DICT(SPA_module_globals, SPA_module_globals_table);

const mp_obj_module_t SPA_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&SPA_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_SPA, SPA_user_cmodule, 1/*MODULE_LARGEMODULE_ENABLED*/);
