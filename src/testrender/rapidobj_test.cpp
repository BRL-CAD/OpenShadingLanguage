// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

#include "rapidobj/rapidobj.hpp"

#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/unittest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>


namespace {

void
check_file_read(const std::filesystem::path& path, size_t minimum_bytes)
{
    std::string source = "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
    while (source.size() < minimum_bytes)
        source += "# Padding to exercise file-reader block boundaries.\n";
    // Also exercise a partial final block without a trailing newline.
    source += "f 1 2 3";
    {
        std::ofstream output(path, std::ios::binary);
        output << source;
        OIIO_CHECK_ASSERT(output.good());
        if (!output.good())
            return;
    }

    const auto result = rapidobj::ParseFile(path);
    OIIO_CHECK_ASSERT(!result.error);
    if (result.error)
        return;

    const std::vector<float> positions { 0, 0, 0, 1, 0, 0, 0, 1, 0 };
    OIIO_CHECK_ASSERT(std::equal(result.attributes.positions.begin(),
                                 result.attributes.positions.end(),
                                 positions.begin(), positions.end()));
    OIIO_CHECK_EQUAL(result.shapes.size(), 1);
    if (result.shapes.size() != 1)
        return;

    const auto& mesh = result.shapes.front().mesh;
    OIIO_CHECK_EQUAL(mesh.num_face_vertices.size(), 1);
    OIIO_CHECK_EQUAL(mesh.indices.size(), 3);
    if (mesh.indices.size() == 3) {
        for (size_t i = 0; i < mesh.indices.size(); ++i)
            OIIO_CHECK_EQUAL(mesh.indices[i].position_index,
                             static_cast<int>(i));
    }
}

}  // namespace


int
main()
{
    const auto path    = std::filesystem::path(
                             OIIO::Filesystem::temp_directory_path())
                         / OIIO::Filesystem::unique_path("osl-obj-%%%%-%%%%.obj");
    const auto missing = rapidobj::ParseFile(path);
    OIIO_CHECK_ASSERT(missing.error);
    OIIO_CHECK_ASSERT(missing.error.code
                      == std::errc::no_such_file_or_directory);

    // Cross the actual reader thresholds so both sequential and parallel
    // reads remain covered if the vendored parser changes its block sizes.
    for (const size_t minimum_bytes :
         { size_t(0), rapidobj::detail::kBlockSize + 1,
           rapidobj::detail::kSingleThreadCutoff + rapidobj::detail::kBlockSize
               + 1 }) {
        check_file_read(path, minimum_bytes);
    }
    OIIO_CHECK_ASSERT(std::filesystem::remove(path));
    return unit_test_failures;
}
