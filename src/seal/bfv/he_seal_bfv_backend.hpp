//*****************************************************************************
// Copyright 2018 Intel Corporation
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
//*****************************************************************************

#pragma once

#include <memory>

#include "ngraph/runtime/backend.hpp"
#include "seal/he_seal_backend.hpp"
#include "seal/he_seal_parameter.hpp"
#include "seal/seal.h"
#include "seal/seal_ciphertext_wrapper.hpp"
#include "seal/seal_plaintext_wrapper.hpp"

namespace ngraph {
namespace runtime {
namespace he {
class HETensor;
class HEPlainTensor;
class HECipherTensor;
class HESealBackend;

namespace he_seal {
class HESealBFVBackend : public HESealBackend {
 public:
  HESealBFVBackend();
  HESealBFVBackend(
      const std::shared_ptr<runtime::he::he_seal::HESealParameter>& sp);
  HESealBFVBackend(HESealBFVBackend& he_backend) = default;
  ~HESealBFVBackend(){};

  std::shared_ptr<runtime::Tensor> create_batched_tensor(
      const element::Type& element_type, const Shape& shape) override;

  std::shared_ptr<seal::SEALContext> make_seal_context(
      const std::shared_ptr<runtime::he::he_seal::HESealParameter> sp)
      const override;

  void encode(std::shared_ptr<runtime::he::HEPlaintext>& output,
              const void* input, const element::Type& element_type,
              size_t count = 1) const override;
  void decode(void* output,
              const std::shared_ptr<runtime::he::HEPlaintext> input,
              const element::Type& element_type,
              size_t count = 1) const override;

  const inline std::shared_ptr<seal::FractionalEncoder> get_frac_encoder()
      const {
    return m_frac_encoder;
  }

  void assert_valid_seal_bfv_parameter(
      const std::shared_ptr<runtime::he::he_seal::HESealParameter>& sp) const;

  /// @brief Checks the noise budget of several tensor views
  ///        Throws an error if the noise budget is exhauasted
  ///        for any of the tensor views.
  void check_noise_budget(
      const std::vector<std::shared_ptr<runtime::he::HETensor>>& tvs) const;

  /// @brief Returns the remaining noise budget for a ciphertext.
  //         A noise budget of <= 0 indicate the ciphertext is no longer
  //         decryptable.
  int noise_budget(const std::shared_ptr<seal::Ciphertext>& ciphertext) const;

 private:
  std::shared_ptr<seal::FractionalEncoder> m_frac_encoder;
};
}  // namespace he_seal
}  // namespace he
}  // namespace runtime
}  // namespace ngraph