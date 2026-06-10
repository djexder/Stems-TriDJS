#pragma once

#include <JuceHeader.h>
#include "StemEngine.h"
#include "UpdateChecker.h"
#include <functional>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour(juce::TextButton::buttonColourId, juce::Colour::fromString("#FF2A2A2A"));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsPressed) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto txt = button.getButtonText();

        // 1. Clear Button (Text only)
        if (txt == juce::CharPointer_UTF8("Voltar ao Início"))
        {
            if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsPressed)
            {
                g.setColour (juce::Colour::fromString("#FF2A2A2A"));
                g.fillRoundedRectangle(bounds, 8.0f);
            }
            return;
        }

        // 1.5. Cancel Button
        if (txt == "CANCEL OPERATION")
        {
            g.setColour (juce::Colour::fromString("#FF2A2A2A"));
            g.fillRoundedRectangle (bounds, bounds.getHeight() / 2.0f);
            
            g.setColour (juce::Colour::fromString(shouldDrawButtonAsHighlighted ? "#FFE5E2E1" : "#FF414754"));
            g.drawRoundedRectangle (bounds, bounds.getHeight() / 2.0f, 1.0f);
            return;
        }

        // 2. Primary Button ("Salvar todas")
        if (txt == "Salvar todas")
        {
            auto colPrimaryBg = juce::Colour::fromString("#FF01F5A0"); // secondary-container green
            if (shouldDrawButtonAsPressed)
                colPrimaryBg = colPrimaryBg.darker(0.2f);
            else if (shouldDrawButtonAsHighlighted)
                colPrimaryBg = colPrimaryBg.brighter(0.1f);

            g.setColour (colPrimaryBg);
            g.fillRoundedRectangle (bounds, 12.0f);
            
            // Add subtle shadow/glow for primary
            g.setColour (colPrimaryBg.withAlpha(0.3f));
            g.drawRoundedRectangle(bounds, 12.0f, 2.0f);
            return;
        }

        // 3. Circular Icon Buttons (Play, Stop, Download)
        bool isCircular = (std::abs(bounds.getWidth() - bounds.getHeight()) < 5);
        if (isCircular)
        {
            auto accentCol = button.findColour(juce::TextButton::textColourOffId);
            
            // Background
            g.setColour (juce::Colour::fromString("#FF201F1F")); // surface-container-high
            g.fillEllipse (bounds.reduced (1.0f));

            // Draw circular border
            g.setColour (accentCol.withAlpha(shouldDrawButtonAsPressed ? 0.9f : (shouldDrawButtonAsHighlighted ? 0.6f : 0.25f)));
            g.drawEllipse (bounds.reduced (1.0f), 1.5f);

            // Glow effect on highlight
            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour (accentCol.withAlpha(0.15f));
                g.drawEllipse (bounds.reduced (2.5f), 3.0f);
            }
            return;
        }

        // 4. Standard rounded pill button ("Select File", "CANCEL OPERATION")
        g.setColour (backgroundColour);
        g.fillRoundedRectangle (bounds, 8.0f);

        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsPressed)
        {
            auto highlightCol = button.findColour(juce::TextButton::textColourOffId);
            g.setColour (highlightCol.withAlpha(shouldDrawButtonAsPressed ? 0.8f : 0.4f));
            g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 1.5f);
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsPressed) override
    {
        auto txt = button.getButtonText();
        
        if (txt == "CANCEL OPERATION")
        {
            g.setFont (juce::Font ("Consolas", 11.0f, juce::Font::bold));
            g.setColour (shouldDrawButtonAsHighlighted ? juce::Colours::white : juce::Colour::fromString("#FFE5E2E1"));
            g.drawText (txt, button.getLocalBounds(), juce::Justification::centred);
            return;
        }

        if (txt == "Salvar todas")
        {
            g.setFont (juce::Font ("Segoe UI", 15.0f, juce::Font::bold));
            g.setColour (juce::Colour::fromString("#FF002111")); // Dark green contrast text
            g.drawText (txt, button.getLocalBounds(), juce::Justification::centred);
            return;
        }

        if (txt == juce::CharPointer_UTF8("Voltar ao Início"))
        {
            g.setFont (juce::Font ("Segoe UI", 12.0f, juce::Font::bold));
            g.setColour (shouldDrawButtonAsHighlighted ? juce::Colour::fromString("#FFFFB4AB") : juce::Colour::fromString("#FFC1C6D7"));
            g.drawText (txt.toUpperCase(), button.getLocalBounds(), juce::Justification::centred);
            return;
        }

        // Circular Icon formatting (draw vector paths for reliability)
        bool isCircular = (std::abs(button.getWidth() - button.getHeight()) < 5);
        if (isCircular)
        {
            auto accentCol = button.findColour(juce::TextButton::textColourOffId);
            g.setColour(shouldDrawButtonAsPressed ? accentCol.brighter(0.2f) : accentCol);

            auto b = button.getLocalBounds().toFloat().reduced(11);
            juce::Path p;

            if (txt == juce::String::fromUTF8("\xe2\x96\xb6"))
            {
                p.addTriangle(b.getX(), b.getY(), b.getX(), b.getBottom(), b.getRight(), b.getCentreY());
                g.fillPath(p);
            }
            else if (txt == juce::String::fromUTF8("\xe2\x8f\xb8"))
            {
                float barW = b.getWidth() / 3.5f;
                g.fillRoundedRectangle(b.getX(), b.getY(), barW, b.getHeight(), 1.5f);
                g.fillRoundedRectangle(b.getRight() - barW, b.getY(), barW, b.getHeight(), 1.5f);
            }
            else if (txt == juce::String::fromUTF8("\xe2\x96\xa0"))
            {
                g.fillRoundedRectangle(b.reduced(2), 2.0f);
            }
            else if (txt == juce::String::fromUTF8("\xe2\x86\x93"))
            {
                auto cx = b.getCentreX();
                auto cy = b.getCentreY();
                float head = b.getWidth() * 0.35f;
                g.drawLine(cx, b.getY(), cx, cy + head * 0.3f, 2.5f);
                p.addTriangle(cx - head, cy, cx + head, cy, cx, cy + head);
                g.fillPath(p);
            }

            return;
        }

        g.setFont (juce::Font ("Segoe UI", 13.0f, juce::Font::bold));
        g.setColour (button.findColour (juce::TextButton::textColourOffId));
        g.drawText (txt, button.getLocalBounds(), juce::Justification::centred);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        // Transparent slider track because we draw the gorgeous simulated waveform underneath!
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        
        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        g.drawLine (sliderPos, bounds.getY(), sliderPos, bounds.getBottom(), 2.0f);
        
        // Small glowing handle triangle at the top of the indicator
        juce::Path p;
        p.addTriangle (sliderPos - 4.0f, bounds.getY(),
                       sliderPos + 4.0f, bounds.getY(),
                       sliderPos, bounds.getY() + 6.0f);
        g.fillPath (p);
    }

    void drawMenuBarBackground (juce::Graphics& g, int width, int height,
                                bool, juce::MenuBarComponent&) override
    {
        g.setColour (juce::Colour::fromString("#FF1C1B1B"));
        g.fillRect (0, 0, width, height);
        g.setColour (juce::Colour::fromString("#FF353534"));
        g.drawLine (0.0f, (float)(height - 1), (float)width, (float)(height - 1), 1.0f);
    }

    void drawMenuBarItem (juce::Graphics& g, int width, int height,
                          int, const juce::String& itemText,
                          bool isMouseOverItem, bool isMenuOpen,
                          bool, juce::MenuBarComponent&) override
    {
        if (isMouseOverItem || isMenuOpen)
        {
            g.setColour (juce::Colour::fromString("#FF353534"));
            g.fillRect (0, 0, width, height);
        }

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font ("Segoe UI", 13.0f, juce::Font::plain));
        g.drawText (itemText, 6, 0, width - 6, height, juce::Justification::centredLeft);
    }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        g.setColour (juce::Colour::fromString("#FF252525"));
        g.fillRect (0, 0, width, height);
        g.setColour (juce::Colour::fromString("#FF353534"));
        g.drawRect (0, 0, width, height, 1);
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        if (isHighlighted && isActive)
        {
            g.setColour (juce::Colour::fromString("#FF353534"));
            g.fillRect (area);
        }

        if (!isSeparator)
        {
            g.setColour (isActive ? juce::Colours::white : juce::Colour::fromString("#FF8B90A0"));
            g.setFont (juce::Font ("Segoe UI", 13.0f, juce::Font::plain));
            g.drawText (text, area.reduced (4, 0), juce::Justification::centredLeft);
        }
        else
        {
            g.setColour (juce::Colour::fromString("#FF353534"));
            g.fillRect (area.reduced (4, 0).withHeight (1));
        }
    }
};

