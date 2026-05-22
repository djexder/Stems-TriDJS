#include "StemEngine.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdio>

#ifdef _MSC_VER
#include <intrin.h>
#endif
#define NOMINMAX

#ifdef _WIN32
#include <windows.h>
#endif

// Include LibTorch
#include <torch/script.h>
#include <torch/torch.h>

// Include Audio Decoders
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

namespace fs = std::filesystem;

// The concrete implementation containing standard PyTorch objects
struct StemEngine::Impl {
    torch::jit::script::Module model;
    torch::Device device{torch::kCPU};
};

StemEngine& StemEngine::getInstance() {
    static StemEngine instance;
    return instance;
}

void StemEngine::initialize() {
    if (initialized) return;
    currentStatus = Status::Booting;
    
    #ifdef _WIN32
    // Elevate CPU and Thread priority during early boot for faster DLL/model parsing
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    #endif

    impl = new Impl();
    initialized = true;
}

void StemEngine::loadModel(const std::string& modelPath) {
    if (!initialized) initialize();
    if (modelLoaded) return;

    currentStatus = Status::ModelLoading;

    try {
        // Prevent PyTorch gradients accumulation
        torch::NoGradGuard no_grad;
        
        // Limit multithreading to avoid choking host CPU cores
        torch::set_num_threads(4);
        torch::set_num_interop_threads(4);

        impl->model = torch::jit::load(modelPath);
        
        // Forcando CPU: o modelo exportado tem constantes (ex: window do STFT)
        // que ficam presas na CPU mesmo quando o modelo � movido para CUDA
        impl->device = torch::Device(torch::kCPU);
        impl->model.to(impl->device);

        modelLoaded = true;
    } catch (const std::exception& e) {
        errorMessage = e.what();
        currentStatus = Status::Failed;
        initialized = false;
        modelLoaded = false;
    }
}

void StemEngine::warmupCUDA() {
    if (!modelLoaded || warmedUp) return;
    
    currentStatus = Status::WarmingUp;
    
    try {
        torch::NoGradGuard no_grad;
        
        // Run a single dummy pass of 7.8 seconds of silence to compile CUDA kernels and claim VRAM
        auto options = torch::TensorOptions().dtype(torch::kFloat32);
        torch::Tensor dummy_chunk = torch::zeros({1, 2, 343980}, options).to(impl->device);
        
        std::vector<torch::jit::IValue> dummy_inputs;
        dummy_inputs.push_back(dummy_chunk);
        
        impl->model.forward(dummy_inputs).toTensor();
        
        warmedUp = true;
        currentStatus = Status::Ready;
        
        #ifdef _WIN32
        // Restore CPU/Thread priority to normal so host OS and JUCE main thread run smoothly
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
        #endif
    } catch (const std::exception& e) {
        errorMessage = e.what();
        currentStatus = Status::Failed;
    }
}

void StemEngine::shutdown() {
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
    initialized = false;
    modelLoaded = false;
    warmedUp = false;
    currentStatus = Status::Idle;
}

std::string StemEngine::getStatusString() const {
    switch (currentStatus) {
        case Status::Idle:         return "IDLE";
        case Status::Booting:      return "BOOTING";
        case Status::ModelLoading: return "MODEL_LOADING";
        case Status::WarmingUp:    return "WARMING_UP";
        case Status::Ready:        return "READY";
        case Status::Failed:       return "FAILED: " + errorMessage;
    }
    return "UNKNOWN";
}

