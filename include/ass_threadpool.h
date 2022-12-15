/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  assfonts is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with assfonts. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  written by wyzdwdz (https://github.com/wyzdwdz)
 */

#ifndef ASSFONTS_ASSTHREADPOOL_H_
#define ASSFONTS_ASSTHREADPOOL_H_

#include <functional>
#include <queue>
#include <vector>

#include <boost/thread.hpp>

namespace ass {

class ThreadPool {
 public:
  ThreadPool(const unsigned int num) {
    threads_.resize(num);
    for (unsigned int i = 0; i < num; ++i) {
      threads_[i] = boost::thread([this]() { RunJobs(); });
    }
  }
  ThreadPool() : ThreadPool(boost::thread::hardware_concurrency()) {}
  ~ThreadPool() = default;

  void LoadJob(const std::function<void()>& job);
  void Join();

 private:
  std::vector<boost::thread> threads_;
  std::queue<std::function<void()>> jobs_;
  boost::mutex mtx_;
  boost::condition_variable mtx_condition_;
  bool should_stop_ = false;

  void RunJobs();
};

}  // namespace ass

#endif