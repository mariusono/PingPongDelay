/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//------------------------------------------------------------
double linearMapping(float rangeIn_top, float rangeIn_bottom, float rangeOut_top, float rangeOut_bottom, float value) {
    double newValue = rangeOut_bottom + ((rangeOut_top - rangeOut_bottom) * (value - rangeIn_bottom) / (rangeIn_top - rangeIn_bottom));
    return newValue;
}
//------------------------------------------------------------

//==============================================================================
PingPongDelayAudioProcessor::PingPongDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
}

PingPongDelayAudioProcessor::~PingPongDelayAudioProcessor()
{
}

//==============================================================================
const juce::String PingPongDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PingPongDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PingPongDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PingPongDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PingPongDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PingPongDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PingPongDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PingPongDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PingPongDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void PingPongDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PingPongDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Print sample rate -- for checking purposes
    gSampleRate = sampleRate;
    T = 1/gSampleRate;
    Logger::getCurrentLogger()->outputDebugString("Sample rate is " + String(sampleRate) + ".");
    
    // Read parameters from sliders
    auto* selection_del_L = dynamic_cast<AudioParameterFloat*> (apvts.getParameter ("DEL_L"));
    del_L_param = selection_del_L->get();
    del_L_param = 0.0; // overwriting.. just in case
    del_L_param_prev = 0.0; // overwriting.. just in case

    auto* selection_del_R = dynamic_cast<AudioParameterFloat*> (apvts.getParameter ("DEL_R"));
    del_R_param = selection_del_R->get();
    del_R_param = 0.0; // overwriting.. just in case
    del_R_param_prev = 0.0; // overwriting.. just in case

    auto* selection_feedback_L = dynamic_cast<AudioParameterFloat*> (apvts.getParameter ("FEEDBACK_L"));
    feedback_L_param = selection_feedback_L->get();
    feedback_L_param = 0.0; // overwriting.. just in case
    feedback_L_param_prev = 0.0; // overwriting.. just in case

    auto* selection_feedback_R = dynamic_cast<AudioParameterFloat*> (apvts.getParameter ("FEEDBACK_R"));
    feedback_R_param = selection_feedback_R->get();
    feedback_R_param = 0.0; // overwriting.. just in case
    feedback_R_param_prev = 0.0; // overwriting.. just in case

    auto* selection_drywet = dynamic_cast<AudioParameterFloat*> (apvts.getParameter ("DRY_WET"));
    gDryWet_param = selection_drywet->get();
    gDryWet_param = 0.0; // overwriting.. just in case
    gDryWet_param_prev = 0.0; // overwriting.. just in case

    gVolume_param = 0.0;
    
    // Resizing buffers and preallocating read and write pointers
    gDelayBuffer_inSig.resize(2); // 2 channels
    for (int i = 0; i < 2; ++i)
        gDelayBuffer_inSig[i].resize(BUFFER_SIZE,0);
    
    gDelayBuffer_crossSig.resize(2); // 2 channels
    for (int i = 0; i < 2; ++i)
        gDelayBuffer_crossSig[i].resize(BUFFER_SIZE,0);
 
    gInitLatency = 8;
    
    gWritePointer_inSig.resize(2,gInitLatency);
    gReadPointer_inSig.resize(2,0);
    
    gWritePointer_crossSig.resize(2,gInitLatency);
    gReadPointer_crossSig.resize(2,0);
    
    outVal.resize(2,0);
    outValDry.resize(2,0);

}

void PingPongDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PingPongDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PingPongDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float* const outputL = buffer.getWritePointer(0);
    float* const outputR = buffer.getWritePointer(1);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


