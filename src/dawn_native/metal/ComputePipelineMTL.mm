// Copyright 2017 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn_native/metal/ComputePipelineMTL.h"

#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"

namespace dawn_native { namespace metal {

    // static
    ResultOrError<Ref<ComputePipeline>> ComputePipeline::Create(
        Device* device,
        const ComputePipelineDescriptor* descriptor) {
        Ref<ComputePipeline> pipeline = AcquireRef(new ComputePipeline(device, descriptor));
        DAWN_TRY(pipeline->Initialize(descriptor));
        return pipeline;
    }

    MaybeError ComputePipeline::Initialize(const ComputePipelineDescriptor* descriptor) {
        auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

        ShaderModule* computeModule = ToBackend(descriptor->computeStage.module);
        const char* computeEntryPoint = descriptor->computeStage.entryPoint;
        ShaderModule::MetalFunctionData computeData;
        DAWN_TRY(computeModule->CreateFunction(computeEntryPoint, SingleShaderStage::Compute,
                                               ToBackend(GetLayout()), &computeData));

        NSError* error = nullptr;
        mMtlComputePipelineState.Acquire([mtlDevice
            newComputePipelineStateWithFunction:computeData.function.Get()
                                          error:&error]);
        if (error != nullptr) {
            NSLog(@" error => %@", error);
            return DAWN_INTERNAL_ERROR("Error creating pipeline state");
        }

        // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
        Origin3D localSize = GetStage(SingleShaderStage::Compute).metadata->localWorkgroupSize;
        mLocalWorkgroupSize = MTLSizeMake(localSize.x, localSize.y, localSize.z);

        mRequiresStorageBufferLength = computeData.needsStorageBufferLength;
        return {};
    }

    void ComputePipeline::Encode(id<MTLComputeCommandEncoder> encoder) {
        [encoder setComputePipelineState:mMtlComputePipelineState.Get()];
    }

    MTLSize ComputePipeline::GetLocalWorkGroupSize() const {
        return mLocalWorkgroupSize;
    }

    bool ComputePipeline::RequiresStorageBufferLength() const {
        return mRequiresStorageBufferLength;
    }

}}  // namespace dawn_native::metal
