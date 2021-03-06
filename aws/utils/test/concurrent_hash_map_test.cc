// Copyright 2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Amazon Software License (the "License").
// You may not use this file except in compliance with the License.
// A copy of the License is located at
//
//  http://aws.amazon.com/asl
//
// or in the "license" file accompanying this file. This file is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing
// permissions and limitations under the License.

#include <atomic>
#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

#include <aws/utils/logging.h>

#include <aws/utils/concurrent_hash_map.h>
#include <aws/mutex.h>

BOOST_AUTO_TEST_SUITE(ConcurrentHashMap)

BOOST_AUTO_TEST_CASE(ConcurrentInsert) {
  std::atomic<int> counter(0);

  aws::utils::ConcurrentHashMap<std::string, std::string> map([&](auto& k) {
    counter++;
    return new std::string("world");
  });

  std::chrono::high_resolution_clock::time_point start =
      std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(50);

  std::vector<std::string> results;
  aws::mutex mutex;

  const int num_threads = 16;
  std::vector<aws::thread> threads;
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back([&] {
      while (std::chrono::high_resolution_clock::now() < start) {
        aws::this_thread::yield();
      }
      auto s = map["hello"];
      aws::lock_guard<aws::mutex> lk(mutex);
      results.push_back(s);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  BOOST_CHECK_EQUAL(counter, 1);
  BOOST_CHECK_EQUAL(results.size(), num_threads);
  for (auto s : results) {
    BOOST_CHECK_EQUAL(s, "world");
  }
}

BOOST_AUTO_TEST_SUITE_END()
