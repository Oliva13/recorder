#ifndef _AVIUTILS_H_
#define _AVIUTILS_H_

#include  "boolean.h"

#define fourChar(x,y,z,w) ( ((w)<<24)|((z)<<16)|((y)<<8)|(x) ) //little-endian

pthread_key_t   saved_avi_param;

unsigned addWord(unsigned word);
unsigned addByte(unsigned char byte);
unsigned addHalfWord(unsigned short halfWord);
unsigned addZeroWords(unsigned numWords);
unsigned add4ByteString(char const* str);
void setWord(unsigned filePosn, unsigned size);

typedef struct 
{
	FILE* fOutFid;
	unsigned fNumBytesWritten;
	unsigned short fMovieWidth;
	unsigned short fMovieHeight;
	unsigned fMovieFPS;
	unsigned fRIFFSizePosition;
	unsigned fRIFFSizeValue;
	unsigned fAVIHMaxBytesPerSecondPosition;
	unsigned fAVIHFrameCountPosition;
	unsigned fMoviSizePosition;
	unsigned fMoviSizeValue;
	unsigned fJunkNumber;
	unsigned fNumSubsessions;
	unsigned fBufferSize;
	unsigned fNumVideoFrames;
	unsigned fNumAudioFrames;
	int64_t  fPrevVideoPts;
	int64_t  fPrevAudioPts;
	unsigned fAudioMaxBytesPerSecond;
	unsigned fVideoMaxBytesPerSecond;
	unsigned short fLastPacketRTPSeqNum;
	Boolean fOurSourceIsActive;
	struct timeval fPrevPresentationTime;
	unsigned fMaxBytesPerSecond;
	Boolean fIsVideo;
	Boolean fIsAudio;
	Boolean fIsByteSwappedAudio;
	unsigned fAVISubsessionTag;
	unsigned fAVICodecHandlerType;
	unsigned fAVISamplingFrequency; // for audio
	u_int16_t fWAVCodecTag; // for audio
	unsigned fAVIScale;
	unsigned fAVIRate;
	unsigned fAVISize;
	unsigned fVideoSTRHFrameCountPosition;
	unsigned fAudioSTRHFrameCountPosition;
	unsigned fNumAudioChannels;
	//char    *fCodecName;
	//char    *fMediumName;
	unsigned fRTPTimestampFrequency;
	unsigned short rtpSeqNum;
	unsigned fNumIndexRecords;
	struct 	 AVIIndexRecord  *fIndexRecordsHead;
	struct   AVIIndexRecord  *fIndexRecordsTail;
	int		 audio_is_enable;
	u_int16_t	 audio_encoder;
	int      audio_samplerate;
	int     stream_name; // 0x55 - 264, 0xaa - 265
	
}avi_param_t;

#define AVIF_HASINDEX           0x00000010 // Index at end of file?
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800 // Use CKType to find key frames?
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000

#define addFileHeader(tag,name) \
	unsigned addFileHeader_##name() { \
    avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param); \
    add4ByteString("" #tag ""); \
	unsigned headerSizePosn = (unsigned)TellFile64(p_avi->fOutFid); addWord(0); \
    add4ByteString("" #name ""); \
    unsigned ignoredSize = 8;/*don't include size of tag or size fields*/ \
    unsigned size = 12

#define addFileHeader1(name) \
   unsigned addFileHeader_##name() { \
   avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param); \
   add4ByteString("" #name ""); \
   unsigned headerSizePosn = (unsigned)TellFile64(p_avi->fOutFid); addWord(0); \
   unsigned ignoredSize = 8;/*don't include size of name or size fields*/ \
   unsigned size = 8

#define addFileHeaderEnd \
  setWord(headerSizePosn, size-ignoredSize); \
  return size; \
}

#define _header(name) unsigned addFileHeader_##name()
  _header(AVI);
  _header(hdrl);
  _header(avih);
  _header(strl);
  _header(strh);
  _header(strf);
  _header(JUNK);
  _header(movi);
#endif