class StemEngineWorker : public juce::Thread
{
public:
    enum class Job { Warmup, ProcessTrack };

    StemEngineWorker (Job jobToRun, std::function<void()> onCompletionCallback)
        : Thread ("StemEngineWorker"), job (jobToRun), onCompletion (onCompletionCallback)
    {
    }

    void setProcessingParams (const std::string& input, const std::string& output, std::function<void(float)> progress)
    {
        inputPath = input;
        outputDir = output;
        progressCallback = progress;
    }

    void run() override
    {
        if (job == Job::Warmup)
        {
            // Direct C++ LibTorch Warmup
            StemEngine::getInstance().initialize();
            
            juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
            juce::File modelFile = exeFile.getSiblingFile("resources").getChildFile("htdemucs_compilado.pt");
            if (!modelFile.existsAsFile())
                modelFile = juce::File::getCurrentWorkingDirectory().getChildFile("resources").getChildFile("htdemucs_compilado.pt");
            if (!modelFile.existsAsFile())
                modelFile = exeFile.getSiblingFile("htdemucs_compilado.pt");

            StemEngine::getInstance().loadModel(modelFile.getFullPathName().toStdString());
            StemEngine::getInstance().warmupCUDA();
        }
        else if (job == Job::ProcessTrack)
        {
            success = StemEngine::getInstance().processTrack(
                inputPath, outputDir, progressCallback, [this]() { return threadShouldExit(); });
        }

        if (onCompletion)
            juce::MessageManager::callAsync (onCompletion);
    }

