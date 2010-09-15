/*
 *  Copyright (C) 2006, 2007, 2008, 2009, 2010 Stephen F. Booth <me@sbooth.org>
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    - Neither the name of Stephen F. Booth nor the names of its 
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AudioToolbox/AudioFormat.h>
#include <CoreServices/CoreServices.h>
#include <stdexcept>

#include "AudioEngineDefines.h"
#include "AudioDecoder.h"
#include "CreateDisplayNameForURL.h"
#include "LoopableRegionDecoder.h"
#include "CoreAudioDecoder.h"
#include "FLACDecoder.h"
#include "WavPackDecoder.h"
#include "MPEGDecoder.h"
#include "OggVorbisDecoder.h"
#include "MusepackDecoder.h"


// ========================================
// Error Codes
// ========================================
const CFStringRef	AudioDecoderErrorDomain					= CFSTR("org.sbooth.SFBAudioEngine.ErrorDomain.AudioDecoder");


#pragma mark Static Methods


CFArrayRef AudioDecoder::CreateSupportedFileExtensions()
{
	CFMutableArrayRef supportedExtensions = CFArrayCreateMutable(kCFAllocatorDefault, 32, &kCFTypeArrayCallBacks);
	
	CFArrayRef decoderExtensions = FLACDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;

	decoderExtensions = WavPackDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;

	decoderExtensions = MPEGDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;

	decoderExtensions = OggVorbisDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;

	decoderExtensions = MusepackDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;

	decoderExtensions = CoreAudioDecoder::CreateSupportedFileExtensions();
	CFArrayAppendArray(supportedExtensions, decoderExtensions, CFRangeMake(0, CFArrayGetCount(decoderExtensions)));
	CFRelease(decoderExtensions), decoderExtensions = NULL;
	
	CFArrayRef result = CFArrayCreateCopy(kCFAllocatorDefault, supportedExtensions);
	
	CFRelease(supportedExtensions), supportedExtensions = NULL;
	
	return result;
}

CFArrayRef AudioDecoder::CreateSupportedMIMETypes()
{
	CFMutableArrayRef supportedMIMETypes = CFArrayCreateMutable(kCFAllocatorDefault, 32, &kCFTypeArrayCallBacks);
	
	CFArrayRef decoderMIMETypes = FLACDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	decoderMIMETypes = WavPackDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	decoderMIMETypes = MPEGDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	decoderMIMETypes = OggVorbisDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	decoderMIMETypes = MusepackDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	decoderMIMETypes = CoreAudioDecoder::CreateSupportedMIMETypes();
	CFArrayAppendArray(supportedMIMETypes, decoderMIMETypes, CFRangeMake(0, CFArrayGetCount(decoderMIMETypes)));
	CFRelease(decoderMIMETypes), decoderMIMETypes = NULL;
	
	CFArrayRef result = CFArrayCreateCopy(kCFAllocatorDefault, supportedMIMETypes);
	
	CFRelease(supportedMIMETypes), supportedMIMETypes = NULL;
	
	return result;
}

bool AudioDecoder::HandlesFilesWithExtension(CFStringRef extension)
{
	assert(NULL != extension);
	
	CFArrayRef supportedExtensions = CreateSupportedFileExtensions();
	if(NULL == supportedExtensions)
		return false;
	
	bool extensionIsSupported = false;
	
	CFIndex numberOfSupportedExtensions = CFArrayGetCount(supportedExtensions);
	for(CFIndex currentIndex = 0; currentIndex < numberOfSupportedExtensions; ++currentIndex) {
		CFStringRef currentExtension = static_cast<CFStringRef>(CFArrayGetValueAtIndex(supportedExtensions, currentIndex));
		if(kCFCompareEqualTo == CFStringCompare(extension, currentExtension, kCFCompareCaseInsensitive)) {
			extensionIsSupported = true;
			break;
		}
	}
	
	CFRelease(supportedExtensions), supportedExtensions = NULL;

	return extensionIsSupported;
}

bool AudioDecoder::HandlesMIMEType(CFStringRef mimeType)
{
	assert(NULL != mimeType);

	CFArrayRef supportedMIMETypes = CreateSupportedMIMETypes();
	if(NULL == supportedMIMETypes)
		return false;
	
	bool mimeTypeIsSupported = false;
	
	CFIndex numberOfSupportedMIMETypes = CFArrayGetCount(supportedMIMETypes);
	for(CFIndex currentIndex = 0; currentIndex < numberOfSupportedMIMETypes; ++currentIndex) {
		CFStringRef currentMIMEType = static_cast<CFStringRef>(CFArrayGetValueAtIndex(supportedMIMETypes, currentIndex));
		if(kCFCompareEqualTo == CFStringCompare(mimeType, currentMIMEType, kCFCompareCaseInsensitive)) {
			mimeTypeIsSupported = true;
			break;
		}
	}

	CFRelease(supportedMIMETypes), supportedMIMETypes = NULL;
	
	return mimeTypeIsSupported;
}

AudioDecoder * AudioDecoder::CreateDecoderForURL(CFURLRef url, CFErrorRef *error)
{
	assert(NULL != url);

	// Create the input source which will feed the decoder
	InputSource *inputSource = InputSource::CreateInputSourceForURL(url, 0, error);
	
	if(NULL == inputSource)
		return NULL;
	
	AudioDecoder *decoder = AudioDecoder::CreateDecoderForInputSource(inputSource, error);
	
	if(NULL == decoder)
		delete inputSource, inputSource = NULL;
	
	return decoder;
}

// If this returns NULL, the caller is responsible for deleting inputSource
// If this returns an AudioDecoder instance, the instance takes ownership of inputSource
AudioDecoder * AudioDecoder::CreateDecoderForInputSource(InputSource *inputSource, CFErrorRef *error)
{
	assert(NULL != inputSource);

	AudioDecoder *decoder = NULL;

	// Open the input source if it isn't already
	if(!inputSource->IsOpen() && !inputSource->Open(error))
		return NULL;
	
	// If this is a file URL, use the extension-based resolvers
	CFURLRef url = inputSource->GetURL();
	CFStringRef scheme = CFURLCopyScheme(url);

	if(kCFCompareEqualTo == CFStringCompare(CFSTR("file"), scheme, kCFCompareCaseInsensitive)) {
		CFStringRef pathExtension = CFURLCopyPathExtension(url);
		
		if(NULL != pathExtension) {
			// Some extensions (.oga for example) support multiple audio codecs (Vorbis, FLAC, Speex)
			
			// As a factory this class has knowledge of its subclasses
			// It would be possible (and perhaps preferable) to switch to a generic
			// plugin interface at a later date
			if(FLACDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new FLACDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			if(NULL == decoder && WavPackDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new WavPackDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			if(NULL == decoder && MPEGDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new MPEGDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			if(NULL == decoder && OggVorbisDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new OggVorbisDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			if(NULL == decoder && MusepackDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new MusepackDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			if(NULL == decoder && CoreAudioDecoder::HandlesFilesWithExtension(pathExtension)) {
				decoder = new CoreAudioDecoder(inputSource);
				if(!decoder->OpenFile(error)) {
					decoder->mInputSource = NULL;
					delete decoder, decoder = NULL;
				}
			}
			
			CFRelease(pathExtension), pathExtension = NULL;
		}
		else if(error) {
			CFMutableDictionaryRef errorDictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 
																			   32,
																			   &kCFTypeDictionaryKeyCallBacks,
																			   &kCFTypeDictionaryValueCallBacks);
			
			CFStringRef displayName = CFURLCopyLastPathComponent(url);
			CFStringRef errorString = CFStringCreateWithFormat(kCFAllocatorDefault, 
															   NULL, 
															   CFCopyLocalizedString(CFSTR("The type of the file “%@” could not be determined."), ""), 
															   displayName);
			
			CFDictionarySetValue(errorDictionary, 
								 kCFErrorLocalizedDescriptionKey, 
								 errorString);
			
			CFDictionarySetValue(errorDictionary, 
								 kCFErrorLocalizedFailureReasonKey, 
								 CFCopyLocalizedString(CFSTR("Unknown file type"), ""));
			
			CFDictionarySetValue(errorDictionary, 
								 kCFErrorLocalizedRecoverySuggestionKey, 
								 CFCopyLocalizedString(CFSTR("The file's extension may be missing or may not match the file's type."), ""));
			
			CFRelease(errorString), errorString = NULL;
			CFRelease(displayName), displayName = NULL;
			
			*error = CFErrorCreate(kCFAllocatorDefault, 
								   InputSourceErrorDomain, 
								   InputSourceFileNotFoundError,
								   errorDictionary);
			
			CFRelease(errorDictionary), errorDictionary = NULL;				
		}
	}
	// Determine the MIME type for the URL
	else {
	}
	
	CFRelease(scheme), scheme = NULL;

	return decoder;
}

AudioDecoder * AudioDecoder::CreateDecoderForURLRegion(CFURLRef url, SInt64 startingFrame, CFErrorRef *error)
{
	AudioDecoder *decoder = AudioDecoder::CreateDecoderForURL(url, error);
	
	if(NULL == decoder)
		return NULL;
	
	if(false == decoder->SupportsSeeking()) {
		delete decoder;
		return NULL;
	}

	return new LoopableRegionDecoder(decoder, startingFrame);
}

AudioDecoder * AudioDecoder::CreateDecoderForURLRegion(CFURLRef url, SInt64 startingFrame, UInt32 frameCount, CFErrorRef *error)
{
	AudioDecoder *decoder = AudioDecoder::CreateDecoderForURL(url, error);
	
	if(NULL == decoder)
		return NULL;
	
	if(false == decoder->SupportsSeeking()) {
		delete decoder;
		return NULL;
	}
	
	return new LoopableRegionDecoder(decoder, startingFrame, frameCount);
}

AudioDecoder * AudioDecoder::CreateDecoderForURLRegion(CFURLRef url, SInt64 startingFrame, UInt32 frameCount, UInt32 repeatCount, CFErrorRef *error)
{
	AudioDecoder *decoder = AudioDecoder::CreateDecoderForURL(url, error);
	
	if(NULL == decoder)
		return NULL;
	
	// In order to repeat a decoder must support seeking
	if(false == decoder->SupportsSeeking()) {
		delete decoder;
		return NULL;
	}
	
	return new LoopableRegionDecoder(decoder, startingFrame, frameCount, repeatCount);
}


#pragma mark Creation and Destruction


AudioDecoder::AudioDecoder()
	: mInputSource(NULL)
{
	memset(&mCallbacks, 0, sizeof(mCallbacks));
	memset(&mSourceFormat, 0, sizeof(mSourceFormat));
}

AudioDecoder::AudioDecoder(InputSource *inputSource)
	: mInputSource(inputSource)
{
	assert(NULL != inputSource);

	memset(&mCallbacks, 0, sizeof(mCallbacks));
	memset(&mFormat, 0, sizeof(mSourceFormat));
	memset(&mSourceFormat, 0, sizeof(mSourceFormat));
	memset(&mChannelLayout, 0, sizeof(mChannelLayout));
}

AudioDecoder::AudioDecoder(const AudioDecoder& rhs)
	: mInputSource(NULL)
{
	*this = rhs;
}

AudioDecoder::~AudioDecoder()
{
	if(mInputSource)
		delete mInputSource, mInputSource = NULL;
}


#pragma mark Operator Overloads


AudioDecoder& AudioDecoder::operator=(const AudioDecoder& rhs)
{
	if(mInputSource)
		delete mInputSource, mInputSource = NULL;
	
	if(rhs.mInputSource)
		mInputSource = rhs.mInputSource;

	mFormat				= rhs.mFormat;
	mChannelLayout		= rhs.mChannelLayout;
	mSourceFormat		= rhs.mSourceFormat;
	
	memcpy(&mCallbacks, &rhs.mCallbacks, sizeof(rhs.mCallbacks));

	return *this;
}


#pragma mark Base Functionality


CFStringRef AudioDecoder::CreateSourceFormatDescription()
{
	CFStringRef		sourceFormatDescription		= NULL;
	UInt32			sourceFormatNameSize		= sizeof(sourceFormatDescription);
	OSStatus		result						= AudioFormatGetProperty(kAudioFormatProperty_FormatName, 
																		 sizeof(mSourceFormat), 
																		 &mSourceFormat, 
																		 &sourceFormatNameSize, 
																		 &sourceFormatDescription);

	if(noErr != result)
		ERR("AudioFormatGetProperty (kAudioFormatProperty_FormatName) failed: %i (%.4s)", result, reinterpret_cast<const char *>(&result));
	
	return sourceFormatDescription;
}

CFStringRef AudioDecoder::CreateFormatDescription()
{
	CFStringRef		sourceFormatDescription		= NULL;
	UInt32			specifierSize				= sizeof(sourceFormatDescription);
	OSStatus		result						= AudioFormatGetProperty(kAudioFormatProperty_FormatName, 
																		 sizeof(mFormat), 
																		 &mFormat, 
																		 &specifierSize, 
																		 &sourceFormatDescription);

	if(noErr != result)
		ERR("AudioFormatGetProperty (kAudioFormatProperty_FormatName) failed: %i (%.4s)", result, reinterpret_cast<const char *>(&result));
	
	return sourceFormatDescription;
}

CFStringRef AudioDecoder::CreateChannelLayoutDescription()
{
	CFStringRef		channelLayoutDescription	= NULL;
	UInt32			specifierSize				= sizeof(channelLayoutDescription);
	OSStatus		result						= AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutName, 
																		 sizeof(mChannelLayout), 
																		 &mChannelLayout, 
																		 &specifierSize, 
																		 &channelLayoutDescription);

	if(noErr != result)
		ERR("AudioFormatGetProperty (kAudioFormatProperty_ChannelLayoutName) failed: %i (%.4s)", result, reinterpret_cast<const char *>(&result));
	
	return channelLayoutDescription;
}


#pragma mark Callbacks


void AudioDecoder::SetDecodingStartedCallback(AudioDecoderCallback callback, void *context)
{
	mCallbacks[0].mCallback = callback;
	mCallbacks[0].mContext = context;
}

void AudioDecoder::SetDecodingFinishedCallback(AudioDecoderCallback callback, void *context)
{
	mCallbacks[1].mCallback = callback;
	mCallbacks[1].mContext = context;
}

void AudioDecoder::SetRenderingStartedCallback(AudioDecoderCallback callback, void *context)
{
	mCallbacks[2].mCallback = callback;
	mCallbacks[2].mContext = context;
}

void AudioDecoder::SetRenderingFinishedCallback(AudioDecoderCallback callback, void *context)
{
	mCallbacks[3].mCallback = callback;
	mCallbacks[3].mContext = context;
}

void AudioDecoder::PerformDecodingStartedCallback()
{
	if(NULL != mCallbacks[0].mCallback)
		mCallbacks[0].mCallback(mCallbacks[0].mContext, this);
}

void AudioDecoder::PerformDecodingFinishedCallback()
{
	if(NULL != mCallbacks[1].mCallback)
		mCallbacks[1].mCallback(mCallbacks[1].mContext, this);
}

void AudioDecoder::PerformRenderingStartedCallback()
{
	if(NULL != mCallbacks[2].mCallback)
		mCallbacks[2].mCallback(mCallbacks[2].mContext, this);
}

void AudioDecoder::PerformRenderingFinishedCallback()
{
	if(NULL != mCallbacks[3].mCallback)
		mCallbacks[3].mCallback(mCallbacks[3].mContext, this);
}