bool StemEngine::processTrack(const std::string& inputPath, 
                              const std::string& outputDir, 
                              std::function<void(float)> progressCallback,
                              std::function<bool()> shouldCancelCallback) {
    if (!modelLoaded) return false;
    
    try {
        torch::NoGradGuard no_grad;
        const std::vector<std::string> STEM_NAMES = {"drums", "bass", "other", "vocals"};
        
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }

        // Read entire file into memory first (handle Unicode paths on Windows)
        #ifdef _WIN32
            int wide_len = MultiByteToWideChar(CP_UTF8, 0, inputPath.c_str(), -1, nullptr, 0);
            std::wstring wide_path(wide_len, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, inputPath.c_str(), -1, &wide_path[0], wide_len);
            wide_path.resize(wide_len - 1);
            FILE* f = _wfopen(wide_path.c_str(), L"rb");
        #else
            FILE* f = fopen(inputPath.c_str(), "rb");
        #endif
        if (!f) return false;

        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> file_data(fsize);
        if (fread(file_data.data(), 1, fsize, f) != (size_t)fsize) {
            fclose(f);
            return false;
        }
        fclose(f);

        // Determine file extension manually
        std::string ext;
        auto dot_pos = inputPath.rfind('.');
        auto slash_pos = inputPath.find_last_of("/\\");
        if (dot_pos != std::string::npos && (slash_pos == std::string::npos || dot_pos > slash_pos))
            ext = inputPath.substr(dot_pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        bool is_mp3 = (ext == ".mp3");

        // Decode from memory
        unsigned int channels = 0;
        unsigned int sample_rate = 0;
        drwav_uint64 total_pcm_frame_count = 0;
        float* sample_data = nullptr;

        if (is_mp3) {
            drmp3_config config;
            drmp3_uint64 mp3_frames;
            sample_data = drmp3_open_memory_and_read_pcm_frames_f32(
                file_data.data(), file_data.size(), &config, &mp3_frames, NULL);
            if (sample_data) {
                channels = config.channels;
                sample_rate = config.sampleRate;
                total_pcm_frame_count = mp3_frames;
            }
        } else {
            sample_data = drwav_open_memory_and_read_pcm_frames_f32(
                file_data.data(), file_data.size(), &channels, &sample_rate, &total_pcm_frame_count, NULL);
        }

        if (sample_data == nullptr) {
            return false;
        }

        if (shouldCancelCallback && shouldCancelCallback()) {
            if (is_mp3) drmp3_free(sample_data, nullptr);
            else drwav_free(sample_data, nullptr);
            return false;
        }

        // Transform floating arrays into LibTorch Tensors
        auto options = torch::TensorOptions().dtype(torch::kFloat32);
        torch::Tensor audio_tensor = torch::from_blob(
            sample_data, 
            {static_cast<long>(total_pcm_frame_count), static_cast<long>(channels)}, 
            options
        ).clone();
        
        if (is_mp3) {
            drmp3_free(sample_data, nullptr);
        } else {
            drwav_free(sample_data, nullptr);
        }

        audio_tensor = audio_tensor.transpose(0, 1);
        if (channels == 1) {
            audio_tensor = audio_tensor.repeat({2, 1}); // stereo
        }
        audio_tensor = audio_tensor.unsqueeze(0).to(impl->device);

        const int chunk_size = 343980;
        int total_frames = audio_tensor.size(2);
        
        // Prepare intermediate audio tensors
        torch::Tensor output_tensor = torch::zeros({4, 2, total_frames}, torch::kFloat32).to(impl->device);
        
        if (progressCallback) progressCallback(0.0f);
        
        // Chunks inference loop
        for (int start = 0; start < total_frames; start += chunk_size) {
            if (shouldCancelCallback && shouldCancelCallback()) {
                return false;
            }

            int end = std::min(start + chunk_size, total_frames);
            int current_chunk_size = end - start;
            
            torch::Tensor chunk = audio_tensor.slice(2, start, end);
            
            if (current_chunk_size < chunk_size) {
                chunk = torch::nn::functional::pad(chunk, torch::nn::functional::PadFuncOptions({0, chunk_size - current_chunk_size}).mode(torch::kConstant).value(0.0));
            }
            
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(chunk);
            
            torch::Tensor chunk_out = impl->model.forward(inputs).toTensor();
            chunk_out = chunk_out.squeeze(0);
            
            output_tensor.slice(2, start, end) = chunk_out.slice(2, 0, current_chunk_size);
            
            float percent = (float)end / (float)total_frames;
            if (progressCallback) progressCallback(percent);
        }
        
        output_tensor = output_tensor.cpu();

        if (shouldCancelCallback && shouldCancelCallback()) {
            return false;
        }

        // Export individual wav stems files
        auto slash_pos2 = inputPath.find_last_of("/\\");
        auto dot_pos2 = inputPath.rfind('.');
        auto name_start = (slash_pos2 == std::string::npos) ? 0 : slash_pos2 + 1;
        auto name_end = (dot_pos2 != std::string::npos && dot_pos2 > slash_pos2) ? dot_pos2 : inputPath.length();
        std::string base_name = inputPath.substr(name_start, name_end - name_start);

        for (int i = 0; i < 4; ++i) {
            if (shouldCancelCallback && shouldCancelCallback()) {
                return false;
            }

            torch::Tensor stem_tensor = output_tensor[i].transpose(0, 1).contiguous();
            
            std::string out_file = base_name + "_" + STEM_NAMES[i] + ".wav";
            std::string full_out_path = outputDir + "/" + out_file;

            drwav_data_format format;
            format.container = drwav_container_riff;
            format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
            format.channels = 2;
            format.sampleRate = sample_rate;
            format.bitsPerSample = 32;

            drwav wav;
            #ifdef _WIN32
                int wout_len = MultiByteToWideChar(CP_UTF8, 0, full_out_path.c_str(), -1, nullptr, 0);
                std::wstring wout_path(wout_len, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, full_out_path.c_str(), -1, &wout_path[0], wout_len);
                wout_path.resize(wout_len - 1);
                bool wrote = drwav_init_file_write_w(&wav, wout_path.c_str(), &format, NULL);
            #else
                bool wrote = drwav_init_file_write(&wav, full_out_path.c_str(), &format, NULL);
            #endif
            if (wrote) {
                drwav_write_pcm_frames(&wav, stem_tensor.size(0), stem_tensor.data_ptr<float>());
                drwav_uninit(&wav);
            }
        }

        if (progressCallback) progressCallback(1.0f);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "In-process inference failed: " << e.what() << std::endl;
        return false;
    }
}
