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

static vx_status tiovx_fc_module_configure_dcc_params(vx_context context, TIOVXFCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->dcc_config = NULL;

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

    return status;
}

static vx_status tiovx_fc_module_create_viss_inputs(vx_context context, TIOVXFCModuleObj *obj)
{
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


static vx_status tiovx_fc_module_configure_scaler_coeffs(vx_context context, TIOVXFCModuleObj *obj)
{
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
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create scaler coeffs object! \n");
    }

    return status;
}

static vx_status tiovx_fc_module_configure_crop_params(vx_context context, TIOVXFCModuleObj *obj)
{
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
    vx_status status = VX_SUCCESS;
    vx_int32 out, buf;

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
        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->msc_output[out].arr[buf]  = NULL;
            obj->msc_output[out].image_handle[buf]  = NULL;
        }
    }

    for(out = 0; out < obj->msc_num_outputs; out++)
    {
        vx_image out_img;

        out_img = vxCreateImage(context, obj->msc_output[out].width, obj->msc_output[out].height, obj->color_format);
        status = vxGetStatus((vx_reference)out_img);

        if(status == VX_SUCCESS)
        {
            for(buf = 0; buf < obj->msc_output[out].bufq_depth; buf++)
            {
                obj->msc_output[out].arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->num_channels);

                status = vxGetStatus((vx_reference)obj->msc_output[out].arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create output array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "flex_connect_scaler_node_output_arr%d_buf%d", out, buf);

                    vxSetReferenceName((vx_reference)obj->msc_output[out].arr[buf], name);
                }

                obj->msc_output[out].image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->msc_output[out].arr[buf], 0);
            }
            vxReleaseImage(&out_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create output image template! \n");
            break;
        }
    }

    if(obj->en_multi_scalar_output == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_path); 
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)obj->file_path, "scaler_write_node_file_path");

            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create file path object for fileio!\n");
        }

        for(out = 0; out < obj->msc_num_outputs; out++)
        {
            char msc_file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

            sprintf(msc_file_prefix, "scaler_output_%d", out);
            obj->msc_file_prefix[out] = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
            status = vxGetStatus((vx_reference)obj->msc_file_prefix[out]);
            if(status == VX_SUCCESS)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_msc_file_prefix_%d", out);

                vxSetReferenceName((vx_reference)obj->msc_file_prefix[out], name);

                vxAddArrayItems(obj->msc_file_prefix[out], TIVX_FILEIO_FILE_PREFIX_LENGTH, &msc_file_prefix[0], 1);
            }
            else
            {
                TIOVX_MODULE_ERROR("[FLEX_CONNECT-MODULE-MODULE] Unable to create file prefix object for output %d!\n", out);
                break;
            }

            obj->msc_write_node[out] = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
            status = vxGetStatus((vx_reference)obj->msc_write_node[out]);
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
    vx_status status = VX_SUCCESS;

    obj->sensorObj = sensorObj;

    if(obj->sensorObj == NULL)
    {
        TIOVX_MODULE_ERROR("Sensor Object handle is NULL!");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_dcc_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_create_viss_inputs(context, obj);
    }
    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_configure_scaler_coeffs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_create_scaler_outputs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_update_crop_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_fc_module_update_input_params(context, obj);
    }

}

