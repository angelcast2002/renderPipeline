#include "shaders.hpp"
#include <vector>
#include <array>

glm::vec3 vertexShader(const glm::vec3& vertex, const Uniforms& uniforms) {
    // Apply transformations to the input vertex using the matrices from the uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex, 1.0f);

    // Perspective divide
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Apply the viewport transform
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);

    // Return the transformed vertex as a vec3
    return glm::vec3(screenVertex);
}

std::vector<std::vector<glm::vec3>> primitiveAssembly(
        const std::vector<glm::vec3>& transformedVertices
) {
    std::vector<std::vector<glm::vec3>> triangles;

    // We will group the transformed vertices in sets of 3 to form triangles
    for (size_t i = 0; i < transformedVertices.size(); i += 3) {
        std::vector<glm::vec3> triangle;
        triangle.push_back(transformedVertices[i]);
        triangle.push_back(transformedVertices[i + 1]);
        triangle.push_back(transformedVertices[i + 2]);
        triangles.push_back(triangle);
    }

    return triangles;
}

std::vector<Fragment> rasterize(const std::vector<std::vector<glm::vec3>>& assembledVertices) {
    std::vector<Fragment> fragments;

    for (const std::vector<glm::vec3>& triangleVertices : assembledVertices) {
        std::vector<Fragment> triangleFragments = triangle(triangleVertices[0], triangleVertices[1], triangleVertices[2]);
        fragments.insert(fragments.end(), triangleFragments.begin(), triangleFragments.end());
    }

    return fragments;
}