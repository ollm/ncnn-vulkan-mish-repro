#include <cmath>
#include <cstdio>

#include <command.h>
#include <gpu.h>
#include <layer.h>
#include <modelbin.h>
#include <option.h>
#include <paramdict.h>

int main()
{
    ncnn::create_gpu_instance();

    if (ncnn::get_gpu_count() == 0)
    {
        std::fprintf(stderr, "No Vulkan GPU is available.\n");
        ncnn::destroy_gpu_instance();
        return 2;
    }

    int result = 0;
    ncnn::VulkanDevice* vkdev = ncnn::get_gpu_device(0);
    ncnn::VkAllocator* blob_allocator = vkdev->acquire_blob_allocator();
    ncnn::VkAllocator* staging_allocator = vkdev->acquire_staging_allocator();

    ncnn::Option opt;
    opt.use_vulkan_compute = true;
    opt.use_fp16_storage = false;
    opt.use_fp16_packed = false;
    opt.use_fp16_arithmetic = false;
    opt.use_int8_storage = false;
    opt.use_int8_arithmetic = false;
    opt.blob_vkallocator = blob_allocator;
    opt.workspace_vkallocator = blob_allocator;
    opt.staging_vkallocator = staging_allocator;

    const float expected[] = {20.f, 80.f, 88.8f, 110.f};
    ncnn::Mat input(1, 4u * sizeof(float), 4);
    float* input_ptr = input;
    for (int i = 0; i < 4; i++)
        input_ptr[i] = expected[i];

    ncnn::Layer* mish = ncnn::create_layer_vulkan("Mish");
    if (!mish)
    {
        std::fprintf(stderr, "Failed to create Vulkan Mish layer.\n");
        result = 3;
    }
    else
    {
        mish->vkdev = vkdev;

        ncnn::ParamDict pd;
        ncnn::ModelBinFromMatArray model(0);
        if (mish->load_param(pd) != 0 || mish->load_model(model) != 0)
        {
            std::fprintf(stderr, "Failed to initialize Vulkan Mish layer.\n");
            result = 4;
        }
        else
        {
            mish->bottom_shapes.resize(1);
            mish->top_shapes.resize(1);
            mish->bottom_shapes[0] = input;
            mish->top_shapes[0] = input;

            if (mish->create_pipeline(opt) != 0)
            {
                std::fprintf(stderr, "Failed to create Vulkan Mish pipeline.\n");
                result = 5;
            }
            else
            {
                ncnn::Mat output;
                ncnn::VkMat input_gpu;
                ncnn::VkCompute cmd(vkdev);

                cmd.record_upload(input, input_gpu, opt);
                if (mish->forward_inplace(input_gpu, cmd, opt) != 0)
                {
                    std::fprintf(stderr, "Vulkan Mish forward failed.\n");
                    result = 6;
                }
                else
                {
                    cmd.record_download(input_gpu, output, opt);
                    if (cmd.submit_and_wait() != 0)
                    {
                        std::fprintf(stderr, "Vulkan command submission failed.\n");
                        result = 7;
                    }
                    else
                    {
                        const float* output_ptr = output;
                        for (int i = 0; i < 4; i++)
                        {
                            std::printf("input=%g output=%g\n", expected[i], output_ptr[i]);
                            if (!std::isfinite(output_ptr[i]) || std::fabs(output_ptr[i] - expected[i]) > 1e-4f)
                                result = 1;
                        }
                    }
                }

                mish->destroy_pipeline(opt);
            }
        }

        delete mish;
    }

    vkdev->reclaim_blob_allocator(blob_allocator);
    vkdev->reclaim_staging_allocator(staging_allocator);
    ncnn::destroy_gpu_instance();
    return result;
}
