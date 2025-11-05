/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

#include "tiovx_fc_module.h"
#include <itt_server.h>


static vx_status tiovx_fc_module_configure_params(vx_context context, TIOVXFCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    SensorObj *sensorObj = obj->sensorObj;
    tivx_vpac_fc_viss_msc_params_t *params = &obj->fc_params;  
    
    fprintf(stderr, "FC-MODULE Configuring FC parameters\n");
    memset(params, 0, sizeof(tivx_vpac_fc_viss_msc_params_t));  
    
    params->tivxVissPrms.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
    params->tivxVissPrms.sensor_dcc_id = sensorObj->sensorParams.dccId;
    params->tivxVissPrms.use_case = 0;
    params->tivxVissPrms.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

    if (params->tivxVissPrms.enable_ir_op) {
        fprintf(stderr, "IR operation is enabled\n");
        params->tivxVissPrms.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;        
        params->tivxVissPrms.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
    }
    else if (params->tivxVissPrms.enable_bayer_op) {        
        fprintf(stderr, "Bayer operation is enabled\n");
        params->tivxVissPrms.fcp[0].mux_output0 = 0;  
        params->tivxVissPrms.fcp[0].mux_output1 = 0;  
    }
    
    
#if defined (VPAC3L)
    fprintf(stderr, "[FC-MODULE-DEBUG] Setting up VISS-MSC mapping\n");
    
    params->msc_in_thread_viss_out_map[0] = TIVX_VPAC_FC_VISS_OUT0;
    params->msc_in_thread_viss_out_map[1] = TIVX_VPAC_FC_VISS_OUT1;
    params->msc_in_thread_viss_out_map[2] = TIVX_VPAC_FC_MSC_CH_INVALID;
    params->msc_in_thread_viss_out_map[3] = TIVX_VPAC_FC_MSC_CH_INVALID;
    
    fprintf(stderr, "The msc_output_select[0] %d\n", obj->msc_output_select[0]);    

    // Debug the mapping params
    fprintf(stderr, "[FC-MODULE-DEBUG] VISS-MSC mapping: %d, %d, %d, %d\n",
            params->msc_in_thread_viss_out_map[0],
            params->msc_in_thread_viss_out_map[1],
            params->msc_in_thread_viss_out_map[2],
            params->msc_in_thread_viss_out_map[3]);

    obj->msc_output_select[0] = TIOVX_FC_MODULE_OUTPUT_EN;

    for(vx_int32 out = 0; out < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; out++)
    {
    if(obj->msc_output_select[0] == TIOVX_FC_MODULE_OUTPUT_EN)
        {   
            obj->msc_output[0].color_format = VX_DF_IMAGE_NV12;
        
            fprintf(stderr, "The msc_output[0] image format is %d\n", obj->msc_output[0].color_format);
        
            fprintf(stderr, "Setting MSC output mapping for NV12 format...\n");
        
            for(int i = 0; i < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; i += 2) { 
                if (i == 0)
                {  
                    params->msc_out_msc_in_map[i] = TIVX_VPAC_FC_MSC0;
                    if (i + 1 < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS)
                    {
                        params->msc_out_msc_in_map[i + 1] = TIVX_VPAC_FC_MSC0;  
                    }
                
                    fprintf(stderr, "Set pair msc_out_msc_in_map[%d] and msc_out_msc_in_map[%d] to MSC0\n", i, i+1);
                }
                else 
                {
                    params->msc_out_msc_in_map[i] = TIVX_VPAC_FC_MSC_TH_INVALID;
                    if (i + 1 < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS) 
                    {
                        params->msc_out_msc_in_map[i + 1] = TIVX_VPAC_FC_MSC_TH_INVALID;
                    }
                }
            }
        
        }
    }
#endif
    // Debug logs
    fprintf(stderr, "NV12 format detected - ensuring msc_out_msc_in_map[0] and [1] both = MSC0 (0)\n");
    fprintf(stderr, "msc_out_msc_in_map[0] = %d, msc_out_msc_in_map[1] = %d\n", 
            params->msc_out_msc_in_map[0], params->msc_out_msc_in_map[1]);

    obj->fc_config = vxCreateUserDataObject(context, "tivx_vpac_fc_viss_msc_params_t",
                                           sizeof(tivx_vpac_fc_viss_msc_params_t),
                                           params); 
    status = vxGetStatus((vx_reference)obj->fc_config);
    if (status != VX_SUCCESS) {
        TIOVX_MODULE_ERROR("[FC-MODULE] Unable to create FC config object!\n");
    }else{
        fprintf(stderr, "Successuly created the obj->fc_config\n");
    }

    fprintf(stderr, "Reached the end of tiovx_fc_module_configure_params function\n");
    
    return status;
}


void tiovx_fc_module_set_coeff(tivx_vpac_msc_coefficients_t *coeff, uint32_t interpolation_method)
{
    fprintf(stderr, "Entering tiovx_fc-module_set_coeff\n");
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 256;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    idx = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 256;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation_method)
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256-weight;
            coeff->multi_phase[0][idx ++] = weight;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256-weight;
            coeff->multi_phase[1][idx ++] = weight;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256-weight;
            coeff->multi_phase[2][idx ++] = weight;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256-weight;
            coeff->multi_phase[3][idx ++] = weight;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
    else
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
}


