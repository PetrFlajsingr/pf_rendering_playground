//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "mvc/Model.h"
#include "mvc/reactive.h"
#include "audio/Buffer.h"
#include "gpu/Buffer.h"

namespace pf {

class AudioAssetModel : public SavableModel {
 public:
  Observable<bool> enablePlayback;
  Observable<std::shared_ptr<audio::Buffer>> audioBuffer;
  Observable<std::shared_ptr<gpu::Buffer>> gpuBuffer;
};

class AudioAssetsModel : public SavableModel {
 public:

};

}  // namespace pf

