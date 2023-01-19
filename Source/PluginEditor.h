/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PingPongDelayAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           public juce::Slider::Listener
{
public:
    PingPongDelayAudioProcessorEditor (PingPongDelayAudioProcessor&);
    ~PingPongDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void sliderValueChanged (Slider* slider) override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PingPongDelayAudioProcessor& audioProcessor;

    
    
    Slider del_L_Slider;
    Label del_L_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> del_L_SliderAttachment;

    Slider del_R_Slider;
    Label del_R_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> del_R_SliderAttachment;

    Slider feedback_L_Slider;
    Label feedback_L_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedback_L_SliderAttachment;

    Slider feedback_R_Slider;
    Label feedback_R_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedback_R_SliderAttachment;
    
    Slider drywet_Slider;
    Label drywet_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> drywet_SliderAttachment;
    
    Slider vol_Slider;
    Label vol_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vol_SliderAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PingPongDelayAudioProcessorEditor)
};
