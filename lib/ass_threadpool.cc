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

#include "ass_threadpool.h"

namespace ass {

void ThreadPool::LoadJob(const std::function<void()>& job) {
  std::unique_lock<std::mutex> lock(mtx_);
  jobs_.push(job);
  lock.unlock();
  mtx_condition_.notify_one();
}

void ThreadPool::Join() {
  std::unique_lock<std::mutex> lock(mtx_);
  should_stop_ = true;
  lock.unlock();
  mtx_condition_.notify_all();
  for (auto& thread : threads_) {
    thread.join();
  }
}

void ThreadPool::RunJobs() {
  std::function<void()> job;
  while (true) {
    std::unique_lock<std::mutex> lock(mtx_);
    mtx_condition_.wait(lock,
                        [this] { return !jobs_.empty() || should_stop_; });
    if (should_stop_ && jobs_.empty()) {
      return;
    }
    job = jobs_.front();
    jobs_.pop();
    job();
  }
}

}  // namespace ass