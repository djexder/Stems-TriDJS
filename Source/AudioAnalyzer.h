#pragma once

#include <JuceHeader.h>
#include <string>
#include <vector>

struct AnalysisResult
{
    double bpm = 0.0;
    std::string key;
    bool bpmDetected = false;
    bool keyDetected = false;
};

class AudioAnalyzer
{
public:
    AnalysisResult analyze(const float* audioData, int numSamples, int sampleRate, int numChannels);

private:
    double detectBPM(const std::vector<float>& mono, int sampleRate);
    std::string detectKey(const std::vector<float>& mono, int sampleRate);

    std::vector<float> computeOnsetEnvelope(const std::vector<float>& audio, int sampleRate);
    std::vector<float> autocorrelation(const std::vector<float>& signal, int maxLag);
    std::vector<double> computeChroma(const std::vector<float>& audio, int sampleRate);
};
