///*
//  ==============================================================================
//
//    MixerControl.cpp
//    Created: 18 Nov 2024 12:08:38pm
//    Author:  Leif Rehtanz
//
//  ==============================================================================
//*/
//
//#include "MixerControl.h"
//
//MixerControl::MixerControl()
//{
//    addAndMakeVisible(mixSlider);
//    mixSlider.setRange(0.0, 1.0);
//    mixSlider.setValue(0.5);
//    mixSlider.setTextValueSuffix(" Mix");
//    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
//    mixSlider.setPopupDisplayEnabled(true, false, this);
//    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
//}
//
//float MixerControl::getMixLevel() const
//{
//    return mixSlider.getValue();
//}
//
//void MixerControl::resized()
//{
//    auto area = getLocalBounds().reduced(10);
//    mixSlider.setBounds(area);
//}
