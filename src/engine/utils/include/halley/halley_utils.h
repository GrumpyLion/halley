#pragma once

namespace Halley {} // Get GitHub to realise this is C++ :3

#include "audio/resampler.h"

#include "concurrency/concurrent.h"
#include "concurrency/shared_recursive_mutex.h"
#include "concurrency/task.h"
#include "concurrency/task_anchor.h"
#include "concurrency/task_set.h"

#include "bytes/byte_serializer.h"
#include "bytes/compression.h"
#include "bytes/config_node_serializer.h"
#include "bytes/fuzzer.h"

#include "data_structures/bin_pack.h"
#include "data_structures/config_database.h"
#include "data_structures/config_node.h"
#include "data_structures/dynamic_grid.h"
#include "data_structures/hash_map.h"
#include "data_structures/mapped_pool.h"
#include "data_structures/maybe.h"
#include "data_structures/maybe_ref.h"
#include "data_structures/memory_pool.h"
#include "data_structures/nullable_reference.h"
#include "data_structures/priority_queue.h"
#include "data_structures/rect_spatial_checker.h"
#include "data_structures/ring_buffer.h"
#include "data_structures/selection_set.h"
#include "data_structures/tree_map.h"
#include "data_structures/vector.h"

#include "file/directory_monitor.h"
#include "file/path.h"

#include "file_formats/binary_file.h"
#include "file_formats/config_file.h"
#include "file_formats/image.h"
#include "file_formats/ini_reader.h"
#include "file_formats/json_file.h"
#include "file_formats/text_file.h"
#include "file_formats/text_reader.h"
#include "file_formats/xml_file.h"
#include "file_formats/yaml_convert.h"

#include "maths/angle.h"
#include "maths/base_transform.h"
#include "maths/bezier.h"
#include "maths/box.h"
#include "maths/circle.h"
#include "maths/colour.h"
#include "maths/line.h"
#include "maths/matrix4.h"
#include "maths/polygon.h"
#include "maths/ops.h"
#include "maths/quaternion.h"
#include "maths/random.h"
#include "maths/range.h"
#include "maths/ray.h"
#include "maths/rect.h"
#include "maths/rolling_data_set.h"
#include "maths/simd.h"
#include "maths/sphere.h"
#include "maths/triangle.h"
#include "maths/tween.h"
#include "maths/vector2.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
#include "maths/uuid.h"

#include "os/os.h"

#include "navigation/navmesh.h"
#include "navigation/navmesh_generator.h"
#include "navigation/navmesh_set.h"
#include "navigation/navigation_query.h"
#include "navigation/navigation_path.h"
#include "navigation/navigation_path_follower.h"
#include "navigation/world_position.h"

#include "plugin/plugin.h"

#include "resources/metadata.h"
#include "resources/resource.h"
#include "resources/resource_data.h"

#include "support/assert.h"
#include "support/console.h"
#include "support/debug.h"
#include "support/exception.h"
#include "support/logger.h"
#include "support/redirect_stream.h"
#include "support/profiler.h"

#include "text/encode.h"
#include "text/fuzzy_text_matcher.h"
#include "text/halleystring.h"
#include "text/i18n.h"
#include "text/string_converter.h"
#include "text/string_serializer.h"

#include "time/halleytime.h"
#include "time/stopwatch.h"

#include "utils/algorithm.h"
#include "utils/attributes.h"
#include "utils/averaging.h"
#include "utils/encrypt.h"
#include "utils/hash.h"
#include "utils/scoped_guard.h"
#include "utils/type_traits.h"
#include "utils/utils.h"
#include "utils/variable.h"
