// Copyright 2009-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "concat_conv_chw.h"

OIDN_NAMESPACE_BEGIN

  ConcatConvCHW::ConcatConvCHW(const Ref<Engine>& engine, const ConcatConvDesc& desc)
    : ConcatConv(desc),
      engine(engine)
  {
    if (src1Desc.layout == TensorLayout::hwc)
      throw std::invalid_argument("unsupported concat+conv source layout");

    TensorDims srcDims{src1Desc.getC() + src2Desc.getC(), src1Desc.getH(), src1Desc.getW()};
    TensorDims srcPaddedDims{src1Desc.getPaddedC() + src2Desc.getPaddedC(), src1Desc.getH(), src1Desc.getW()};
    srcDesc = {srcDims, srcPaddedDims, src1Desc.layout, src1Desc.dataType};

    conv = engine->newConv({srcDesc, weightDesc, biasDesc, activation, PostOp::None});
  }

  void ConcatConvCHW::updateSrc()
  {
    if (src1->getBuffer() != src2->getBuffer() ||
        (static_cast<char*>(src1->getData()) + src1->getByteSize()) != static_cast<char*>(src2->getData()))
      throw std::invalid_argument("concat+conv sources are not pre-concatenated in memory");

    std::shared_ptr<Tensor> src;
    if (src1->getBuffer())
      src = src1->getBuffer()->newTensor(srcDesc, src1->getByteOffset());
    else
      src = engine->newTensor(srcDesc, src1->getData());
    
    conv->setSrc(src);
  }

OIDN_NAMESPACE_END