vx_status tiovx_fc_module_add_write_output_node(vx_graph graph, TIOVXFCModuleObj *obj, vx_int32 out)
{
    fprintf(stderr, "Entering tiovx_fc_module_add_write_output_node\n");
    vx_status status = VX_SUCCESS;
    
    // for (int out = 0; out <= TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; out++)
    // {
    //     if (obj->msc_output_select[out] == TIOVX_FC_MODULE_OUTPUT_EN)
    //     {  
            if (obj->msc_output[0].arr[0] == NULL) {
                fprintf(stderr, "ERROR: msc_output[%d].arr[0] is NULL\n", out);
                status = VX_ERROR_INVALID_REFERENCE;
                
            }else
            {
                fprintf(stderr, "msc_output object array is %p\n",obj->msc_output[0].arr[0]);
                fprintf(stderr, "Check for output_node 1\n");
                vx_image output_img = (vx_image)vxGetObjectArrayItem(obj->msc_output[0].arr[0], 0);
                fprintf(stderr, "output_img is written successfully %p\n", output_img);

                if (obj->msc_file_prefix[out] == NULL)
                {
                    fprintf(stderr, "ERROR: msc_file_prefix[%d] is NULL\n", out);
                    vxReleaseImage(&output_img);
                    status = VX_ERROR_INVALID_REFERENCE;
                }else
                {
                    fprintf(stderr, "msc_file_prefix[%d] in write_output_node is %p\n",out, obj->msc_file_prefix);
                }

                if (obj->file_path == NULL)
                {
                    fprintf(stderr, "ERROR: file_path is NULL\n");
                    vxReleaseImage(&output_img);
                    status = VX_ERROR_INVALID_REFERENCE;
                }else
                {
                    fprintf(stderr, "file_path[%d] in write_output_node is %p\n",out, obj->file_path);
                }

                if (graph == NULL)
                {
                    fprintf(stderr, "ERROR: graph is NULL\n");
                    vxReleaseImage(&output_img);
                    status = VX_ERROR_INVALID_REFERENCE;
                }
                else 
                {
                    fprintf(stderr, "The graph is created successfully: %p\n", graph);
                }

                fprintf(stderr, "Check for output_node 2.9\n");

                obj->msc_write_node[0] = tivxWriteImageNode(graph, output_img, obj->file_path, obj->msc_file_prefix[0]);
                
                if(obj->msc_write_node[0] == NULL){
                    fprintf(stderr, "The obj->msc_write_node[0] is NULL\n");
                }
                else{
                    fprintf(stderr,"The obj->msc_write_node[0] is:%p\n",obj->msc_write_node[0]);                    
                }

                fprintf(stderr, "Check for output_node 3\n");
                vxReleaseImage(&output_img);
                fprintf(stderr, "Check for output_node 4\n");

                status = vxGetStatus((vx_reference)obj->msc_write_node[0]);
                fprintf(stderr, "Check for output_node 5\n");
                
                if((vx_status)VX_SUCCESS == status)
                {
                    vxSetNodeTarget(obj->msc_write_node[out], VX_TARGET_STRING, TIVX_TARGET_MPU_0);

                    vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
                    vxReplicateNode(graph, obj->msc_write_node[out], replicate, 3);
                    fprintf(stderr, "Created node to write msc output\n");

                }
                else
                {
                    TIOVX_MODULE_ERROR("[FLEXCONNECT-MODULE] Unable to create node to write msc output! \n");
                }
            }

            

    //     }
        
    // }

    if((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object output_h3a = (vx_user_data_object)vxGetObjectArrayItem(obj->viss_h3a_stats_arr[0], 0);

        obj->h3a_write_node = tivxWriteUserDataObjectNode(graph, output_h3a, obj->file_path, obj->h3a_file_prefix);
        vxReleaseUserDataObject(&output_h3a);

        status = vxGetStatus((vx_reference)obj->h3a_write_node);
        if((vx_status)VX_SUCCESS == status)
        {
            vxSetNodeTarget(obj->h3a_write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, obj->h3a_write_node, replicate, 3);
        }
        else
        {
            printf("[FLEXCONNECT-MODULE] Unable to create node to write H3A stats! \n");
        }
    }
    return (status);

}

static vx_status tiovx_fc_module_configure_scaler_coeffs(vx_context context, TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_configure_scaler_coeffs\n");
    vx_status status = VX_SUCCESS;

    tivx_vpac_msc_coefficients_t coeffs;

    tiovx_fc_module_set_coeff(&coeffs, obj->interpolation_method);

    /* Set Coefficients */
    obj->msc_coeff_obj = vxCreateUserDataObject(context,
                                "tivx_vpac_msc_coefficients_t",
                                sizeof(tivx_vpac_msc_coefficients_t),
                                NULL);
    status = vxGetStatus((vx_reference)obj->msc_coeff_obj);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)obj->msc_coeff_obj, "flexconnect_node_msc_coeff_obj");

        status = vxCopyUserDataObject(obj->msc_coeff_obj, 0,
                                    sizeof(tivx_vpac_msc_coefficients_t),
                                    &coeffs,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);
    }
    else
    {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create scaler coeffs object! \n");
    }

    return status;
}

static vx_status tiovx_fc_module_configure_dcc_params(vx_context context, TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_configure_dcc_params\n");
    vx_status status = VX_SUCCESS;

    obj->dcc_config = NULL;

    if (obj->dcc_config_file_path[0] == '\0')
    {
        TIOVX_MODULE_PRINTF("No DCC config file, skipping DCC configuration\n");
        return VX_SUCCESS;
    }

    FILE *fp = fopen(obj->dcc_config_file_path, "rb");
    if(fp == NULL)
    {
        TIOVX_MODULE_ERROR("Unable to open DCC config file %s!\n", obj->dcc_config_file_path);
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        int32_t dcc_buff_size;

        fseek(fp, 0L, SEEK_END);
        dcc_buff_size = (int32_t)ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (dcc_buff_size > 0)
        {
            uint8_t * dcc_buf;
            vx_map_id dcc_buf_map_id;

            obj->dcc_config = vxCreateUserDataObject(context, "dcc_fc_viss", dcc_buff_size, NULL );
            status = vxGetStatus((vx_reference)obj->dcc_config);

            if((vx_status)VX_SUCCESS == status)
            {
                vxMapUserDataObject(
                        obj->dcc_config, 0,
                        dcc_buff_size,
                        &dcc_buf_map_id,
                        (void **)&dcc_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST, 0);

                int32_t bytes_read = fread(dcc_buf, sizeof(uint8_t), dcc_buff_size, fp);

                if(bytes_read != dcc_buff_size)
                {
                    TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] DCC config bytes read %d not matching bytes expected %d \n", bytes_read, dcc_buff_size);
                    status = VX_FAILURE;
                }

                vxUnmapUserDataObject(obj->dcc_config, dcc_buf_map_id);
            }
            else
            {
                TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create DCC config object! \n");
            }
        }
    }

    if(fp != NULL)
    {
        fclose(fp);
    }
    fprintf(stderr, "Exit tiovx_fc_module_configure_dcc_params\n");
    return status;
}

