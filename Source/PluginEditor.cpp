/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PingPongDelayAudioProcessorEditor::PingPongDelayAudioProcessorEditor (PingPongDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible(del_L_Slider);
    del_L_Slider.setTextValueSuffix(" [ms]");
    del_L_Slider.addListener(this);
    del_L_Slider.setRange(0.0,2000.0);
    del_L_Slider.setValue(0.0);
    addAndMakeVisible(del_L_Label);
    del_L_Label.setText("Delay L", juce::dontSendNotification);
    del_L_Label.attachToComponent(&del_L_Slider, true);
    
    del_L_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"DEL_L",del_L_Slider);
    
    addAndMakeVisible(del_R_Slider);
    del_R_Slider.setTextValueSuffix(" [ms]");
    del_R_Slider.addListener(this);
    del_R_Slider.setRange(0.0,2000.0);
    del_R_Slider.setValue(0.0);
    addAndMakeVisible(del_R_Label);
    del_R_Label.setText("Delay R", juce::dontSendNotification);
    del_R_Label.attachToComponent(&del_R_Slider, true);
    
    del_R_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"DEL_R",del_R_Slider);
    
    addAndMakeVisible(feedback_L_Slider);
    feedback_L_Slider.setTextValueSuffix(" [-]");
    feedback_L_Slider.addListener(this);
    feedback_L_Slider.setRange(0.0,1.0);
    feedback_L_Slider.setValue(0.0);
    addAndMakeVisible(feedback_L_Label);
    feedback_L_Label.setText("Feedback L", juce::dontSendNotification);
    feedback_L_Label.attachToComponent(&feedback_L_Slider, true);
    
    feedback_L_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"FEEDBACK_L",feedback_L_Slider);
    
    addAndMakeVisible(feedback_R_Slider);
    feedback_R_Slider.setTextValueSuffix(" [-]");
    feedback_R_Slider.addListener(this);
    feedback_R_Slider.setRange(0.0,1.0);
    feedback_R_Slider.setValue(0.0);
    addAndMakeVisible(feedback_R_Label);
    feedback_R_Label.setText("Feedback R", juce::dontSendNotification);
    feedback_R_Label.attachToComponent(&feedback_R_Slider, true);
    
    feedback_R_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"FEEDBACK_R",feedback_R_Slider);
    
    addAndMakeVisible(drywet_Slider);
    drywet_Slider.setTextValueSuffix(" [-]");
    drywet_Slider.addListener(this);
    drywet_Slider.setRange(0.0,1.0);
    drywet_Slider.setValue(0.0);
    addAndMakeVisible(drywet_Label);
    drywet_Label.setText("Dry Wet", juce::dontSendNotification);
    drywet_Label.attachToComponent(&drywet_Slider, true);
    
    drywet_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"DRY_WET",drywet_Slider);
    
    addAndMakeVisible(vol_Slider);
    vol_Slider.setTextValueSuffix(" [-]");
    vol_Slider.addListener(this);
    vol_Slider.setRange(-20.0,20.0);
    vol_Slider.setValue(0.0);
    addAndMakeVisible(vol_Label);
    vol_Label.setText("Volume", juce::dontSendNotification);
    vol_Label.attachToComponent(&vol_Slider, true);
    
    vol_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"VOLUME",vol_Slider);
}

PingPongDelayAudioProcessorEditor::~PingPongDelayAudioProcessorEditor()
{
}

//==============================================================================
void PingPongDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void PingPongDelayAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;

    del_L_Slider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    del_R_Slider.setBounds(sliderLeft, 60, getWidth() - sliderLeft - 10, 20);
    feedback_L_Slider.setBounds(sliderLeft, 60+40, getWidth() - sliderLeft - 10, 20);
    feedback_R_Slider.setBounds(sliderLeft, 60+40+40, getWidth() - sliderLeft - 10, 20);
    drywet_Slider.setBounds(sliderLeft, 60+40+40+40, getWidth() - sliderLeft - 10, 20);
    vol_Slider.setBounds(sliderLeft, 60+40+40+40+40, getWidth() - sliderLeft - 10, 20);
    

}


void PingPongDelayAudioProcessorEditor::sliderValueChanged(Slider* slider)
{

    if (slider == &del_L_Slider)
    {
        audioProcessor.set_del_L_param(del_L_Slider.getValue());
    }
    else if (slider == &del_R_Slider)
    {
        audioProcessor.set_del_R_param(del_R_Slider.getValue());
    }
    else if (slider == &feedback_L_Slider)
    {
        audioProcessor.set_feedback_L_param(feedback_L_Slider.getValue());
    }
    else if (slider == &feedback_R_Slider)
    {
        audioProcessor.set_feedback_R_param(feedback_R_Slider.getValue());
    }
    else if (slider == &drywet_Slider)
    {
        audioProcessor.set_drywet_param(drywet_Slider.getValue());
    }
    else if (slider == &vol_Slider)
    {
        audioProcessor.set_gVolume_param(vol_Slider.getValue());
    }
}