//    Logger::getCurrentLogger()->outputDebugString("gFactDry is " + String(gFactDry) + ".");
//    Logger::getCurrentLogger()->outputDebugString("gFactWet is " + String(gFactWet) + ".");
//    Logger::getCurrentLogger()->outputDebugString("gDryWet_param is " + String(gDryWet_param) + ".");
//    Logger::getCurrentLogger()->outputDebugString("drywet is " + String(drywet) + ".");

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    

    
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {

        // READ PARAMS
        del_L = (1-0.99)*del_L_param + 0.99*del_L_param_prev;
        del_L_param_prev = del_L;
        
//        Logger::getCurrentLogger()->outputDebugString("del_L is " + String(del_L) + ".");

        
        del_R = (1-0.99)*del_R_param + 0.99*del_R_param_prev;
        del_R_param_prev = del_R;
        
        for (int channel = 0; channel < 2; ++channel) // doing this because im overwriting the first channel after going through the first iteration of the loop !
        {
            auto* input = buffer.getWritePointer (channel);

            // Current input sample
            float in = input[i];
            
            outValDry[channel] = in; // Dry output
            
            

            feedback_L = (1-0.8)*feedback_L_param + 0.8*feedback_L_param_prev;
            feedback_L_param_prev = feedback_L_param;
            
            feedback_R = (1-0.8)*feedback_R_param + 0.8*feedback_R_param_prev;
            feedback_R_param_prev = feedback_R_param;
            
            gDryWet = (1-0.8)*gDryWet_param + 0.8*gDryWet_param_prev;
            gDryWet_param_prev = gDryWet_param;
            
            // delays in samples
            float del_L_samples = floor(del_L*gSampleRate/1000);
            float del_L_frac_part = del_L*gSampleRate/1000 - del_L_samples;
            float del_R_samples = floor(del_R*gSampleRate/1000);
            float del_R_frac_part = del_R*gSampleRate/1000 - del_R_samples;
            
            if (channel == 0)
            {
                gDelayBuffer_inSig[channel][gWritePointer_inSig[channel]] = in;
                
//                int index = (int)floorf(gReadPointer_inSig[channel]);
//                float alpha = gReadPointer_inSig[channel] - index;
                
                int index = (gReadPointer_inSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                int index_m1 = (gReadPointer_inSig[channel] - static_cast<int>(del_L_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p1 = (gReadPointer_inSig[channel] - static_cast<int>(del_L_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p2 = (gReadPointer_inSig[channel] - static_cast<int>(del_L_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                
                float alpha = del_L_frac_part;
                
                inSig_L_del_L = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_inSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_inSig[channel][index_p2]/(6) );
                
//                int outPointer_L = (gReadPointer_inSig[channel] - 1 - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_L_frac = (gReadPointer_inSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                
//                inSig_L_del_L = del_L_frac_part*gDelayBuffer_inSig[channel][outPointer_L] + (1-del_L_frac_part)*gDelayBuffer_inSig[channel][outPointer_L_frac];

                crossSig_L = a1*inSig_L_del_L + feedback_L*crossSig_R_del_L;
                
                gDelayBuffer_crossSig[channel][gWritePointer_crossSig[channel]] = crossSig_L;

//                int outPointer_L_cross = (gReadPointer_crossSig[channel] - 1 - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_L_cross_frac = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                
                index = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_m1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p2 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha = del_L_frac_part;

                crossSig_L_del_L = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_crossSig[channel][index_p2]/(6) );
                
                
                index = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_m1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p2 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha = del_R_frac_part;
                
                crossSig_L_del_R = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_crossSig[channel][index_p2]/(6) );
                
//                int outPointer_R_cross = (gReadPointer_crossSig[channel] - 1 - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_R_cross_frac = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                
//                crossSig_L_del_R = del_R_frac_part*gDelayBuffer_crossSig[channel][outPointer_R_cross] + (1-del_R_frac_part)*gDelayBuffer_crossSig[channel][outPointer_R_cross_frac];
//                crossSig_L_del_L = del_L_frac_part*gDelayBuffer_crossSig[channel][outPointer_L_cross] + (1-del_L_frac_part)*gDelayBuffer_crossSig[channel][outPointer_L_cross_frac];
                
                outVal[channel] = in + c1*crossSig_L;
            }
            else if (channel == 1)
            {
                gDelayBuffer_inSig[channel][gWritePointer_inSig[channel]] = in;
                
//                int outPointer_R = (gReadPointer_inSig[channel] - 1 - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_R_frac = (gReadPointer_inSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                
                int index = (gReadPointer_inSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                int index_m1 = (gReadPointer_inSig[channel] - static_cast<int>(del_R_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p1 = (gReadPointer_inSig[channel] - static_cast<int>(del_R_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                int index_p2 = (gReadPointer_inSig[channel] - static_cast<int>(del_R_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                float alpha = del_R_frac_part;
                
                inSig_R_del_R = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_inSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_inSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_inSig[channel][index_p2]/(6) );
                
//                inSig_R_del_R = del_R_frac_part*gDelayBuffer_inSig[channel][outPointer_R] + (1-del_R_frac_part)*gDelayBuffer_inSig[channel][outPointer_R_frac];

                crossSig_R = a2*inSig_R_del_R + feedback_R*crossSig_L_del_R;
                
                gDelayBuffer_crossSig[channel][gWritePointer_crossSig[channel]] = crossSig_R;

                index = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_m1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p2 = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha = del_R_frac_part;
                
                
                crossSig_R_del_R = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_crossSig[channel][index_p2]/(6) );
                
                
                index = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
                index_m1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) - 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p1 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + 1 + BUFFER_SIZE) % BUFFER_SIZE;
                index_p2 = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + 2 + BUFFER_SIZE) % BUFFER_SIZE;
                alpha = del_L_frac_part;
                
                
                crossSig_R_del_L = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_m1]/(-6)
                                         + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index]/2
                                         + alpha*(alpha+1)*(alpha-2)*gDelayBuffer_crossSig[channel][index_p1]/(-2)
                                         + alpha*(alpha+1)*(alpha-1)*gDelayBuffer_crossSig[channel][index_p2]/(6) );
                
                
//                int outPointer_L_cross = (gReadPointer_crossSig[channel] - 1 - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_L_cross_frac = (gReadPointer_crossSig[channel] - static_cast<int>(del_L_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_R_cross = (gReadPointer_crossSig[channel] - 1 - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//                int outPointer_R_cross_frac = (gReadPointer_crossSig[channel] - static_cast<int>(del_R_samples) + BUFFER_SIZE) % BUFFER_SIZE;
//
//                crossSig_R_del_R = del_R_frac_part*gDelayBuffer_crossSig[channel][outPointer_R_cross] + (1-del_R_frac_part)*gDelayBuffer_crossSig[channel][outPointer_R_cross_frac];
//                crossSig_R_del_L = del_L_frac_part*gDelayBuffer_crossSig[channel][outPointer_L_cross] + (1-del_L_frac_part)*gDelayBuffer_crossSig[channel][outPointer_L_cross_frac];
                
                outVal[channel] = in + c2*crossSig_R;
            }
            
            // update gWritePointer
            gWritePointer_inSig[channel] = gWritePointer_inSig[channel] + 1;
            if (gWritePointer_inSig[channel] >= BUFFER_SIZE)
                gWritePointer_inSig[channel] = 0;

            // update gReadPointer
            gReadPointer_inSig[channel] = gReadPointer_inSig[channel] + 1;
            if (gReadPointer_inSig[channel] >= BUFFER_SIZE)
                gReadPointer_inSig[channel] = 0;
            
            // update gWritePointer_head_shadow
            gWritePointer_crossSig[channel] = gWritePointer_crossSig[channel] + 1;
            if (gWritePointer_crossSig[channel] >= BUFFER_SIZE)
                gWritePointer_crossSig[channel] = 0;

            // update gReadPointer_head_shadow
            gReadPointer_crossSig[channel] = gReadPointer_crossSig[channel] + 1;
            if (gReadPointer_crossSig[channel] >= BUFFER_SIZE)
                gReadPointer_crossSig[channel] = 0;
            
            
//
//            if (abs((outVal_post_pinnae[channel])) > 1)
//            {
//                Logger::getCurrentLogger()->outputDebugString("Output is too loud!");
//            }
            
            
            drywet = linearMapping(1.0f, 0.0f, 1.0f, -1.0f, gDryWet);
            
            if(drywet<-1.0)
            {
                drywet = -1.0;
            }else if (drywet>0.99)
            {
                drywet = 1.0;
            }
            
            gFactDry = (powf(0.5*(1.0-drywet),0.5));
            gFactWet = (powf(0.5*(1.0+drywet),0.5));
            
//            Logger::getCurrentLogger()->outputDebugString("gFactDry is " + String(gFactDry) + ".");
//            Logger::getCurrentLogger()->outputDebugString("gFactWet is " + String(gFactWet) + ".");

            
            if (channel == 0)
            {
//                output = (output * gFactWet + output_dry * gFactDry);

                outputL[i] = (outVal[channel] * gFactWet + outValDry[channel] * gFactDry) * powf(10,(gVolume_param/20));
//                outputL[i] = (outVal_post_pinnae[channel]) * 1.0;
//                outputL[i] = (outVal_head_shadow[channel]) * 1.0;
                if (abs(outputL[i]) > 1)
                {
                    Logger::getCurrentLogger()->outputDebugString("Output left is too loud!");
                }
            }
            else if (channel == 1)
            {
                outputR[i] = (outVal[channel] * gFactWet + outValDry[channel] * gFactDry) * powf(10,(gVolume_param/20));
//                outputR[i] = (outVal_post_pinnae[channel]) * 1.0;
//                outputR[i] = (outVal_head_shadow[channel]) * 1.0;
                if (abs(outputR[i]) > 1)
                {
                    Logger::getCurrentLogger()->outputDebugString("Output right is too loud!");
                }
            }
        }
        // ..do something to the data...
    }
}

//==============================================================================
bool PingPongDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PingPongDelayAudioProcessor::createEditor()
{
    return new PingPongDelayAudioProcessorEditor (*this);
}

//==============================================================================
void PingPongDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PingPongDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PingPongDelayAudioProcessor();
}
