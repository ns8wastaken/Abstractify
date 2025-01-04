#include <raylib.h>
#include <rlgl.h>
#include <cstdint>
#include <vector>


struct ComputeShader
{
    char* source;
    unsigned int shader;
    unsigned int program;

    std::vector<unsigned int> ssbos = {};

    ComputeShader(const char* fileName)
    {
        source  = LoadFileText(fileName);
        shader  = rlCompileShader(source, RL_COMPUTE_SHADER);
        program = rlLoadComputeShaderProgram(shader);
    }

    template <typename T>
    inline void AddSSBO(const T* data, size_t dataSize)
    {
        ssbos.push_back(rlLoadShaderBuffer(static_cast<unsigned int>(dataSize), data, RL_DYNAMIC_COPY));
    }

    template <typename T>
    inline void UpdateSSBO(unsigned int ssboIndex, const T* data, size_t dataSize)
    {
        if (ssboIndex < ssbos.size()) {
            rlUpdateShaderBuffer(ssbos[ssboIndex], data, static_cast<unsigned int>(dataSize), 0);
        }
    }

    inline void Unload()
    {
        for (const unsigned int& ssbo : ssbos) {
            rlUnloadShaderBuffer(ssbo);
        }
        UnloadFileText(source);
        rlUnloadShaderProgram(program);
    }
};