static vx_status tiovx_fc_module_create_viss_input(vx_context context, TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_create_viss_input\n");
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    SensorObj *sensorObj = obj->sensorObj;

    /* Create ae_awb results buffer (uninitialized) */
    if(obj->viss_ae_awb_result_bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] ae-awb result buffer queue depth %d greater than max supported %d!\n", obj->viss_ae_awb_result_bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->viss_ae_awb_result_arr[buf]  = NULL;
        obj->viss_ae_awb_result_handle[buf]  = NULL;
    }

    vx_user_data_object ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), NULL);
    status = vxGetStatus((vx_reference)ae_awb_result);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->viss_ae_awb_result_bufq_depth; buf++)
        {
            obj->viss_ae_awb_result_arr[buf] = vxCreateObjectArray(context, (vx_reference)ae_awb_result, sensorObj->num_cameras_enabled);
            status = vxGetStatus((vx_reference)obj->viss_ae_awb_result_arr[buf]);

            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create ae-awb result object array! \n");
                break;
            }
            obj->viss_ae_awb_result_handle[buf] = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->viss_ae_awb_result_arr[buf], 0);

            for (int i=0; i < sensorObj->num_cameras_enabled; i++) {
                void *map_ptr;
                vx_map_id map_id;
                vx_user_data_object ae_awb_result_obj = NULL;

                ae_awb_result_obj = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->viss_ae_awb_result_arr[buf], i);
                vxMapUserDataObject(ae_awb_result_obj, 0, sizeof(tivx_ae_awb_params_t), &map_id, &map_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
                vxUnmapUserDataObject(ae_awb_result_obj, map_id);
                vxReleaseReference((vx_reference *)&ae_awb_result_obj);
            }
        }
        vxReleaseUserDataObject(&ae_awb_result);
    }
    else
    {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create ae-awb result object! \n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        if(obj->viss_input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] viss_input raw image buffer queue depth %d greater than max supported %d!\n", obj->viss_input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->viss_input.arr[buf]  = NULL;
            obj->viss_input.image_handle[buf]  = NULL;
        }

        tivx_raw_image raw_image = tivxCreateRawImage(context, &obj->viss_input.params);
        status = vxGetStatus((vx_reference)raw_image);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->viss_input.bufq_depth; buf++)
            {
                obj->viss_input.arr[buf] = vxCreateObjectArray(context, (vx_reference)raw_image, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->viss_input.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create viss_input raw image array! \n");
                }
                obj->viss_input.image_handle[buf] = (tivx_raw_image)vxGetObjectArrayItem((vx_object_array)obj->viss_input.arr[buf], 0);
            }
            tivxReleaseRawImage(&raw_image);
        }
        else
        {
            TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create viss_input raw image! \n");
        }
    }

    return status;
}



static vx_status tiovx_fc_module_configure_crop_params(vx_context context, TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_configure_crop_params\n");
    vx_status status = VX_SUCCESS;
    vx_int32 out;

    for (out = 0; out < obj->msc_num_outputs; out++)
    {
        obj->msc_crop_obj[out] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t),
                NULL);

        status = vxGetStatus((vx_reference)obj->msc_crop_obj[out]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = vxCopyUserDataObject(obj->msc_crop_obj[out], 0,
                    sizeof(tivx_vpac_msc_crop_params_t),
                    obj->msc_crop_params + out,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Creating user data object for crop params failed!, %d\n", out);
        }
    }

    return status;
}