    bool wasSuccessful() const { return success; }

private:
    Job job;
    std::function<void()> onCompletion;
    std::string inputPath;
    std::string outputDir;
    std::function<void(float)> progressCallback;
    bool success = false;
};

class MainComponent  : public juce::Component,
                       public juce::FileDragAndDropTarget,
                       public juce::Timer,
                       public juce::MenuBarModel
{
public:
    MainComponent();
    ~MainComponent() override;

    void startWarmup();

    void paint (juce::Graphics&) override;
    void resized() override;

    // MenuBarModel
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;

    // Drag and Drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    // Timer for UI updates and animations
    void timerCallback() override;

    std::function<void(bool success)> onInitializationComplete;

private:
    enum class AppState
    {
        ScreenSplash,
        ScreenUpload,
        ScreenProcessing,
        ScreenResults
    };

    AppState currentState = AppState::ScreenSplash;
    juce::Image splashImage;
    int splashTimeoutCounter = 0;

    // =========================================================================
    // Core Logic
    juce::File tempDir;
    juce::File inputFile;
    std::unique_ptr<StemEngineWorker> engineWorker;

    void setupTempDirectory();
    void cleanTempDirectory();
    void startProcessing (const juce::File& fileToProcess);
    void finalizeProcessing();

    // =========================================================================
    // UI Elements
    
    // Screen 1: Upload
    juce::TextButton selectFileBtn { "+ Selecionar Arquivo" };
    void openFileChooser();

    // Screen 2: Processing
    double progressValue = 0.0;
    double targetProgressValue = 0.0;
    juce::ProgressBar progressBar { progressValue };
    juce::Label progressLabel;
    juce::String processStatusText;
    juce::TextButton cancelBtn { "CANCEL OPERATION" };

    // Screen 3: Results
    struct StemPlayer
    {
        juce::String name;
        juce::Colour colour;
        juce::File audioFile;
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        juce::AudioTransportSource transportSource;
        
        juce::TextButton playBtn;
        juce::TextButton stopBtn;
        juce::Slider seekBar;
        juce::TextButton downloadBtn;
    };

    std::vector<std::unique_ptr<StemPlayer>> stemPlayers;
    
    juce::TextButton downloadAllBtn { "Salvar todas" };
    juce::TextButton clearBtn { juce::CharPointer_UTF8("Voltar ao Início") };

    // Track analysis result
    double trackBpm = 0.0;
    juce::String trackKey;

    // Export status feedback
    juce::String exportStatus;
    int exportStatusCounter = 0;
    bool exportWriteStarted = false;
    int exportStartCheckCounter = 0;

    // Audio Engine
    juce::AudioFormatManager formatManager;
    juce::MixerAudioSource mixerSource;
    juce::AudioSourcePlayer sourcePlayer;
    juce::AudioDeviceManager deviceManager;
    
    void loadAudioFiles();
    void setupStemPlayer(StemPlayer& player, const juce::String& name, juce::Colour colour, const juce::File& file);
    void togglePlayPause(StemPlayer& player);
    void toggleAllPlayPause();
    
    // Export Functions
    void exportSingleStem(const juce::File& fileToExport, const juce::String& defaultName);
    void exportAllAsZip();

    // Theming helpers
    void drawUploadScreen(juce::Graphics& g);
    void drawProcessingScreen(juce::Graphics& g);
    void drawResultsScreen(juce::Graphics& g);

    void mouseMove(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    CustomLookAndFeel customLookAndFeel;
    std::unique_ptr<UpdateChecker> updateChecker;
    juce::MenuBarComponent menuBar;
    int menuBarHeight = 0;

private:
    juce::Rectangle<int> cardBanner1, cardBanner2;
    juce::Rectangle<int> card3Bounds;
    juce::Rectangle<int> downloadBtnBounds;
    int hoveredCard = 0; // 0 = none, 1 = banner1, 2 = banner2
    bool hoveredDownloadBtn = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
