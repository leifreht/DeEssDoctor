#include <JuceHeader.h>
#include "MainComponent.h"

class DeEssDoctorApplication : public juce::JUCEApplication
{
public:
    DeEssDoctorApplication() = default;
    
    const juce::String getApplicationName() override       { return "DeEssDoctorApplication"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        mainWindow.reset (new MainWindow ("DeEssDoctorApplication", new MainComponent(), *this));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (const juce::String& name, juce::Component* c, juce::JUCEApplication& app)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                      .findColour (juce::ResizableWindow::backgroundColourId),
                              juce::DocumentWindow::allButtons),
              App (app)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            App.systemRequestedQuit();
        }

    private:
        juce::JUCEApplication& App;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (DeEssDoctorApplication)
