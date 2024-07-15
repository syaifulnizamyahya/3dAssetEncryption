// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>

#include "tiny_gltf.h"

static std::string GetFilePathExtension(const std::string& FileName) {
    if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
    return "";
}

void printVertices(const tinygltf::Model& model, int first) {
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const tinygltf::Accessor& accessor =
                    model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& bufferView =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* positions = reinterpret_cast<const float*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                size_t vertexCount = accessor.count;

                std::cout << "Mesh: " << mesh.name
                    << ", Primitive: " << primitive.material << std::endl;
                if (first == 0) {
                    for (size_t i = 0; i < vertexCount; ++i) {
                        float x = positions[i * 3 + 0];
                        float y = positions[i * 3 + 1];
                        float z = positions[i * 3 + 2];
                        std::cout << "Vertex " << i << ": (" << x << ", " << y << ", " << z
                            << ")" << std::endl;
                    }
                }
                else {
                    for (size_t i = 0; i < first; ++i) {
                        float x = positions[i * 3 + 0];
                        float y = positions[i * 3 + 1];
                        float z = positions[i * 3 + 2];
                        std::cout << "Vertex " << i << ": (" << x << ", " << y << ", " << z
                            << ")" << std::endl;
                    }
                }
            }
        }
    }
}

void printVertices(const tinygltf::Model& model) { printVertices(model, 0); }

// Function to obfuscate vertex positions using XOR with a pseudo-random number
// pros
// 1. bitwise op is fast
// cons
// 1. totally deformed and blown object. unpredictable
// 2. gltf metadata warning
void obfuscateVertices(tinygltf::Model& model, unsigned int seed) {
    // std::default_random_engine generator(seed);
    // Using Mersen Twister random func. Should be platform agnostic (havent find
    // any proof though). Alternatively use pcg.
    // https://www.pcg-random.org/index.html Or use any deterministic, platform
    // agnostic random method.
    std::mt19937 generator(seed);
    std::uniform_int_distribution<uint32_t> distribution;

    for (auto& mesh : model.meshes) {
        for (auto& primitive : mesh.primitives) {
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                tinygltf::Accessor& accessor =
                    model.accessors[primitive.attributes["POSITION"]];
                tinygltf::BufferView& bufferView =
                    model.bufferViews[accessor.bufferView];
                tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                // Casting to standard uint32, not float
                uint32_t* positions = reinterpret_cast<uint32_t*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                // not really vertex count. More like all vertex component (vertex count
                // * 3)
                size_t vertexCount = accessor.count * accessor.type;

                // FOR EACH VERTEX COMPONENT
                for (size_t i = 0; i < vertexCount; ++i) {
                    // if (i < 5) {
                    //   std::cout << "Before pos:" << positions[i]
                    //             << " val:" << std::bit_cast<float >(positions[i])
                    //             << " rand:" << distribution(generator) << std::endl;
                    // }
                    // PERFORM XOR AGAINST RANDOM NUMBER
                    positions[i] ^= distribution(generator);
                    // if (i < 5) {
                    //   std::cout << "After  pos:" << positions[i]
                    //             << " val:" << std::bit_cast<float >(positions[i])
                    //             << " rand:" << distribution(generator) << std::endl;
                    // }
                }
            }
        }
    }
}

// Vertex index random reordering
void shuffleVertices(tinygltf::Model& model, unsigned int seed) {
    std::mt19937 generator(seed);

    for (auto& mesh : model.meshes) {
        for (auto& primitive : mesh.primitives) {
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                tinygltf::Accessor& accessor =
                    model.accessors[primitive.attributes["POSITION"]];
                tinygltf::BufferView& bufferView =
                    model.bufferViews[accessor.bufferView];
                tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                float* positions = reinterpret_cast<float*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                size_t vertexCount = accessor.count;
                // Store permutation of vertex index
                std::vector<size_t> permutation(vertexCount);
                // Fill with 0, 1, 2, ..., vertexCount-1. As correct index.
                // permutation.at(1) = 1 etc
                std::iota(permutation.begin(), permutation.end(), 0);

                std::vector<float> tempPositions(vertexCount * 3);
                // Suffle randomly the index based on random generator.
                // permutation.at(1) = 224 etc
                // std::shuffle(permutation.begin(), permutation.end(), generator);
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                // Copy the vertex component indexed in permutation to new vertex
                // index(tempPositions)
                for (size_t i = 0; i < vertexCount; ++i) {
                    tempPositions[i * 3 + 0] = positions[permutation[i] * 3 + 0];
                    tempPositions[i * 3 + 1] = positions[permutation[i] * 3 + 1];
                    tempPositions[i * 3 + 2] = positions[permutation[i] * 3 + 2];
                }

                std::copy(tempPositions.begin(), tempPositions.end(), positions);
            }
        }
    }
}

