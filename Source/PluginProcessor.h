/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PingPongDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PingPongDelayAudioProcessor();
    ~PingPongDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // FOR PARAMETERS !
    juce::AudioProcessorValueTreeState apvts;
    
    
    void set_del_L_param(float val)
    {
        del_L_param = val;
    }
    void set_del_R_param(float val)
    {
        del_R_param = val;
    }
    void set_feedback_L_param(float val)
    {
        feedback_L_param = val;
    }
    void set_feedback_R_param(float val)
    {
        feedback_R_param = val;
    }

    void set_drywet_param(float val)
    {
        gDryWet_param = val;
    }
    void set_gVolume_param(float val) { gVolume_param = val; }

    
    // Parameters updated in PluginEditor -> need to be public.
    float del_L_param_prev;
    float del_R_param_prev;
    float feedback_L_param_prev;
    float feedback_R_param_prev;
    float gDryWet_param_prev;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PingPongDelayAudioProcessor)
    
    int BUFFER_SIZE = 262144;
    
    std::vector<std::vector<float>> gDelayBuffer_inSig; // delay buffer from input
    std::vector<int> gWritePointer_inSig; // write pointer for delay buffer from input
    std::vector<int> gReadPointer_inSig; // read pointer for delay buffer from input
    
    std::vector<std::vector<float>> gDelayBuffer_crossSig; // delay buffer loaded from head shadow model
    std::vector<int> gWritePointer_crossSig; // write pointer for delay buffer loaded from head shadow model
    std::vector<int> gReadPointer_crossSig; // read pointer for delay buffer loaded from head shadow model
    
    int gInitLatency;
    
    float crossSig_R = 0;
    float crossSig_L = 0;
    float crossSig_R_del_R = 0;
    float crossSig_R_del_L = 0;
    float crossSig_L_del_L = 0;
    float crossSig_L_del_R = 0;
    float inSig_L_del_L = 0;
    float inSig_R_del_R = 0;
    
//    float feedback_L = 0;
//    float feedback_R = 0;
    
    float a1 = 1;
    float a2 = 1;
    float c1 = 1;
    float c2 = 1;

    float gSampleRate, T;
    
//    float del_L;
//    float del_R;
    
    
    // AUDIO PARAMS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        params.push_back(std::make_unique<AudioParameterFloat>("DEL_L","Del_L",0.0f,2000.0f,0.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("DEL_R","Del_R",0.0f,2000.0f,0.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("FEEDBACK_L","Feedback_L",0.0f,1.0f,0.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("FEEDBACK_R","Feedback_R",0.0f,1.0f,0.0f));
        params.push_back(std::make_unique<AudioParameterFloat>("DRY_WET","Dry_Wet",0.0f,1.0f,1.0f)); // in dB
        params.push_back(std::make_unique<AudioParameterFloat>("VOLUME","Volume",-20.0f,20.0f,0.0f)); // in dB

        return { params.begin(), params.end()};
    }
    
    float del_L_param;
    float del_R_param;
    float feedback_L_param;
    float feedback_R_param;
    float gVolume_param;
    float gDryWet_param;
    
    float del_L;
    float del_R;
    float feedback_L;
    float feedback_R;
    float gVolume;
    float gDryWet;
    
    std::vector<float> outVal, outValDry;

    float gFactDry, gFactWet;
    float drywet;
    
//    gDelayBuffer_inSig = zeros(2,BUFFER_SIZE); % no channels x BUFFER_SIZE
//    gWritePointer_inSig = ones(2,1);
//    gReadPointer_inSig = ones(2,1);
//
//    gDelayBuffer_crossSig = zeros(2,BUFFER_SIZE); % no channels x BUFFER_SIZE
//    gWritePointer_crossSig = ones(2,1);
//    gReadPointer_crossSig = ones(2,1);
    
};