static vx_status tiovx_fc_module_create_scaler_outputs(vx_context context, TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_create_scaler_outputs\n");
    vx_status status = VX_SUCCESS;
    vx_int32 out, buf;

    // if (obj->num_channels <= 0) {
    //     TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Invalid num_channels: %d, setting to default (1)\n", obj->num_channels);
    //     obj->num_channels = 1;  
    // }
    
    fprintf(stderr,"[create_scaler_outputs] Creating scaler outputs with num_channels=%d\n", obj->num_channels);
    

    if(obj->msc_num_outputs > TIOVX_FC_MODULE_MAX_MSC_OUTPUTS)
    {   
        
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Number of outputs %d greater than max supported %d!\n", obj->msc_num_outputs, TIOVX_FC_MODULE_MAX_MSC_OUTPUTS);
        return VX_FAILURE;
    }

    for(out = 0; out < obj->msc_num_outputs; out++)
    {
        if(obj->msc_output[out].bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->msc_output[out].bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }
    }


    for(out = 0; out < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; out++)
    {
        // // Make sure output is enabled if it's within our num_outputs range
        // if (out < obj->msc_num_outputs) {
        //     obj->msc_output_select[out] = TIOVX_FC_MODULE_OUTPUT_EN;
        //     fprintf(stderr,"[create_scaler_outputs] Enabling output %d\n", out);
        // } else {
        //     obj->msc_output_select[out] = TIOVX_FC_MODULE_OUTPUT_NA;
        // }

        fprintf(stderr,"Initialise arrays to NULL\n");

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->msc_output[out].arr[buf]  = NULL;
            obj->msc_output[out].image_handle[buf]  = NULL;
        }
    }

    for(out = 0; out < obj->msc_num_outputs; out++)
    {
        vx_image out_img;
        
        // Check if dimensions are valid
        if (obj->msc_output[out].width <= 0 || obj->msc_output[out].height <= 0) {
            TIOVX_MODULE_ERROR("[create_scaler_outputs] Invalid dimensions for output %d: width=%d, height=%d\n", 
                              out, obj->msc_output[out].width, obj->msc_output[out].height);
            status = VX_ERROR_INVALID_PARAMETERS;
            break;
        }

        fprintf(stderr, "[create_scaler_outputs] Creating output image: width=%d, height=%d, color_format=0x%x\n", 
                obj->msc_output[out].width, 
                obj->msc_output[out].height,
                obj->msc_output[out].color_format);
        
        out_img = vxCreateImage(context, obj->msc_output[out].width, obj->msc_output[out].height, obj->color_format);
        status = vxGetStatus((vx_reference)out_img);

        if(status == VX_SUCCESS) {
            // Log success
            fprintf(stderr, "[create_scaler_outputs] Successfully created output template image for output %d\n", out);
            fprintf(stderr, "out_img created successfully%p\n", out_img);
        }

        if(status == VX_SUCCESS)
        {
            for(buf = 0; buf < obj->msc_output[out].bufq_depth; buf++)
            {
                if (obj->num_channels <= 0) {
                    TIOVX_MODULE_ERROR("[create_scaler_outputs] num_channels is invalid: %d\n", obj->num_channels);
                    status = VX_ERROR_INVALID_PARAMETERS;
                break;
            }

                fprintf(stderr, "[create_scaler_outputs] Creating object array for output %d buffer %d with %d channels\n", 
                        out, buf, obj->num_channels);
                obj->msc_output[out].arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->num_channels);

                status = vxGetStatus((vx_reference)obj->msc_output[out].arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[create_scaler_outputs] Unable to create output array! \n");
                    break;
                }else{
                    fprintf(stderr, "Successfully created object array: %p\n", obj->msc_output[out].arr[buf]);
                }
                
                vx_size count = 0;
                vxQueryObjectArray(obj->msc_output[out].arr[buf], VX_OBJECT_ARRAY_NUMITEMS, &count, sizeof(count));
                fprintf(stderr, "[create_scaler_outputs] Created array with %zu items\n", count);
                        
                // Make sure array is populated
                if (count == 0) {
                    TIOVX_MODULE_ERROR("[create_scaler_outputs] Object array has 0 items\n");
                    status = VX_ERROR_INVALID_REFERENCE;
                    break;
                }
                
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "flex_connect_scaler_node_output_arr%d_buf%d", out, buf);

                    vxSetReferenceName((vx_reference)obj->msc_output[out].arr[buf], name);
                }

                obj->msc_output[out].image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->msc_output[out].arr[buf], 0);
                
                if (obj->msc_output[out].image_handle[buf] == NULL) {
                        TIOVX_MODULE_ERROR("[create_scaler_outputs] Failed to get image handle for output %d buffer %d\n", 
                            out, buf);
                        status = VX_ERROR_INVALID_REFERENCE;
                        break;
                }
            }
            vxReleaseImage(&out_img);
        }
        else
        {
                          
            TIOVX_MODULE_ERROR("[create_scaler_outputs] Unable to create output image template! \n");
            break;
        }
    }

    obj->en_out_write = 1;

    fprintf(stderr, "en_out_write is %d\n", obj->en_out_write);

    if(obj->en_out_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        
        if (obj->file_path == NULL)
        {
                fprintf(stderr, "ERROR: file_path is NULL\n");
                status = VX_ERROR_INVALID_REFERENCE;
        }
        else
        {
                fprintf(stderr, "file_path is %p\n", obj->file_path);
        }
        
        fprintf(stderr, "Check some 0\n");
        status = vxGetStatus((vx_reference)obj->file_path); 
        fprintf(stderr, "Check some 1\n");

        if(status == VX_SUCCESS)
        {
            fprintf(stderr, "Check some 2\n");
            vxSetReferenceName((vx_reference)obj->file_path, "scaler_write_node_file_path");
            fprintf(stderr, "Check some 3\n");
            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
            fprintf(stderr, "Check some 4\n");
        }
        else
        {
            TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create file path object for fileio!\n");
        }

        for(out = 0; out < obj->msc_num_outputs; out++)
        {
            char msc_file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

            sprintf(msc_file_prefix, "scaler_output_%d", out);
            obj->msc_file_prefix[out] = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);

            if (obj->msc_file_prefix[out] == NULL)
            {
                    fprintf(stderr, "ERROR: file_path is NULL\n");
                    status = VX_ERROR_INVALID_REFERENCE;
            }
            else
            {
                    fprintf(stderr, "msc_file_prefix[%d] is %p\n",out, obj->msc_file_prefix);
            }

            status = vxGetStatus((vx_reference)obj->msc_file_prefix[out]);

            if(status == VX_SUCCESS)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_msc_file_prefix_%d", out);

                fprintf(stderr, "Check some 5\n");

                vxSetReferenceName((vx_reference)obj->msc_file_prefix[out], name);

                fprintf(stderr, "Check some 6\n");

                vxAddArrayItems(obj->msc_file_prefix[out], TIVX_FILEIO_FILE_PREFIX_LENGTH, &msc_file_prefix[0], 1);
            }
            else
            {
                TIOVX_MODULE_ERROR("[FLEX_CONNECT-MODULE-MODULE] Unable to create file prefix object for output %d!\n", out);
                break;
            }

            obj->msc_write_cmd[out] = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
            if (obj->msc_write_cmd[out] == NULL)
            {
                    fprintf(stderr, "ERROR: msc_write_cmd is NULL\n");
                    status = VX_ERROR_INVALID_REFERENCE;
            }
            else
            {
                    fprintf(stderr, "msc_write_cmd[%d] is %p\n",out, obj->msc_write_cmd);
            }

            fprintf(stderr, "Check some 6\n");

            status = vxGetStatus((vx_reference)obj->msc_write_cmd[out]);
            
            fprintf(stderr, "Check some 6\n");

            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[FLEX_CONNECT-MODULE] Unable to create write cmd object for output %d!\n", out);
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "flex_connect_node_write_cmd_%d", out);

                vxSetReferenceName((vx_reference)obj->msc_write_cmd[out], name);
            }
        }

    }
    else
    {
        obj->file_path   = NULL;
        for(out = 0; out < TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; out++)
        {
            obj->msc_file_prefix[out] = NULL;
            obj->msc_write_node[out]  = NULL;
            obj->msc_write_cmd[out]   = NULL;
        }
    }

    return status;
}

