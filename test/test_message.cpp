//*****************************************************************************
// Copyright 2018-2019 Intel Corporation
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

#include <chrono>
#include <memory>

#include "gtest/gtest.h"
#include "he_plaintext.hpp"
#include "seal/seal.h"
#include "seal/seal_util.hpp"
#include "tcp/tcp_message.hpp"
#include "util/all_close.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

using namespace std;
using namespace ngraph;

TEST(tcp_message, save_cipher) {
  using namespace seal;

  EncryptionParameters parms(scheme_type::CKKS);
  size_t poly_modulus_degree = 8192;
  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(
      CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));
  auto context = SEALContext::Create(parms);

  KeyGenerator keygen(context);
  auto public_key = keygen.public_key();
  auto secret_key = keygen.secret_key();
  auto relin_keys = keygen.relin_keys();

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);
  CKKSEncoder ckks_encoder(context);

  vector<double> in_vals{0.0, 1.1, 2.2, 3.3};

  Plaintext plain;
  double scale = pow(2.0, 40);
  ckks_encoder.encode(in_vals, scale, plain);

  Ciphertext encrypted;
  encryptor.encrypt(plain, encrypted);

  size_t n = 1;
  std::vector<seal::Ciphertext> seal_ciphers(n);
  for (size_t i = 0; i < n; ++i) {
    seal_ciphers[i] = encrypted;
  }

  ngraph::he::TCPMessage message(ngraph::he::MessageType::execute,
                                 seal_ciphers);

  EXPECT_EQ(message.count(), n);

  for (size_t i = 0; i < n; ++i) {
    seal::Ciphertext cipher;
    ngraph::he::HEPlaintext plain;

    auto t1 = std::chrono::high_resolution_clock::now();
    message.load_cipher(cipher, i, context);
    auto t2 = std::chrono::high_resolution_clock::now();
    NGRAPH_INFO << "load time "
                << std::chrono::duration_cast<std::chrono::microseconds>(t2 -
                                                                         t1)
                       .count()
                << "us";

    ngraph::he::decrypt(plain, cipher, false, decryptor, ckks_encoder);

    auto out_vals = plain.values();
    out_vals = std::vector<double>{out_vals.begin(),
                                   out_vals.begin() + in_vals.size()};

    EXPECT_TRUE(test::all_close(out_vals, in_vals, 1e-3, 1e-3));
  }
}