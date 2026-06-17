#pragma once

#include <JuceHeader.h>

// URL do arquivo JSON de versão (Google Drive)
#define UPDATE_CHECK_URL "https://raw.githubusercontent.com/djexder/Stems-TriDJS/main/version.json"

// Versão atual do software
#define APP_VERSION "1.1.4"

class UpdateChecker : public juce::Thread
{
public:
    struct VersionInfo
    {
        juce::String latestVersion;
        juce::String downloadUrl;
        juce::String updateDate;
        bool mandatory = false;
        juce::StringArray changelog;
    };

    enum class Status
    {
        Checking,
        UpToDate,
        UpdateAvailable,
        Error
    };

    UpdateChecker();
    ~UpdateChecker() override;

    void startCheck();
    Status getStatus() const { return status; }
    const VersionInfo& getVersionInfo() const { return versionInfo; }
    juce::String getCurrentVersion() const { return APP_VERSION; }

    std::function<void()> onCheckComplete;

private:
    void run() override;
    bool isNewerVersion(const juce::String& remote, const juce::String& local);

    Status status = Status::Checking;
    VersionInfo versionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateChecker)
};