vx_status tiovx_fc_module_init(vx_context context, TIOVXFCModuleObj *obj, SensorObj *sensorObj)
{
    fprintf(stderr, "Entering tiovx_fc_module_init\n");
    vx_status status = VX_SUCCESS;

    obj->num_channels = sensorObj ? sensorObj->num_cameras_enabled : 1;
    
    if (obj->num_channels <= 0) {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Invalid number of channels: %d, setting to 1\n", obj->num_channels);
        obj->num_channels = 1;
    }

    obj->sensorObj = sensorObj;

    if(obj->sensorObj == NULL)
    {
        TIOVX_MODULE_ERROR("Sensor Object handle is NULL!");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_dcc_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_scaler_coeffs(context, obj);
    }
    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_create_viss_input(context, obj);
    }
    
    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_create_scaler_outputs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_crop_params(context, obj);
    }

    return status;

}

vx_status tiovx_fc_module_deinit(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_deinit\n");
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    if(((vx_status)VX_SUCCESS == status) && (obj->fc_config != NULL))
    {
        fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing config handle!\n");
        status = vxReleaseUserDataObject(&obj->fc_config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->dcc_config != NULL))
    {
        fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing DCC config handle!\n");
        status = vxReleaseUserDataObject(&obj->dcc_config);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->viss_ae_awb_result_bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing ae-awb result handle!\n");
                status = vxReleaseUserDataObject(&obj->viss_ae_awb_result_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing ae-awb result arr!\n");
                if(obj->viss_ae_awb_result_arr[buf] != NULL)
                {
                    status = vxReleaseObjectArray(&obj->viss_ae_awb_result_arr[buf]);
                }
            }
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->viss_input.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing raw viss_input image handle!\n");
                status = tivxReleaseRawImage(&obj->viss_input.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing raw viss_input image arr!\n");
                status = vxReleaseObjectArray(&obj->viss_input.arr[buf]);
            }
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->viss_h3a_stats_bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing h3a stats handle!\n");
                status = vxReleaseUserDataObject(&obj->viss_h3a_stats_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                fprintf(stderr,"[FLEX-CONNECT-MODULE] Releasing h3a stats arr!\n");
                status = vxReleaseObjectArray(&obj->viss_h3a_stats_arr[buf]);
            }
        }
    }

     for( int out = 0; out < obj->msc_num_outputs; out++)
    {
        for(buf = 0; buf < obj->msc_output[out].bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing output image handle, bufq = %d!\n", buf);
                status = vxReleaseImage(&obj->msc_output[out].image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing output image arr, bufq %d!\n", buf);
                status = vxReleaseObjectArray(&obj->msc_output[out].arr[buf]);
            }
        }
        status = vxReleaseUserDataObject(obj->msc_crop_obj + out);
    }

    return status;
}

// vx_status tiovx_fc_module_delete(TIOVXFCModuleObj *obj)
// {
//     vx_status status = VX_SUCCESS;

//     vx_int32 msc_num_outputs = obj->msc_num_outputs;
//     vx_int32 out;

//     if(obj->node != NULL)
//     {
//         TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing node reference!\n");
//         status = vxReleaseNode(&obj->node);
        
//         if (status != VX_SUCCESS) {
//             TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Error releasing node: %d\n", status);
//         }

//     return status;
//     }

//     for(out = 0; out < msc_num_outputs; out++)
//     {
//         if(obj->msc_write_node[out] != NULL)
//         {
//             if((vx_status)VX_SUCCESS == status)
//             {
//                 TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing write node [%d]!\n", out);
//                 status = vxReleaseNode(&obj->msc_write_node[out]);
//             }
//         }
//     }


//     if(((vx_status)VX_SUCCESS == status) && (obj->h3a_write_node != NULL))
//     {
//         TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing h3a write node reference!\n");
//         status = vxReleaseNode(&obj->h3a_write_node);
//     }

//     return status;
// }

vx_status tiovx_fc_module_delete(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "[tiovx_fc_module_delete] Entering tiovx_fc_module_delete\n");
    vx_status status = VX_SUCCESS;
    vx_int32 msc_num_outputs = obj->msc_num_outputs;
    vx_int32 out;
    
    // First, check if the node exists before trying to release it
    if(obj->node != NULL)
    {
        fprintf(stderr, "[FLEX-CONNECT-MODULE] Releasing node reference!\n");
        fprintf(stderr, "The node is: %p\n", obj->node);
        fprintf(stderr, "Releasing the node");

        status = vxReleaseNode(&obj->node);
        
        fprintf(stderr, "Node verification check\n");
        
        if (status != VX_SUCCESS) {
            fprintf(stderr, "[FLEX-CONNECT-MODULE] Error releasing node: %d\n", status);
        }
    
    }
    else
    {
        fprintf(stderr, "The node is NULL\n");
    }
    
    // Release all write nodes
    for(out = 0; out < msc_num_outputs; out++)
    {
        if(obj->msc_write_node[out] != NULL)
        {
            fprintf(stderr,"[MULTI-SCALER-MODULE] Releasing write node [%d]!\n", out);
            vx_status temp_status = vxReleaseNode(&obj->msc_write_node[out]);
            if(temp_status != VX_SUCCESS)
            {
                fprintf(stderr, "[FLEX-CONNECT-MODULE] Error releasing write node %d: %d\n", out, temp_status);
                // Update status only if it was previously successful
                if (status == VX_SUCCESS) {
                    status = temp_status;
                }
            }
            obj->msc_write_node[out] = NULL;
        }
        else
        {
            fprintf(stderr, "The msc_output_node is NULL\n");
        }
    }
    
    // Release H3A write node if it exists
    if(obj->h3a_write_node != NULL)
    {
        fprintf(stderr, "[FLEX-CONNECT-MODULE] Releasing h3a write node reference!\n");
        vx_status temp_status = vxReleaseNode(&obj->h3a_write_node);
        if(temp_status != VX_SUCCESS)
        {
            fprintf(stderr, "[FLEX-CONNECT-MODULE] Error releasing H3A write node: %d\n", temp_status);
            // Update status only if it was previously successful
            if (status == VX_SUCCESS) {
                status = temp_status;
            }
        }
        // Clear the reference regardless of status
        obj->h3a_write_node = NULL;
    }
    else
    {
        fprintf(stderr, "The h3a_write_node is NULL\n");
    }
    
    return status;
}

