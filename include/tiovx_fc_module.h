/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef _TIOVX_FC_MODULE
#define _TIOVX_FC_MODULE
/**
* \defgroup group_tiovx_fc_module FlexConnect Module
*
* \brief This section contains host side module APIs for using TIOVX FlexConnect (FC) node tivxVpacFcVissMscNode
*
* \ingroup group_tiovx_modules
*
* @{
*/
#include "tiovx_modules_common.h"
#include <TI/hwa_vpac_fc.h>
#include <tiovx_sensor_module.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIOVX_FC_MODULE_OUTPUT_NA (0)
#define TIOVX_FC_MODULE_OUTPUT_EN (1)

/** \brief Maximum number of MSC outputs allowed from FlexConnect node
*
*/
#define TIOVX_FC_MODULE_MAX_MSC_OUTPUTS 10

/** \brief Maximum number of VISS outputs allowed from FlexConnect node
*
*/
#define TIOVX_FC_MODULE_MAX_VISS_OUTPUTS 4

/** \brief TIOVX FlexConnect module object
*
* Contains the data objects required to use tivxVpacFcVissMscNode
*
*/
typedef struct {
    /*! Node object */
    vx_node node;
    
    /*! VISS configuration parameters */
    vx_user_data_object viss_config;
    
    /*! VISS parameters structure */
    tivx_vpac_viss_params_t viss_params;
    
    /*! User data object for DCC parameter, used as node parameter */
    vx_user_data_object dcc_config;
    
    /*! Input params data objects */
    vx_user_data_object fc_input_prm_obj;

    /*! DCC config file path */
    vx_char dcc_config_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
    
    /*! Object array of AE-AWB result from 2A algorithm */
    vx_object_array viss_ae_awb_result_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    
    /*! First reference of AE-AWB result from ae_awb_result_arr */
    vx_user_data_object viss_ae_awb_result_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    
    /* graph_parameter index of ae_awb_result */
    vx_int32 ae_awb_result_graph_parameter_index;
    
    /* Buf pool depth of ae_awb_result */
    vx_int32 viss_ae_awb_result_bufq_depth;
    
    /*! Object array of H3A objects */
    vx_object_array viss_h3a_stats_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    
    /*! First reference of h3a status from h3a_stats_arr */
    vx_user_data_object viss_h3a_stats_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    
    /* graph_parameter index of h3a_stats */
    vx_int32 viss_h3a_stats_graph_parameter_index;
    
    /* Buf pool depth of h3a_stats output */
    vx_int32 viss_h3a_stats_bufq_depth;
    
    /*! Instance of raw image object for VISS input */
    RawImgObj viss_input;
    
    /*! VISS output image objects */
    ImgObj viss_output[TIOVX_FC_MODULE_MAX_VISS_OUTPUTS];
    
    /*! MSC output image objects */
    ImgObj msc_output[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! FlexConnect configuration parameters */
    vx_user_data_object fc_config;
    
    /*! FlexConnect parameters structure */
    tivx_vpac_fc_viss_msc_params_t fc_params;
    
    /*! MSC filter coefficients data object */
    vx_user_data_object msc_coeff_obj;
    
    /*! MSC crop params data objects */
    vx_user_data_object msc_crop_obj[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! MSC crop params */
    tivx_vpac_msc_crop_params_t msc_crop_params[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! MSC input params data objects */
    vx_user_data_object msc_input_prm_obj;
    
    /*! VISS output selector mapped to 4 outputs.
     * If value is 0 then output is not required
     * If value is 1 then output is required
     */
    vx_int32 viss_output_select[TIOVX_FC_MODULE_MAX_VISS_OUTPUTS];
    
    /*! MSC output selector mapped to TIOVX_FC_MODULE_MAX_MSC_OUTPUTS outputs.
     * If value is 0 then output is not required
     * If value is 1 then output is required
     */
    vx_int32 msc_output_select[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! Interpolation method for MSC scaling */
    vx_int32 interpolation_method;
    
    /*! Number of channels to process in a batch */
    vx_int32 num_channels;
    
    /*! Pointer to sensor object */
    SensorObj *sensorObj;
    
    /*! Flag to indicate whether or not the intermediate output is written */
    vx_int32 en_out_write;
    
    /*! Color format used by scaler node; supported values are \ref VX_DF_IMAGE_U8 and \ref VX_DF_IMAGE_NV12 */
    vx_int32 color_format;

    /*! File path used to write output */
    vx_array file_path;
    
    /* Number or MSC outputs for a given inputl*/
    vx_int32 msc_num_outputs;

    /*! Flag to enable writing output  */
    vx_int32 en_multi_scalar_output;

    /*! File path prefix for VISS outputs */
    vx_array viss_file_prefix[TIOVX_FC_MODULE_MAX_VISS_OUTPUTS];
    
    /*! File path prefix for MSC outputs */
    vx_array msc_file_prefix[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! File path prefix for H3A output */
    vx_array h3a_file_prefix;
    
    /*! Write nodes for VISS outputs */
    vx_node viss_write_node[TIOVX_FC_MODULE_MAX_VISS_OUTPUTS];
    
    /*! Write nodes for MSC outputs */
    vx_node msc_write_node[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! Write node for H3A output */
    vx_node h3a_write_node;
    
    /*! Write command objects */
    vx_user_data_object msc_write_cmd[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    
    /*! Output file path */
    vx_char output_file_path[TIOVX_MODULES_MAX_OBJ_NAME_SIZE];
    
    /*! Name of FlexConnect module */
    vx_char obj_name[TIOVX_MODULES_MAX_OBJ_NAME_SIZE];
} TIOVXFCModuleObj;

/** \brief FlexConnect module init helper function
*
* This FlexConnect init helper function will create all the data objects required to create the FlexConnect
* node
*
* \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
* \param [out] obj        FlexConnect Module object which gets populated with node data objects
* \param [in]  sensorObj  Sensor Module object used to initialize data object parameters;
*                         must be initialized prior to passing to this function
*/
vx_status tiovx_fc_module_init(vx_context context, TIOVXFCModuleObj *obj, SensorObj *sensorObj);

/** \brief FlexConnect module deinit helper function
*
* This FlexConnect deinit helper function will release all the data objects created during the init call
*
* \param [in,out] obj    FlexConnect Module object which contains node data objects which are released in this function
*
*/
vx_status tiovx_fc_module_deinit(TIOVXFCModuleObj *obj);

/** \brief FlexConnect module delete helper function
*
* This FlexConnect delete helper function will delete the node and write nodes that are created during the create call
*
* \param [in,out] obj   FlexConnect Module object which contains node objects which are released in this function
*
*/
vx_status tiovx_fc_module_delete(TIOVXFCModuleObj *obj);

/** \brief FlexConnect module create helper function
*
* This FlexConnect create helper function will create the node using all the data objects created during the init call.
* Internally calls output write node creation if en_out_write is set
*
* \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the node is created
* \param [in,out] obj            FlexConnect Module object which contains node and write nodes which are created in this function
* \param [in]     raw_image_arr  Raw image input object array to VISS node. Must be created separately, typically passed from output of capture node
* \param [in]     ae_awb_result_arr  AE/AWB result object array to VISS node. Must be created separately, typically passed from output of 2A node
* \param [in]     target_string  Target string specifying which hardware accelerator to use
*
*/
vx_status tiovx_fc_module_create(vx_graph graph, TIOVXFCModuleObj *obj, 
    vx_object_array raw_image_arr, vx_object_array ae_awb_result_arr, const char* target_string);

/** \brief FlexConnect module release buffers helper function
*
* This FlexConnect helper function will release the buffers allocated during vxVerifyGraph stage
*
* \param [in] obj  FlexConnect Module object
*
*/
vx_status tiovx_fc_module_release_buffers(TIOVXFCModuleObj *obj);

void tiovx_fc_module_params_init(TIOVXFCModuleObj *obj);

vx_status tiovx_fc_module_configure_controls(TIOVXFCModuleObj *obj);
#ifdef __cplusplus
}
#endif

#endif /* _TIOVX_FC_MODULE */