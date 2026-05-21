#pragma once

#include <optional>
#include <cstddef>

#include "../common/frame.h"

std::vector<segment_desc_t> build_desc(const std::string& trajectory_path);

// Reads only the first frame of a trajectory (any chemfiles-supported format)
// and returns its atom count. Returns std::nullopt if the file cannot be read.
// Used to sanity-check a description against the actual trajectory.
std::optional<size_t> probe_trajectory_n_atoms(const std::string& trajectory_path);