vx_status tiovx_fc_module_create(vx_graph graph, TIOVXFCModuleObj *obj, vx_object_array raw_image_arr, vx_object_array ae_awb_result_arr, const char* target_string)
{
    fprintf(stderr, "Entering tiovx_fc_module_create\n");
    vx_status status = VX_SUCCESS;

    tivx_raw_image viss_raw_image = NULL;
    vx_user_data_object ae_awb_result = NULL;
    // vx_user_data_object h3a_stats = NULL;
    vx_image msc_outputs[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS] = {NULL};

    if (graph == NULL) {
        fprintf(stderr, "[FC-ERROR] Input graph is NULL\n");
        return VX_ERROR_INVALID_REFERENCE;
    }

    fprintf(stderr, "[FC-MODULE-DEBUG] Starting module_create with num_outputs=%d\n", obj->msc_num_outputs);
    
    // Make sure color format is valid
    if (obj->color_format != VX_DF_IMAGE_NV12 && obj->color_format != VX_DF_IMAGE_U8) {
        fprintf(stderr, "[FC-WARNING] Invalid color_format: 0x%08X, forcing to NV12\n", obj->color_format);
        obj->color_format = VX_DF_IMAGE_NV12;
    }

    for (int i = 0; i < obj->msc_num_outputs; i++) {
        fprintf(stderr, "[FC-MODULE-DEBUG] Output[%d]: width=%d, height=%d, format=%d\n", 
                i, obj->msc_output[i].width, obj->msc_output[i].height, obj->color_format);
        fprintf(stderr, "[FC-MODULE-DEBUG] Output[%d] crop: start_x=%d, start_y=%d, width=%d, height=%d\n",
                i, obj->msc_crop_params[i].crop_start_x, obj->msc_crop_params[i].crop_start_y,
                obj->msc_crop_params[i].crop_width, obj->msc_crop_params[i].crop_height);
    }


    for(int i = 0; i < obj->msc_num_outputs; i++)
    {
        if(obj->msc_output_select[i] == TIOVX_FC_MODULE_OUTPUT_EN) 
        {   
        
            fprintf(stderr, "[FC-DEBUG] Checking output %d\n", i);
    
            // ✓ Validate the array exists first!
            if (obj->msc_output[i].arr[0] == NULL) {
                TIOVX_MODULE_ERROR("[FC-MODULE] Output %d array is NULL!\n", i);
                status = VX_ERROR_INVALID_REFERENCE;
                goto cleanup;
            }

            fprintf(stderr, "[FC-DEBUG] Output %d array pointer: %p\n", i, obj->msc_output[i].arr[0]);

        // Check if it's a valid object array
        vx_enum type = VX_TYPE_INVALID;
        status = vxQueryReference((vx_reference)obj->msc_output[i].arr[0], VX_REFERENCE_TYPE, &type, sizeof(type));
            if (status != VX_SUCCESS || type != VX_TYPE_OBJECT_ARRAY) {
                fprintf(stderr, "[FC-ERROR] Output %d arr[0] is not an object array (type=0x%x)\n", i, type);
                status = VX_ERROR_INVALID_REFERENCE;
                goto cleanup;
            }

        fprintf(stderr, "[FC-DEBUG] Output %d is valid object array, getting item\n", i);

        msc_outputs[i] = (vx_image)vxGetObjectArrayItem(obj->msc_output[i].arr[0], 0);

        fprintf(stderr, "[FC-DEBUG] MSC output %d item: %p\n", i, msc_outputs[i]);

            if(msc_outputs[i] == NULL) {
                TIOVX_MODULE_ERROR("[FC-MODULE] Failed to get MSC output %d item\n", i);
                status = VX_ERROR_INVALID_REFERENCE;
                goto cleanup;
            }

             // Verify the output format matches what we expect
            vx_df_image format = 0;
            status = vxQueryImage(msc_outputs[i], VX_IMAGE_FORMAT, &format, sizeof(format));
            if (status == VX_SUCCESS) {
                fprintf(stderr, "[FC-DEBUG] MSC output %d format: 0x%08X\n", i, format);
                
                // Check if format matches our expectation
                if (format != obj->color_format) {
                    fprintf(stderr, "[FC-WARNING] Output format (0x%08X) doesn't match expected (0x%08X)\n",
                            format, obj->color_format);
                }
            }
        } else {
            // This output not enabled
            msc_outputs[i] = NULL;

        }
    } 


    // raw image viss_input 
    if(raw_image_arr != NULL)
    {
        viss_raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
        fprintf(stderr, "Failed created viss_raw_image\n");
    }
    else
    {
        viss_raw_image = (tivx_raw_image)vxGetObjectArrayItem(obj->viss_input.arr[0], 0);
        fprintf(stderr, "Successfully created viss_raw_image %p\n", viss_raw_image);
    }
   
    if(ae_awb_result_arr != NULL) {
        ae_awb_result = (vx_user_data_object)vxGetObjectArrayItem(ae_awb_result_arr, 0);
    }
    
    // fprintf(stderr, "h3a_stats check 1\n");
    
    // // h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(obj->viss_h3a_stats_arr[0], 0);

    // // if (h3a_stats == NULL){
    // //     fprintf(stderr, "h3a_stats is NULL\n");
    // // }else{
    // //     fprintf(stderr, "The content of h3a_stats is %p, h3a_stats\n", h3a_stats);
    // // }

    // fprintf(stderr, "h3a_stats check 2\n");

    for(int i = 0; i < obj->msc_num_outputs; i++) {
        if(obj->msc_output_select[i] == TIOVX_FC_MODULE_OUTPUT_EN) {


            msc_outputs[i] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->msc_output[i].arr[0], 0);
           
            fprintf(stderr, "[module_configure_params] MSC output 0 item: %p\n", msc_outputs[i]);


            fprintf(stderr, "[FC-MSC-MODULE] MSC output %d item: %p\n", i, msc_outputs[i]);


            if(msc_outputs[i] == NULL) {
                TIOVX_MODULE_ERROR("[FC-MSC-MODULE] Failed to get MSC output %d item\n", i);

                status = VX_ERROR_INVALID_REFERENCE;
            }

        }
    }

    status = vxCopyUserDataObject(obj->fc_config, 0,
                                  sizeof(tivx_vpac_fc_viss_msc_params_t),
                                  &obj->fc_params,
                                  VX_WRITE_ONLY,
                                  VX_MEMORY_TYPE_HOST);
    if(status != VX_SUCCESS) {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Failed to update FC config parameters\n");
        goto cleanup;
    }else{
        fprintf(stderr, "Updated the FC config parameters\n");
    }

    fprintf(stderr, "Working 4\n");
    
    // Debug target string
    fprintf(stderr, "[FC-DEBUG] Using target string: %s\n", target_string ? target_string : "(null)");
    

    obj->node = tivxVpacFcVissMscNode(graph,
                                obj->fc_config,
                                NULL,
                                NULL,
                                viss_raw_image,
                                NULL, // viss_out0
                                NULL, // viss_out1 
                                NULL, // viss_out2
                                NULL, // viss_out3
                                NULL, // viss_h3a
                                NULL, // viss_hist0
                                NULL, // viss_hist1
                                NULL, // viss_raw_hist
                                msc_outputs[0], // msc_out0
                                NULL, // msc_out1
                                NULL, // msc_out2
                                NULL, // msc_out3
                                NULL, // msc_out4
                                NULL, // msc_out5
                                NULL, // msc_out6
                                NULL, // msc_out7
                                NULL, // msc_out8
                                NULL); // msc_out9
    fprintf(stderr, "Working 5\n");        

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        fprintf(stderr, "Node creation status is success.\n");

        // Don't set node target if target_string is NULL or empty
        if (target_string != NULL && target_string[0] != '\0') 
        {
            status = vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);
            
            if (status != VX_SUCCESS) 
            {
                fprintf(stderr, "[FC-ERROR] Failed to set node target '%s': %d\n", target_string, status);
            
            } else {
                fprintf(stderr, "[FC-DEBUG] Successfully set target to: %s\n", target_string);
            }
        } 
        else 
        {
            fprintf(stderr, "[FC-WARNING] No target string provided, using default\n");
        }

        status = vxSetReferenceName((vx_reference)obj->node, "flexconnect_node");

        if (status != VX_SUCCESS) {
            fprintf(stderr, "[FC-ERROR] Failed to set node name: %d\n", status);
        }

        fprintf(stderr, "The node is %p\n,",obj->node);
        
        vx_bool replicate[23];

        replicate[0] = vx_false_e;  
        replicate[1] = vx_false_e;  
        replicate[2] = vx_false_e;  
        
        replicate[3] = vx_true_e;   
        
        replicate[4] = vx_false_e;  
        replicate[5] = vx_false_e;  
        replicate[6] = vx_false_e;  
        replicate[7] = vx_false_e;  
        
        replicate[8] = vx_false_e; 
        
        replicate[9] = vx_false_e; 
        replicate[10] = vx_false_e; 
        replicate[11] = vx_false_e;
        
        replicate[12] = (obj->msc_output_select[0] == 1) ? vx_true_e : vx_false_e; // msc_out0
        replicate[13] = (obj->msc_output_select[1] == 1) ? vx_true_e : vx_false_e; // msc_out1
        replicate[14] = (obj->msc_output_select[2] == 1) ? vx_true_e : vx_false_e; // msc_out2
        replicate[15] = (obj->msc_output_select[3] == 1) ? vx_true_e : vx_false_e; // msc_out3
        replicate[16] = (obj->msc_output_select[4] == 1) ? vx_true_e : vx_false_e; // msc_out4
        replicate[17] = (obj->msc_output_select[5] == 1) ? vx_true_e : vx_false_e; // msc_out5
        replicate[18] = (obj->msc_output_select[6] == 1) ? vx_true_e : vx_false_e; // msc_out6
        replicate[19] = (obj->msc_output_select[7] == 1) ? vx_true_e : vx_false_e; // msc_out7
        replicate[20] = (obj->msc_output_select[8] == 1) ? vx_true_e : vx_false_e; // msc_out8
        replicate[21] = (obj->msc_output_select[9] == 1) ? vx_true_e : vx_false_e; // msc_out9
       
        
        // Set MSC output replication flags
        for (int i = 0; i < 10; i++) { // TIOVX_FC_MODULE_MAX_MSC_OUTPUTS or 10
            replicate[12 + i] = (obj->msc_output_select[i] == TIOVX_FC_MODULE_OUTPUT_EN && msc_outputs[i] != NULL) ? 
                                vx_true_e : vx_false_e;
            fprintf(stderr, "[FC-DEBUG] Setting replication for output %d to %s\n", 
                   i, replicate[12 + i] == vx_true_e ? "true" : "false");
        }
        
        status = vxReplicateNode(graph, obj->node, replicate, 22);
        fprintf(stderr, "check 1\n");

        if (status != VX_SUCCESS) {
            fprintf(stderr, "[FC-ERROR] Failed to replicate node: %d\n", status);
        }
    } else {
        fprintf(stderr, "check 2\n");
        fprintf(stderr, "[FC-ERROR] Unable to create FlexConnect Node: %d\n", status);
    }
    
    cleanup:
    fprintf(stderr, "check 3\n");
    vx_status original_status = status; 

    if (viss_raw_image != NULL) 
    {
        fprintf(stderr, "[FC-CLEANUP] Releasing viss_raw_image %p\n", viss_raw_image);
        tivxReleaseRawImage(&viss_raw_image);
    }
    else
    {
        fprintf(stderr, "The viss_raw_image is %p, unable to relaease", viss_raw_image);
    }

    if(ae_awb_result != NULL)
    {
        vxReleaseUserDataObject(&ae_awb_result);
    }

    // fprintf(stderr, "check 4\n");

    // vxReleaseUserDataObject(&h3a_stats);

    fprintf(stderr, "check 5\n");

    obj->en_out_write = 1;

    if(obj->en_out_write == 1)
    {
        fprintf(stderr, "[FC-DEBUG] Calling add_write_output_node with original status=%d\n", original_status);
        vx_status write_status = tiovx_fc_module_add_write_output_node(graph, obj, 0);
        fprintf(stderr, "[FC-DEBUG] add_write_output_node returned status=%d\n", write_status);
        if (original_status == VX_SUCCESS && write_status != VX_SUCCESS) {
            fprintf(stderr, "[FC-WARNING] write_output_node failed but preserving original success status\n");
        }
    }else {
        fprintf(stderr, "Skipping en_out_write (value = %d)\n", obj->en_out_write);
    }
    fprintf(stderr, "[FC-DEBUG] Final status before return: %d (original: %d)\n", status, original_status);
    fprintf(stderr, "End of tiovx_fc_module_create\n");
    return original_status;
}

