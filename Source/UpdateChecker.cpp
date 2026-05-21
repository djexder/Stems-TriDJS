#include "UpdateChecker.h"

UpdateChecker::UpdateChecker()
    : Thread("UpdateChecker")
{
}

UpdateChecker::~UpdateChecker()
{
    stopThread(5000);
}

void UpdateChecker::startCheck()
{
    status = Status::Checking;
    startThread();
}

void UpdateChecker::run()
{
    juce::URL url(UPDATE_CHECK_URL);

    auto http = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(8000)
            .withResponseHeaders(nullptr));

    if (http == nullptr)
    {
        status = Status::UpToDate;
        if (onCheckComplete)
            juce::MessageManager::callAsync(onCheckComplete);
        return;
    }

    auto jsonString = http->readEntireStreamAsString();
    auto json = juce::JSON::parse(jsonString);
    auto* obj = json.getDynamicObject();

    if (obj == nullptr)
    {
        status = Status::UpToDate;
        if (onCheckComplete)
            juce::MessageManager::callAsync(onCheckComplete);
        return;
    }

    auto remoteVersion = obj->getProperty("version").toString();
    auto downloadUrl = obj->getProperty("download_url").toString();
    auto updateDate = obj->getProperty("update_date").toString();
    auto mandatory = (bool)obj->getProperty("mandatory");
    auto* changelogArray = obj->getProperty("changelog").getArray();

    if (remoteVersion.isEmpty())
    {
        status = Status::UpToDate;
        if (onCheckComplete)
            juce::MessageManager::callAsync(onCheckComplete);
        return;
    }

    if (isNewerVersion(remoteVersion, APP_VERSION))
    {
        versionInfo.latestVersion = remoteVersion;
        versionInfo.downloadUrl = downloadUrl;
        versionInfo.updateDate = updateDate;
        versionInfo.mandatory = mandatory;

        if (changelogArray != nullptr)
        {
            for (auto& item : *changelogArray)
                versionInfo.changelog.add(item.toString());
        }

        status = Status::UpdateAvailable;
    }
    else
    {
        status = Status::UpToDate;
    }

    if (onCheckComplete)
        juce::MessageManager::callAsync(onCheckComplete);
}

bool UpdateChecker::isNewerVersion(const juce::String& remote, const juce::String& local)
{
    auto remoteParts = juce::StringArray::fromTokens(remote, ".", "");
    auto localParts = juce::StringArray::fromTokens(local, ".", "");

    int maxParts = juce::jmax(remoteParts.size(), localParts.size());
    for (int i = 0; i < maxParts; ++i)
    {
        int r = (i < remoteParts.size()) ? remoteParts[i].getIntValue() : 0;
        int l = (i < localParts.size()) ? localParts[i].getIntValue() : 0;
        if (r > l) return true;
        if (r < l) return false;
    }
    return false;
}
