/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "fiemap_writer.h"

namespace android {
namespace fiemap_writer {

// Wrapper around FiemapWriter that is able to split images across files if
// necessary.
class SplitFiemap final {
  public:
    using ProgressCallback = std::function<bool(uint64_t, uint64_t)>;

    // Create a new split fiemap file. If |max_piece_size| is 0, the number of
    // pieces will be determined automatically by detecting the filesystem.
    // Otherwise, the file will be split evenly (with the remainder in the
    // final file).
    static std::unique_ptr<SplitFiemap> Create(const std::string& file_path, uint64_t file_size,
                                               uint64_t max_piece_size,
                                               ProgressCallback progress = {});

    // Open an existing split fiemap file.
    static std::unique_ptr<SplitFiemap> Open(const std::string& file_path);

    ~SplitFiemap();

    // Return a list of all files created for a split file.
    static bool GetSplitFileList(const std::string& file_path, std::vector<std::string>* list);

    // Destroy all components of a split file. If the root file does not exist,
    // this returns true and does not report an error.
    static bool RemoveSplitFiles(const std::string& file_path, std::string* message = nullptr);

    const std::vector<struct fiemap_extent>& extents();
    uint32_t block_size() const;
    uint64_t size() const { return total_size_; }

    // Non-copyable & Non-movable
    SplitFiemap(const SplitFiemap&) = delete;
    SplitFiemap& operator=(const SplitFiemap&) = delete;
    SplitFiemap& operator=(SplitFiemap&&) = delete;
    SplitFiemap(SplitFiemap&&) = delete;

  private:
    SplitFiemap() = default;
    void AddFile(FiemapUniquePtr&& file);

    bool creating_ = false;
    std::string list_file_;
    std::vector<FiemapUniquePtr> files_;
    std::vector<struct fiemap_extent> extents_;
    uint64_t total_size_ = 0;
};

}  // namespace fiemap_writer
}  // namespace android