vx_status tiovx_fc_module_release_buffers(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_release_buffers\n");
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    void *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32 bufq, ch;

    /* Free raw viss_input handles */
    for(bufq = 0; bufq < obj->viss_input.bufq_depth; bufq++)
    {
        for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->viss_input.arr[bufq], ch);
            status = vxGetStatus((vx_reference)ref);

            if((vx_status)VX_SUCCESS == status)
            {
                /* Export handles to get valid size information. */
                status = tivxReferenceExportHandle(ref,
                                                   virtAddr,
                                                   size,
                                                   TIOVX_MODULES_MAX_REF_HANDLES,
                                                   &numEntries);

                if((vx_status)VX_SUCCESS == status)
                {
                    vx_int32 ctr;

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        if(virtAddr[ctr] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[FC-MODULE] Freeing raw viss_input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[ctr], size[ctr]);
                            tivxMemFree(virtAddr[ctr], size[ctr], TIVX_MEM_EXTERNAL);
                        }
                    }

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        virtAddr[ctr] = NULL;
                    }

                    /* Assign NULL handles to the OpenVx objects as it will avoid
                        doing a tivxMemFree twice, once now and once during release */
                    status = tivxReferenceImportHandle(ref,
                                                    (const void **)virtAddr,
                                                    (const uint32_t *)size,
                                                    numEntries);
                }
                vxReleaseReference(&ref);
            }
        }
    }

    return status;
}

