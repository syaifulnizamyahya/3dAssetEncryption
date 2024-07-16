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
                int count = (first > 0) ? first : vertexCount;
                for (size_t i = 0; i < count; ++i) {
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

void printVertices(const tinygltf::Model& model) { printVertices(model, 0); }

// Perform XOR to vertex component
void obfuscateVertices(tinygltf::Model& model, unsigned int seed) {
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

                uint32_t* positions = reinterpret_cast<uint32_t*>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                size_t vertexComponentCount = accessor.count * accessor.type;

                // For each vertex component (x, y, z)
                for (size_t i = 0; i < vertexComponentCount; ++i) {
                    // Perform XOR with random
                    positions[i] ^= distribution(generator);
                }
            }
        }
    }
}

// Shuffle vertices
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

                // Storage for permutation of vertex index
                std::vector<size_t> permutation(vertexCount);

                // Fill with 0, 1, 2, ..., vertexCount-1. As correct index.
                // permutation.at(1) = 1 etc
                std::iota(permutation.begin(), permutation.end(), 0);

                std::vector<float> tempPositions(vertexCount * 3);

                // Suffle vertex index based on random generator.
                // permutation.at(1) = 224 etc
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                // Swap index in permutation to new vertex
                // vertex.at(1) = newVertex.at(224)
                for (size_t i = 0; i < vertexCount; ++i) {
                    tempPositions[i * 3 + 0] = positions[permutation[i] * 3 + 0];
                    tempPositions[i * 3 + 1] = positions[permutation[i] * 3 + 1];
                    tempPositions[i * 3 + 2] = positions[permutation[i] * 3 + 2];
                }

                std::move(tempPositions.begin(), tempPositions.end(), positions);
            }
        }
    }
}

// Reverse the vertex shuffling 
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

                // Storage for permutation of vertex index
                std::vector<size_t> permutation(vertexCount);

                // Fill with 0, 1, 2, ..., 
                // permutation.at(1) = 1 etc
                std::iota(permutation.begin(), permutation.end(), 0);

                // Suffle vertex index based on random generator.
                // permutation.at(1) = 224 etc
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                std::vector<float> tempPositions(vertexCount * 3);
                // Reverse shuffle. Copy index in vertex index (position) back to
                // correct vertex index.
                for (size_t i = 0; i < vertexCount; ++i) {
                    tempPositions[permutation[i] * 3 + 0] = positions[i * 3 + 0];
                    tempPositions[permutation[i] * 3 + 1] = positions[i * 3 + 1];
                    tempPositions[permutation[i] * 3 + 2] = positions[i * 3 + 2];
                }

                std::move(tempPositions.begin(), tempPositions.end(), positions);
            }
        }
    }
}

void shuffleVertexComponents(tinygltf::Model& model, unsigned int seed) {
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

                size_t vertexComponentCount = accessor.count * accessor.type;

                // Storage for permutation of vertex index
                std::vector<size_t> permutation(vertexComponentCount);

                // Fill with 0, 1, 2, ..., vertexCount-1. As correct index.
                // permutation.at(1) = 1 etc
                std::iota(permutation.begin(), permutation.end(), 0);

                std::vector<float> tempPositions(vertexComponentCount);

                // Suffle vertex index based on random generator.
                // permutation.at(1) = 224 etc
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                // For each vertex component (x, y, z)
                for (size_t i = 0; i < vertexComponentCount; ++i) {
                    // Swap vertex component 
                    tempPositions[i] = positions[permutation[i]];
                }

                std::move(tempPositions.begin(), tempPositions.end(), positions);
            }
        }
    }
}

// Reverse the vertex shuffling 
void reverseShuffleVertexComponents(tinygltf::Model& model, unsigned int seed) {
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

                size_t vertexComponentCount = accessor.count * accessor.type;

                // Storage for permutation of vertex index
                std::vector<size_t> permutation(vertexComponentCount);

                // Fill with 0, 1, 2, ..., 
                // permutation.at(1) = 1 etc
                std::iota(permutation.begin(), permutation.end(), 0);

                // Suffle vertex index based on random generator.
                // permutation.at(1) = 224 etc
                std::ranges::shuffle(permutation.begin(), permutation.end(), generator);

                std::vector<float> tempPositions(vertexComponentCount);
                // Reverse shuffle. Copy index in vertex index (position) back to
                // correct vertex index.
                for (size_t i = 0; i < vertexComponentCount; ++i) {
                    tempPositions[permutation[i]] = positions[i];
                }

                std::move(tempPositions.begin(), tempPositions.end(), positions);
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
        printf("Usage : \n");
        printf("App.exe reference.gltf encrypted.gltf decrypted.gltf\n");
        printf("Example usage:\n");
        printf("ConsoleApplication1.exe models\\Duck\\glTF\\Duck.gltf models\\Duck\\glTF\\_EncryptedDuck.gltf models\\Duck\\glTF\\_DecryptedDuck.gltf\n");
        exit(1);
    }

    // Store original JSON string for `extras` and `extensions`
    bool store_original_json_for_extras_and_extensions = false;
    if (argc > 2) {
        store_original_json_for_extras_and_extensions = true;
    }

    tinygltf::Model model;
    tinygltf::TinyGLTF reference;
    tinygltf::TinyGLTF encrypted;
    tinygltf::TinyGLTF decrypted;

    std::string err;
    std::string warn;
    std::string reference_filename(argv[1]);
    std::string encrypted_filename(argv[2]);
    std::string decrypted_filename(argv[3]);
    std::string ext = GetFilePathExtension(reference_filename);

    reference.SetStoreOriginalJSONForExtrasAndExtensions(
        store_original_json_for_extras_and_extensions);

    bool ret = false;
    if (ext.compare("glb") == 0) {
        std::cout << "Reading binary glTF" << std::endl;
        // assume binary glTF.
        ret = reference.LoadBinaryFromFile(&model, &err, &warn,
            reference_filename.c_str());
    }
    else {
        std::cout << "Reading ASCII glTF" << std::endl;
        // assume ascii glTF.
        ret =
            reference.LoadASCIIFromFile(&model, &err, &warn, reference_filename.c_str());
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

    // print original
    std::cout << "Original" << std::endl;
    printVertices(model, 5);

    int key = 1337;

    // 3 encryption method.
    // uncomment one only. 
    // remember to select correct decryption method

    //obfuscateVertices(model, key);
    shuffleVertices(model, key);
    //shuffleVertexComponents(model, key);

    //print encrypted
    std::cout << "Encrypted" << std::endl;
    printVertices(model, 5);

    //write encrypted model
    ret = encrypted.WriteGltfSceneToFile(&model, encrypted_filename, true, true,
        true, false);
    if (!ret) {
        std::cerr << "Failed to write glTF file: " << encrypted_filename
            << std::endl;
        return -1;
    }
    std::cout << "Successfully encrypt and saved glTF file: "
        << encrypted_filename << std::endl;

    // 3 decryption method.
    // uncomment one only. 
    // select correct decryption method based on encryption method used

    //obfuscateVertices(model, key);
    reverseShuffleVertices(model, key);
    //reverseShuffleVertexComponents(model, key);

    //print decrypted
    std::cout << "Decrypted" << std::endl;
    printVertices(model, 5);

    //write decrypted model
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
