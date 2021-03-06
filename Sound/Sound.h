#pragma once
#include <Xaudio2.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>

#include "../Common.h"

using std::cout;
using std::endl;

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
    DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL)) {
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
    return hr;
}

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition)
{
    HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK) {
        DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

        switch (dwChunkType) {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL)) {
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
            break;

        default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT)) {
				return HRESULT_FROM_WIN32(GetLastError());
			}
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc) {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) {
			return S_FALSE;
		}

    }

    return S_OK;

}


HRESULT initSound() {
    IXAudio2* xAudio2 = nullptr;
    IXAudio2MasteringVoice *masterVoice = nullptr;
    HRESULT hr;
	if (FAILED(hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
		return hr;
	}
	if (FAILED(hr = xAudio2->CreateMasteringVoice(&masterVoice))) {
		return hr;
	}

    // get riff chunks from .wav
    WAVEFORMATEXTENSIBLE wfx{ 0 };
    XAUDIO2_BUFFER buffer = { 0 };


    TCHAR * strFileName = _TEXT("test.wav");
    // Open the file
    HANDLE hFile = CreateFile(
        strFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);


    //2
	if (INVALID_HANDLE_VALUE == hFile) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}
    //3
    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    //check the file type, should be fourccWAVE or 'XWMA'
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE) {
		return S_FALSE;
	}

    //4
    FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

    //5 
    //fill out the audio data buffer with the contents of the fourccDATA chunk
    FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    BYTE * pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    //6
    buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
    buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
    buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer



    // play

    IXAudio2SourceVoice *sourceVoice;
	if (FAILED(hr = xAudio2->CreateSourceVoice(&sourceVoice, (WAVEFORMATEX*)&wfx))) { 
		return hr; 
	}
	if (FAILED(hr = sourceVoice->SubmitSourceBuffer(&buffer))) {
		return hr;
	}
    // start
	if (FAILED(hr = sourceVoice->Start(0))) {
		return hr;
	}

	return hr;
}