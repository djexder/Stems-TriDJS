#include "AudioAnalyzer.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>
#include <cstring>

AnalysisResult AudioAnalyzer::analyze(const float* audioData, int numSamples,
                                       int sampleRate, int numChannels)
{
    AnalysisResult result;

    if (audioData == nullptr || numSamples < sampleRate || sampleRate <= 0)
        return result;

    // Convert to mono
    int numFrames = numSamples / numChannels;
    std::vector<float> mono(numFrames, 0.0f);
    for (int i = 0; i < numFrames; ++i)
    {
        float sum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            sum += audioData[i * numChannels + ch];
        mono[i] = sum / (float)numChannels;
    }

    // Use first 60 seconds for analysis (or whole track if shorter)
    int analysisLen = std::min((int)mono.size(), sampleRate * 60);
    std::vector<float> analysisData(mono.begin(), mono.begin() + analysisLen);

    result.bpm = detectBPM(analysisData, sampleRate);
    result.bpmDetected = result.bpm > 0.0;

    result.key = detectKey(analysisData, sampleRate);
    result.keyDetected = !result.key.empty();

    return result;
}

//==============================================================================
// BPM Detection
//==============================================================================

std::vector<float> AudioAnalyzer::computeOnsetEnvelope(
    const std::vector<float>& audio, int sampleRate)
{
    int frameSize = 2048;
    int hopSize = 512;

    if ((int)audio.size() < frameSize)
        return {};

    int numFrames = ((int)audio.size() - frameSize) / hopSize + 1;
    std::vector<float> energy(numFrames, 0.0f);

    for (int i = 0; i < numFrames; ++i)
    {
        int start = i * hopSize;
        float sum = 0.0f;
        for (int j = 0; j < frameSize; ++j)
            sum += audio[start + j] * audio[start + j];
        energy[i] = sum / (float)frameSize;
    }

    // Onset = positive half-wave rectified energy difference
    std::vector<float> onset(numFrames, 0.0f);
    for (int i = 1; i < numFrames; ++i)
    {
        float diff = energy[i] - energy[i - 1];
        onset[i] = diff > 0.0f ? diff : 0.0f;
    }

    return onset;
}

std::vector<float> AudioAnalyzer::autocorrelation(
    const std::vector<float>& signal, int maxLag)
{
    int n = (int)signal.size();
    int lagLimit = std::min(maxLag, n - 1);
    std::vector<float> result(lagLimit + 1, 0.0f);

    for (int lag = 1; lag <= lagLimit; ++lag)
    {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n - lag; ++i)
        {
            sum += (double)signal[i] * (double)signal[i + lag];
            ++count;
        }
        result[lag] = count > 0 ? (float)(sum / count) : 0.0f;
    }

    return result;
}

double AudioAnalyzer::detectBPM(const std::vector<float>& mono, int sampleRate)
{
    auto onset = computeOnsetEnvelope(mono, sampleRate);
    if (onset.size() < 50)
        return 0.0;

    // Frame rate of onset envelope = sampleRate / hopSize
    double frameRate = (double)sampleRate / 512.0;

    // Search lags corresponding to BPM range 60-200
    int minLag = std::max(10, (int)std::round(frameRate * 60.0 / 200.0));
    int maxLag = std::min((int)onset.size() - 1,
                          (int)std::round(frameRate * 60.0 / 40.0));
    if (minLag >= maxLag)
        return 0.0;

    auto ac = autocorrelation(onset, maxLag);
    if (ac.size() <= (size_t)minLag)
        return 0.0;

    // Collect all local peaks in valid BPM range
    struct Peak { int lag; float value; };
    std::vector<Peak> peaks;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        if (lag >= (int)ac.size())
            break;

        bool isPeak = true;
        for (int j = -2; j <= 2; ++j)
        {
            if (j == 0) continue;
            int idx = lag + j;
            if (idx >= 0 && idx < (int)ac.size() && ac[idx] >= ac[lag])
            {
                isPeak = false;
                break;
            }
        }

        if (isPeak)
            peaks.push_back({lag, ac[lag]});
    }

    if (peaks.empty())
        return 0.0;

    // Sort by strength descending
    std::sort(peaks.begin(), peaks.end(),
              [](const Peak& a, const Peak& b) { return a.value > b.value; });

    int bestLag = peaks[0].lag;
    float bestVal = peaks[0].value;

    // Check if there's a strong peak at roughly half this lag (= double BPM)
    // This prevents picking up the half-time sub-harmonic
    for (const auto& p : peaks)
    {
        if (p.lag >= bestLag)
            continue;
        if (p.lag > bestLag / 3 && p.lag < bestLag / 1.4 && p.value > bestVal * 0.5f)
        {
            bestLag = p.lag;
            bestVal = p.value;
            break;
        }
    }

    return std::round(60.0 * frameRate / (double)bestLag);
}