vx_status tiovx_fc_module_deinit(TIOVXFCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    if(((vx_status)VX_SUCCESS == status) && (obj->fc_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing config handle!\n");
        status = vxReleaseUserDataObject(&obj->fc_config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->dcc_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing DCC config handle!\n");
        status = vxReleaseUserDataObject(&obj->dcc_config);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->viss_ae_awb_result_bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing ae-awb result handle!\n");
                status = vxReleaseUserDataObject(&obj->viss_ae_awb_result_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing ae-awb result arr!\n");
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
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing raw viss_input image handle!\n");
                status = tivxReleaseRawImage(&obj->viss_input.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing raw viss_input image arr!\n");
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
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing h3a stats handle!\n");
                status = vxReleaseUserDataObject(&obj->viss_h3a_stats_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing h3a stats arr!\n");
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
}

vx_status tiovx_fc_module_delete(TIOVXFCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);

    return status;
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->h3a_write_node != NULL))
    {
        TIOVX_MODULE_PRINTF("[FLEX-CONNECT-MODULE] Releasing h3a write node reference!\n");
        status = vxReleaseNode(&obj->h3a_write_node);
    }

}

vx_status tiovx_fc_module_create(vx_graph graph, TIOVXFCModuleObj *obj, vx_object_array raw_image_arr, vx_object_array ae_awb_result_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    tivx_raw_image viss_raw_image = NULL;
    vx_user_data_object ae_awb_result = NULL;
    vx_user_data_object h3a_stats = NULL;
    vx_image viss_output0 = NULL;
    vx_image viss_output1 = NULL;
    vx_image viss_output2 = NULL;
    vx_image viss_output3 = NULL;
    vx_image msc_out0 = NULL;
    vx_image msc_out1 = NULL;
    vx_image msc_out2 = NULL;
    vx_image msc_out3 = NULL;
    vx_image msc_out4 = NULL;
    vx_image msc_out5 = NULL;
    vx_image msc_out6 = NULL;
    vx_image msc_out7 = NULL;
    vx_image msc_out8 = NULL;
    vx_image msc_out9 = NULL;


    // raw image viss_input 
    if(raw_image_arr != NULL)
    {
        viss_raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
    }
    else
    {
        viss_raw_image = (tivx_raw_image)vxGetObjectArrayItem(obj->viss_input.arr[0], 0);
    }

    // MSC output
    if(obj->msc_output_select[0] == TIOVX_FC_MODULE_OUTPUT_EN)
    {
        msc_out0 = (vx_image)vxGetObjectArrayItem(obj->msc_output->arr[0], 0);
    }
    
    // if(obj->msc_output_select[1] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out1 = (vx_image)vxGetObjectArrayItem(obj->msc_out1.arr[0], 0);
    // }
    // if(obj->msc_output_select[2] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out2 = (vx_image)vxGetObjectArrayItem(obj->msc_out2.arr[0], 0);
    // }
    // if(obj->msc_output_select[3] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out3 = (vx_image)vxGetObjectArrayItem(obj->msc_out3.arr[0], 0);
    // }
    // if(obj->msc_output_select[0] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out4 = (vx_image)vxGetObjectArrayItem(obj->msc_out4.arr[0], 0);
    // }
    // if(obj->msc_output_select[1] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out5 = (vx_image)vxGetObjectArrayItem(obj->msc_out5.arr[0], 0);
    // }
    // if(obj->msc_output_select[2] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out6 = (vx_image)vxGetObjectArrayItem(obj->msc_out6.arr[0], 0);
    // }
    // if(obj->msc_output_select[3] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out7 = (vx_image)vxGetObjectArrayItem(obj->msc_out7.arr[0], 0);
    // }
    //  if(obj->msc_output_select[2] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out8 = (vx_image)vxGetObjectArrayItem(obj->msc_out8.arr[0], 0);
    // }
    // if(obj->msc_output_select[3] == TIOVX_FC_MODULE_OUTPUT_EN)
    // {
    //     msc_out9 = (vx_image)vxGetObjectArrayItem(obj->msc_out9.arr[0], 0);
    // }


    obj->node = tivxVpacFcVissMscNode(graph,
                                    obj->fc_config,
                                    ae_awb_result,
                                    obj->dcc_config,
                                    viss_raw_image,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    msc_out0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
    

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);
        vxSetReferenceName((vx_reference)obj->node, "flexconnect_node");

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
        
        vxReplicateNode(graph, obj->node, replicate, 22);      
    }
    else 
    {
        TIOVX_MODULE_ERROR("[FLEX-CONNECT-MODULE] Unable to create Flexconect Node! \n");

    }
    vxReleaseImage(&viss_raw_image);
    if(ae_awb_result != NULL)
    {
        vxReleaseUserDataObject(&ae_awb_result);
    }
    vxReleaseUserDataObject(&h3a_stats);

    if(obj->en_out_write == 1)
    {
        status = tiovx_fc_module_add_write_output_node(graph, obj);
    }

    return status;
}

vx_status tiovx_fc_module_release_buffers(TIOVXFCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    void *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32 out, bufq, ch;

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


}

vx_status tiovx_fc_module_add_write_output_node(vx_graph graph, TIOVXFCModuleObj *obj)
{

    vx_status status = VX_SUCCESS;
    
    // Add nodes for MSC output

    for (int out = 0; out <= TIOVX_FC_MODULE_MAX_MSC_OUTPUTS; out++)
    {
        if (obj->viss_output_select[out] == TIOVX_FC_MODULE_OUTPUT_EN)
        {  
            vx_image output_img = (vx_image)vxGetObjectArrayItem(obj->msc_output[out].arr[0], 0);
            obj->msc_write_node[out] = tivxWriteImageNode(graph, output_img, obj->file_path, obj->msc_file_prefix[out]);
            vxReleaseImage(&output_img);

            status = vxGetStatus((vx_reference)obj->msc_write_node[out]);
            if((vx_status)VX_SUCCESS == status)
            {
                vxSetNodeTarget(obj->msc_write_node[out], VX_TARGET_STRING, TIVX_TARGET_MPU_0);

                vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
                vxReplicateNode(graph, obj->msc_write_node[out], replicate, 3);
            }
            else
            {
                TIOVX_MODULE_ERROR("[FLEXCONNECT-MODULE] Unable to create node to write msc output! \n");
            }

        }
        
    }

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



}

vx_status tiovx_fc_module_send_write_output_cmd(TIOVXFCModuleObj *obj, vx_int32 start_frame, vx_int32 num_frames, vx_int32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;
    vx_int32 out;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    for(out = 0; out < obj->msc_num_outputs; out++)
    {
        status = vxCopyUserDataObject(obj->msc_write_cmd[out], 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_reference refs[2];

            refs[0] = (vx_reference)obj->msc_write_cmd[out];

            status = tivxNodeSendCommand(obj->msc_write_cmd[out], TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
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

void tiovx_fc_module_set_coeff(tivx_vpac_msc_coefficients_t *coeff, uint32_t interpolation_method)
{
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

vx_status tiovx_fc_module_update_filter_coeffs(TIOVXFCModuleObj *obj)
{

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

    vx_status status = VX_SUCCESS;
    vx_reference refs[TIOVX_FC_MODULE_MAX_MSC_OUTPUTS];
    vx_int32 out;

    refs[0] = (vx_reference)obj->msc_coeff_obj;
    
    if((vx_status)VX_SUCCESS == status)
    {
        status = tivxNodeSendCommand(obj->node, 0u,
                                 TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS,
                                 refs, 1u);

        TIOVX_MODULE_PRINTF("[FLEXCONNECT-MODULE] Flexconnect Set Crop  Command Done!\n")
    }

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[FLEXCONNECT-MODULE] Node send command TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS, failed!\n");
    }

    return status;
}

vx_status tiovx_fc_module_update_input_params(TIOVXFCModuleObj *obj)
{
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