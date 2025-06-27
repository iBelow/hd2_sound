#pragma once
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <string>

// Mock implementation of IAudioEndpointVolume for testing
class MockAudioEndpointVolume : public IAudioEndpointVolume {
private:
    ULONG refCount = 1;
    float currentVolume = 0.5f;  // 50% by default
    bool isMuted = false;
    
public:
    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == __uuidof(IAudioEndpointVolume) || riid == __uuidof(IUnknown)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&refCount);
    }
    
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG newRef = InterlockedDecrement(&refCount);
        if (newRef == 0) {
            delete this;
        }
        return newRef;
    }
    
    // IAudioEndpointVolume methods we care about
    HRESULT STDMETHODCALLTYPE GetMasterVolumeLevelScalar(float* pfLevel) override {
        if (!pfLevel) return E_POINTER;
        *pfLevel = currentVolume;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE SetMasterVolumeLevelScalar(float fLevel, LPCGUID pguidEventContext) override {
        if (fLevel < 0.0f || fLevel > 1.0f) return E_INVALIDARG;
        currentVolume = fLevel;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE GetMute(BOOL* pbMute) override {
        if (!pbMute) return E_POINTER;
        *pbMute = isMuted ? TRUE : FALSE;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE SetMute(BOOL bMute, LPCGUID pguidEventContext) override {
        isMuted = (bMute != FALSE);
        return S_OK;
    }
    
    // Unused methods (stub implementations)
    HRESULT STDMETHODCALLTYPE RegisterControlChangeNotify(IAudioEndpointVolumeCallback* pNotify) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE UnregisterControlChangeNotify(IAudioEndpointVolumeCallback* pNotify) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetChannelCount(UINT* pnChannelCount) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetMasterVolumeLevel(float fLevelDB, LPCGUID pguidEventContext) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetMasterVolumeLevel(float* pfLevelDB) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetChannelVolumeLevel(UINT nChannel, float fLevelDB, LPCGUID pguidEventContext) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetChannelVolumeLevel(UINT nChannel, float* pfLevelDB) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetChannelVolumeLevelScalar(UINT nChannel, float fLevel, LPCGUID pguidEventContext) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetChannelVolumeLevelScalar(UINT nChannel, float* pfLevel) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetVolumeRange(float* pflVolumeMindB, float* pflVolumeMaxdB, float* pflVolumeIncrementdB) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE VolumeStepUp(LPCGUID pguidEventContext) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE VolumeStepDown(LPCGUID pguidEventContext) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD* pdwHardwareSupportMask) override { return E_NOTIMPL; }
    
    // Test helper methods
    void SetTestVolume(float volume) { currentVolume = volume; }
    float GetTestVolume() const { return currentVolume; }
};

// Mock device for testing
class MockAudioDevice {
public:
    std::wstring deviceName;
    MockAudioEndpointVolume* mockVolume;
    
    MockAudioDevice(const std::wstring& name) : deviceName(name) {
        mockVolume = new MockAudioEndpointVolume();
    }
    
    ~MockAudioDevice() {
        if (mockVolume) {
            mockVolume->Release();
        }
    }
};
