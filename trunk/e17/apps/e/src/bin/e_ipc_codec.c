#include "e.h"

/* local subsystem functions */

/* encode functions, Should these be global? */

/* local subsystem globals */
static Eet_Data_Descriptor *_e_ipc_int_edd = NULL;
static Eet_Data_Descriptor *_e_ipc_double_edd = NULL;

/* externally accessible functions */
int
e_ipc_codec_init(void)
{
   _e_ipc_int_edd = E_CONFIG_DD_NEW("int", E_Ipc_Int);
   E_CONFIG_VAL(_e_ipc_int_edd, E_Ipc_Int, val, INT);

   _e_ipc_double_edd = E_CONFIG_DD_NEW("double", E_Ipc_Double);
   E_CONFIG_VAL(_e_ipc_double_edd, E_Ipc_Double, val, DOUBLE);
   
   return 1;
}

void
e_ipc_codec_shutdown(void)
{
   E_CONFIG_DD_FREE(_e_ipc_int_edd);
   E_CONFIG_DD_FREE(_e_ipc_double_edd);
}

int
e_ipc_codec_int_dec(char *data, int bytes, int *dest)
{
   E_Ipc_Int *dat;
   
   if (!data) return 0;
   dat = eet_data_descriptor_decode(_e_ipc_int_edd, data, bytes);
   if (!dat) return 0;
   if (dest) *dest = dat->val;
   free(dat);
   return 1;
}

void *
e_ipc_codec_int_enc(int val, int *size_ret)
{
   E_Ipc_Int dat;
   
   dat.val = val;
   return eet_data_descriptor_encode(_e_ipc_int_edd, &dat, size_ret);
}

int
e_ipc_codec_double_dec(char *data, int bytes, double *dest)
{
   E_Ipc_Double *dat;
   
   if (!data) return 0;
   dat = eet_data_descriptor_decode(_e_ipc_double_edd, data, bytes);
   if (!dat) return 0;
   if (dest) *dest = dat->val;
   free(dat);
   return 1;
}

void *
e_ipc_codec_double_enc(double val, int *size_ret)
{
   E_Ipc_Double dat;
   
   dat.val = val;
   return eet_data_descriptor_encode(_e_ipc_double_edd, &dat, size_ret);
}

/* local subsystem globals */

