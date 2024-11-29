///*
//  ==============================================================================
//
//    Algorithms.cpp
//    Created: 18 Nov 2024 1:55:04pm
//    Author:  Leif Rehtanz
//
//  ==============================================================================
//*/
//
//#include "Algorithms.h"
//#include <JuceHeader.h>
//
//// Implement the Amplitude Threshold Algorithm
//void amplitudeThresholdAlgorithm(juce::AudioBuffer<float>& buffer)
//{
//    const float threshold = 0.2f; // Amplitude threshold
//    const int numChannels = buffer.getNumChannels();
//    const int numSamples = buffer.getNumSamples();
//
//    for (int channel = 0; channel < numChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer(channel);
//
//        for (int i = 0; i < numSamples; ++i)
//        {
//            if (std::abs(channelData[i]) < threshold)
//                channelData[i] = 0.0f; // Zero-out values below the threshold
//        }
//    }
//}
//
//// Implement the Spectral Analysis Algorithm
//void spectralAnalysisAlgorithm(juce::AudioBuffer<float>& buffer)
//{
//    const int fftOrder = 10; // 2^10 = 1024
//    const int fftSize = 1 << fftOrder; // 1024
//    const float sampleRate = 44100.0f;
//    const float sibilantMinFreq = 5000.0f; // Lower range of sibilance
//    const float sibilantMaxFreq = 10000.0f; // Upper range of sibilance
//
//    juce::dsp::FFT fft(fftOrder);
//    juce::HeapBlock<juce::dsp::Complex<float>> fftBuffer(fftSize);
//
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* channelData = buffer.getWritePointer(channel);
//
//        for (int i = 0; i + fftSize <= buffer.getNumSamples(); i += fftSize)
//        {
//            // Copy data into FFT buffer as complex numbers (imaginary parts zero)
//            for(int j = 0; j < fftSize; ++j)
//            {
//                fftBuffer[j].real() = channelData[i + j];
//                fftBuffer[j].imag() = 0.0f;
//            }
//
//            // Perform forward FFT
//            fft.performFFT(reinterpret_cast<float*>(fftBuffer.get()));
//
//            // Analyze frequency bins for energy in sibilant range
//            int startBin = juce::roundToInt(sibilantMinFreq / sampleRate * fftSize);
//            int endBin = juce::roundToInt(sibilantMaxFreq / sampleRate * fftSize);
//
//            for (int bin = startBin; bin < endBin; ++bin)
//            {
//                // Example condition: suppress frequencies with low magnitude
//                float magnitude = std::sqrt(fftBuffer[bin].real() * fftBuffer[bin].real() +
//                                            fftBuffer[bin].imag() * fftBuffer[bin].imag());
//                if (magnitude < 0.2f)
//                {
//                    fftBuffer[bin].real() = 0.0f;
//                    fftBuffer[bin].imag() = 0.0f;
//                }
//            }
//
//            // Perform inverse FFT
//            fft.performInverseFFT(reinterpret_cast<float*>(fftBuffer.get()));
//
//            // Copy data back
//            for(int j = 0; j < fftSize; ++j)
//            {
//                channelData[i + j] = fftBuffer[j].real(); // Disregard imaginary parts
//            }
//        }
//    }
//}