// Reverse the vertex shuffling using the stored permutation indices
void reverseShuffleVertices(tinygltf::Model& model, unsigned int seed) {
    std::mt19937 generator(seed);
    for (auto& mesh : model.meshes) {
        for (auto& primitive : mesh.primitives) {
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                tinygltf::Accessor& accessor =
                    model.accessors[primitive.attributes["POSITION"]];
                tinygltf::BufferView& bufferView =
                    model.bufferViews[accessor.bufferView];
                tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                float* positions = reinterpret_cast<float*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                size_t vertexCount = accessor.count;
                std::vector<size_t> permutation(vertexCount);
                std::iota(permutation.begin(), permutation.end(),
                    0);  // Fill with 0, 1, 2, ..., vertexCount-1

                // std::shuffle(permutation.begin(), permutation.end(), generator);
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                std::vector<float> tempPositions(vertexCount * 3);
                // Reverse shuffle. Copy index in vertex index (position) back to
                // correct vertex index.
                for (size_t i = 0; i < vertexCount; ++i) {
                    tempPositions[permutation[i] * 3 + 0] = positions[i * 3 + 0];
                    tempPositions[permutation[i] * 3 + 1] = positions[i * 3 + 1];
                    tempPositions[permutation[i] * 3 + 2] = positions[i * 3 + 2];
                }

                std::copy(tempPositions.begin(), tempPositions.end(), positions);
            }
        }
    }
}

int main(int argc, char** argv) {
    // 1. take 3 argument
    //     a. input
    //     b. encrypted
    //     c. decrypted
    // 2. open input file
    // 3. encrypt and save
    // 4. decrypt and save

    if (argc < 3) {
        printf("Needs input.gltf\n");
        exit(1);
    }

    // Store original JSON string for `extras` and `extensions`
    bool store_original_json_for_extras_and_extensions = false;
    if (argc > 2) {
        store_original_json_for_extras_and_extensions = true;
    }

    tinygltf::Model model;
    tinygltf::TinyGLTF gltf_ctx;
    tinygltf::TinyGLTF encrypted;
    tinygltf::TinyGLTF decrypted;

    std::string err;
    std::string warn;
    std::string input_filename(argv[1]);
    std::string encrypted_filename(argv[2]);
    std::string decrypted_filename(argv[3]);
    std::string ext = GetFilePathExtension(input_filename);

    gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(
        store_original_json_for_extras_and_extensions);

    bool ret = false;
    if (ext.compare("glb") == 0) {
        std::cout << "Reading binary glTF" << std::endl;
        // assume binary glTF.
        ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn,
            input_filename.c_str());
    }
    else {
        std::cout << "Reading ASCII glTF" << std::endl;
        // assume ascii glTF.
        ret =
            gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, input_filename.c_str());
    }

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    std::cout << "Original" << std::endl;
    printVertices(model, 5);

    int key = 1337;
    // ENCRYPT
    obfuscateVertices(model, key);
    // shuffleVertices(model, key);
    std::cout << "Encrypted" << std::endl;
    printVertices(model, 5);

    ret = encrypted.WriteGltfSceneToFile(&model, encrypted_filename, true, true,
        true, false);
    if (!ret) {
        std::cerr << "Failed to write glTF file: " << encrypted_filename
            << std::endl;
        return -1;
    }

    std::cout << "Successfully encrypt and saved glTF file: "
        << encrypted_filename << std::endl;

    // DECRYPT
    obfuscateVertices(model, key);
    // reverseShuffleVertices(model, key);
    std::cout << "Decrypted" << std::endl;
    printVertices(model, 5);

    ret = decrypted.WriteGltfSceneToFile(&model, decrypted_filename, true, true,
        true, false);
    if (!ret) {
        std::cerr << "Failed to write glTF file: " << decrypted_filename
            << std::endl;
        return -1;
    }

    std::cout << "Successfully decrypt and saved glTF file: "
        << decrypted_filename << std::endl;

    system("pause");

    return 0;
}