//==============================================================================
// Key Detection
//==============================================================================

std::vector<double> AudioAnalyzer::computeChroma(
    const std::vector<float>& audio, int sampleRate)
{
    const int fftSize = 4096;
    const int hopSize = 2048;

    if ((int)audio.size() < fftSize)
        return {};

    int order = (int)std::log2(fftSize);
    juce::dsp::FFT fft(order);

    int numFrames = ((int)audio.size() - fftSize) / hopSize + 1;
    if (numFrames <= 0)
        numFrames = 1;

    // Chroma bins 0=C, 1=C#, ..., 11=B
    std::vector<double> chroma(12, 0.0);
    int frameCount = 0;

    // Hanning window
    double pi = juce::MathConstants<double>::pi;
    std::vector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5 * (1.0 - std::cos(2.0 * pi * i / (fftSize - 1)));

    // For performance, sample only a subset of frames if too many
    int step = std::max(1, numFrames / 200);
    double totalMagnitude = 0.0;

    for (int frame = 0; frame < numFrames; frame += step)
    {
        int start = frame * hopSize;

        // Prepare FFT buffers using JUCE Complex type
        std::vector<juce::dsp::Complex<float>> input(fftSize);
        std::vector<juce::dsp::Complex<float>> output(fftSize);

        for (int i = 0; i < fftSize && start + i < (int)audio.size(); ++i)
            input[i] = juce::dsp::Complex<float>((float)(audio[start + i] * window[i]), 0.0f);

        fft.perform(input.data(), output.data(), false);

        // Map FFT bins to chroma bins
        for (int bin = 0; bin <= fftSize / 2; ++bin)
        {
            double mag = std::sqrt(
                (double)output[bin].real() * (double)output[bin].real() +
                (double)output[bin].imag() * (double)output[bin].imag());

            if (mag < 1e-10)
                continue;

            double freq = (double)bin * (double)sampleRate / (double)fftSize;

            // Limit to musical range: ~65Hz (C2) to ~2093Hz (C7)
            if (freq < 65.0 || freq > 2100.0)
                continue;

            // Convert frequency to MIDI note number
            double midiNote = 12.0 * std::log2(freq / 440.0) + 69.0;
            if (midiNote < 0.0 || midiNote > 127.0)
                continue;

            // Fractional chroma bin with linear interpolation
            double chromaIdx = std::fmod(midiNote, 12.0);
            if (chromaIdx < 0.0)
                chromaIdx += 12.0;

            int idxLow = (int)std::floor(chromaIdx);
            int idxHigh = (idxLow + 1) % 12;
            double frac = chromaIdx - (double)idxLow;

            chroma[idxLow] += mag * (1.0 - frac);
            chroma[idxHigh] += mag * frac;
            totalMagnitude += mag;
        }

        ++frameCount;
    }

    // Normalize
    if (totalMagnitude > 0.0 && frameCount > 0)
    {
        for (int i = 0; i < 12; ++i)
            chroma[i] /= totalMagnitude;
    }

    return chroma;
}

std::string AudioAnalyzer::detectKey(const std::vector<float>& mono,
                                      int sampleRate)
{
    auto chroma = computeChroma(mono, sampleRate);
    if (chroma.empty())
        return {};

    // Krumhansl-Schmuckler key profiles (normalized)
    const double majorProfile[12] = {
        6.35, 2.23, 3.48, 2.33, 4.38, 4.09,
        2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
    const double minorProfile[12] = {
        6.33, 2.68, 3.52, 5.38, 2.60, 3.53,
        2.54, 4.75, 3.98, 2.69, 3.34, 3.17};

    const char* noteNames[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"};

    double bestCorrelation = -1e10;
    int bestKey = 0;
    bool bestIsMajor = true;

    for (int isMajor = 0; isMajor < 2; ++isMajor)
    {
        const double* profile = isMajor ? majorProfile : minorProfile;

        for (int keyShift = 0; keyShift < 12; ++keyShift)
        {
            double correlation = 0.0;
            for (int i = 0; i < 12; ++i)
            {
                int chromaIndex = (i + keyShift) % 12;
                correlation += chroma[chromaIndex] * profile[i];
            }

            if (correlation > bestCorrelation)
            {
                bestCorrelation = correlation;
                bestKey = keyShift;
                bestIsMajor = isMajor == 1;
            }
        }
    }

    std::string keyName = noteNames[bestKey];
    if (!bestIsMajor)
        keyName += "m";

    return keyName;
}
