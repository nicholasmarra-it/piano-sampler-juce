
#include <JuceHeader.h>

class WaveformDisplay : public juce::Component,
                        private juce::ChangeListener,
                        public juce::FileDragAndDropTarget
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    std::function<void(const juce::File&)> onFileDropped = nullptr;

    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        for (auto& f : files)
            if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") || f.endsWithIgnoreCase(".aiff"))
                return true;
        return false;
    }

    void filesDropped(const juce::StringArray& files, int, int) override
    {
        if (files.isEmpty()) return;
        juce::File f(files[0]);
        if (onFileDropped) onFileDropped(f);
    }

    void paint(juce::Graphics&) override;
    void resized() override;

    void setAudioThumbnail(juce::AudioThumbnail* newThumbnail);
    void setPlaybackPosition(double position);
    void setLoopRange(double start, double end);
    void clearThumbnail(); 

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    juce::AudioThumbnail* thumbnail = nullptr;
    double playbackPosition = 0.0;
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool showLoopRange = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};