vx_status tiovx_fc_module_send_write_output_cmd(TIOVXFCModuleObj *obj, vx_int32 start_frame, vx_int32 num_frames, vx_int32 num_skip)
{
    fprintf(stderr, "Entering tiovx_fc_module_send_write_output_cmd\n");
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    for(vx_int32 out = 0; out < obj->msc_num_outputs; out++)
    {
        status = vxCopyUserDataObject(obj->msc_write_cmd[out], 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_reference refs[2];

            refs[0] = (vx_reference)obj->msc_write_cmd[out];

            status = tivxNodeSendCommand(obj->msc_write_node[out], TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                    TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                    refs, 1u);

            if(VX_SUCCESS != status)
            {
                TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] write node send command failed!\n");
            }

            TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] write node send command success!\n");
        }
    }

    return (status);
}


vx_status tiovx_fc_module_update_filter_coeffs(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_update_filter_coeffs\n");
    vx_status status = VX_SUCCESS;

    vx_reference refs[1];

    refs[0] = (vx_reference)obj->msc_coeff_obj;
    if((vx_status)VX_SUCCESS == status)
    {
        status = tivxNodeSendCommand(obj->node, 0u,
                                 TIVX_VPAC_FC_MSC_CMD_SET_COEFF,
                                 refs, 1u);

        TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] App Send MSC Command Done!\n");
    }

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Node send command failed!\n");
    }

    return status;
}

void tiovx_fc_module_crop_params_init( TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering ttiovx_fc_module_crop_params_init\n");
    vx_int32 out;

    for (out = 0; out < obj->msc_num_outputs; out++)
    {
        obj->msc_crop_params[out].crop_start_x = 0;
        obj->msc_crop_params[out].crop_start_y = 0;
        obj->msc_crop_params[out].crop_width = obj->viss_input.params.width;
        obj->msc_crop_params[out].crop_height = obj->viss_input.params.height;
    }


}

vx_status tiovx_fc_module_update_crop_params(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_update_crop_params\n");
    vx_status status = VX_SUCCESS;
    vx_reference refs[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    // vx_int32 out;

    // for (out = 0; out < obj->msc_num_outputs; out++)
    // {
    //     refs[out] = (vx_reference)obj->msc_crop_obj[out];
    // }    

    refs[0] = (vx_reference)obj->msc_crop_obj[0];
    
    status = tivxNodeSendCommand(obj->node, 0u,
                                TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS,
                                refs, 1u);

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[FLEXCONNECT-MODULE] Node send command TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS, failed!\n");
    }

    return status;
}

vx_status tiovx_fc_module_update_input_params(TIOVXFCModuleObj *obj)
{
    fprintf(stderr, "Entering tiovx_fc_module_update_input_params\n");
    vx_status status = VX_SUCCESS;
    vx_reference refs[1];

    refs[0] = (vx_reference)(obj->fc_input_prm_obj);
    status = tivxNodeSendCommand(obj->node, 0u,
                                 TIVX_VPAC_FC_MSC_CMD_SET_INPUT_PARAMS,
                                 refs, 1u);

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR(
                "[FLEX-CONNECT-MODULE] Node send command "
                "TIVX_VPAC_FC_MSC_CMD_SET_INPUT_PARAMS, failed!\n");
    }

    vxReleaseUserDataObject(&(obj->fc_input_prm_obj));

    return status;
}