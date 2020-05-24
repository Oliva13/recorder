#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "recorder.h"

unsigned addByte(unsigned char byte)
{
  avi_param_t *p_avi;
  p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param);
  putc(byte, p_avi->fOutFid);
  return 1;
}

unsigned addWord(unsigned word)
{
  // Add "word" to the file in little-endian order:
  addByte(word);
  addByte(word>>8);
  addByte(word>>16);
  addByte(word>>24);
  return 4;
}

unsigned addHalfWord(unsigned short halfWord)
{
  // Add "halfWord" to the file in little-endian order:
  addByte((unsigned char)halfWord);
  addByte((unsigned char)(halfWord>>8));
  return 2;
}

unsigned addZeroWords(unsigned numWords)
{
  unsigned i;
  for (i = 0; i < numWords; ++i)
  {
    addWord(0);
  }
  return numWords*4;
}

unsigned add4ByteString(char const* str)
{
  addByte(str[0]); addByte(str[1]); addByte(str[2]);
  addByte(str[3] == '\0' ? ' ' : str[3]); // e.g., for "AVI "
  return 4;
}

void setWord(unsigned filePosn, unsigned size)
{
	 avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param);		 
     do
     {
        if (SeekFile64(p_avi->fOutFid, filePosn, SEEK_SET) < 0)
	     break;
        addWord(size);
        if (SeekFile64(p_avi->fOutFid, 0, SEEK_END) < 0)
	     break; // go back to where we were
        return;
     } while (0);
     // One of the SeekFile64()s failed, probable because we're not a seekable file
	 printf("AVIFileSink::setWord(): SeekFile64 failed\n");
}

addFileHeader(LIST,hdrl);
  	 size += addFileHeader_avih();
	 p_avi->fIsVideo = True;
     size += addFileHeader_strl();
   	 if(p_avi->audio_is_enable)
	 {
	    p_avi->fIsVideo = False;
		size += addFileHeader_strl();
     }
	 // Then add another JUNK entry
     ++(p_avi->fJunkNumber);
     size += addFileHeader_JUNK();
addFileHeaderEnd;

addFileHeader1(avih);
    unsigned usecPerFrame = p_avi->fMovieFPS == 0 ? 0 : 1000000/(p_avi->fMovieFPS);
    size += addWord(usecPerFrame); // dwMicroSecPerFrame
    p_avi->fAVIHMaxBytesPerSecondPosition = (unsigned)TellFile64(p_avi->fOutFid);
    size += addWord(0); // dwMaxBytesPerSec (fill in later)
    size += addWord(0); // dwPaddingGranularity
    size += addWord(AVIF_TRUSTCKTYPE|AVIF_HASINDEX|AVIF_ISINTERLEAVED); // dwFlags
    p_avi->fAVIHFrameCountPosition = (unsigned)TellFile64(p_avi->fOutFid);
    size += addWord(0); // dwTotalFrames (fill in later)
    size += addWord(0); // dwInitialFrame
    size += addWord(p_avi->fNumSubsessions); // dwStreams
    size += addWord(p_avi->fBufferSize); // dwSuggestedBufferSize
    size += addWord(p_avi->fMovieWidth); // dwWidth
    size += addWord(p_avi->fMovieHeight); // dwHeight
    size += addZeroWords(4); // dwReserved
addFileHeaderEnd;

addFileHeader(LIST,strl);
	size += addFileHeader_strh();
    size += addFileHeader_strf();
    p_avi->fJunkNumber = 0;
    size += addFileHeader_JUNK();
addFileHeaderEnd;

addFileHeader1(strh);
	if (p_avi->fIsVideo)
	{
		p_avi->fAVISubsessionTag = fourChar('0','0','d','c');
		//p_avi->fAVISubsessionTag = fourChar('0'+subsessionIndex/10,'0'+subsessionIndex%10,'d','c');
		if(p_avi->stream_name == 0xaa)
		{
			p_avi->fAVICodecHandlerType = fourChar('H','E','V','C');
		}
		else
		{
			if(p_avi->stream_name == 0x55)
			{
				p_avi->fAVICodecHandlerType = fourChar('H','2','6','4');
			}
			else
			{
				p_avi->fAVICodecHandlerType = fourChar('?','?','?','?');
			}
		}
		p_avi->fAVIScale = 1;
		p_avi->fAVIRate =  p_avi->fMovieFPS;
		p_avi->fAVISize =  p_avi->fMovieWidth * p_avi->fMovieHeight * 3;
	}
	else
	{
		p_avi->fAVISubsessionTag = fourChar('0', '1', 'w', 'b');
		p_avi->fAVICodecHandlerType = 1;
		p_avi->fAVISamplingFrequency = p_avi->audio_samplerate;
		p_avi->fNumAudioChannels = 1;
		p_avi->fWAVCodecTag = p_avi->audio_encoder;
		p_avi->fAVIScale = p_avi->fAVISize = p_avi->fNumAudioChannels;
		if(p_avi->audio_encoder == 0x0045)
		{	
			p_avi->fAVIRate = p_avi->fAVISize * (p_avi->fAVISamplingFrequency/4);
		}
		else
		{
			p_avi->fAVIRate = p_avi->fAVISize * (p_avi->fAVISamplingFrequency);
		}
	}

    size += add4ByteString(p_avi->fIsVideo ? "vids" : "auds"); // fccType
    size += addWord(p_avi->fAVICodecHandlerType); // fccHandler
    size += addWord(0); // dwFlags
    size += addWord(0); // wPriority + wLanguage
    size += addWord(0); // dwInitialFrames
    size += addWord(p_avi->fAVIScale); // dwScale
    size += addWord(p_avi->fAVIRate); // dwRate
    size += addWord(0); // dwStart
    if(p_avi->fIsVideo )
	{
		p_avi->fVideoSTRHFrameCountPosition = (unsigned)TellFile64(p_avi->fOutFid);
	}
	else
	{
		p_avi->fAudioSTRHFrameCountPosition = (unsigned)TellFile64(p_avi->fOutFid);
	}
    size += addWord(0); // dwLength (fill in later)
    size += addWord(p_avi->fBufferSize); // dwSuggestedBufferSize
    size += addWord((unsigned)-1); // dwQuality
    size += addWord(p_avi->fAVISize); // dwSampleSize
    size += addWord(0); // rcFrame (start)	
    if (p_avi->fIsVideo) 
    {
        size += addHalfWord(p_avi->fMovieWidth);
        size += addHalfWord(p_avi->fMovieHeight);
    }
    else
    {
        size += addWord(0);
    }
addFileHeaderEnd;

addFileHeader1(strf);
	if (p_avi->fIsVideo)
    {
      // Add a BITMAPINFO header:flags
      unsigned extraDataSize = 0;
      size += addWord(10*4 + extraDataSize); // size
      size += addWord(p_avi->fMovieWidth);
      size += addWord(p_avi->fMovieHeight);
      size += addHalfWord(1); // planes
      size += addHalfWord(24); // bits-per-sample #####
      size += addWord(p_avi->fAVICodecHandlerType); // compr. type
      size += addWord(p_avi->fAVISize);
      size += addZeroWords(4); // ??? #####
      // Later, add extra data here (if any) #####
    }
    else
       {
          // Add a WAVFORMATEX header:
		  size += addHalfWord(p_avi->fWAVCodecTag);
          unsigned numChannels = 1; //fOurSubsession.fOurSubsession.numChannels();
		  size += addHalfWord(numChannels);
          size += addWord(p_avi->fAVISamplingFrequency);
          size += addWord(p_avi->fAVIRate); // bytes per second
          size += addHalfWord(p_avi->fAVISize); // block alignment
		  //unsigned bitsPerSample = (p_avi->fAVISize * 8)/numChannels;
		  unsigned bitsPerSample = 2;
          size += addHalfWord(bitsPerSample);
		  size += addHalfWord(0);
      }
addFileHeaderEnd;

#define AVI_MASTER_INDEX_SIZE   256

addFileHeader1(JUNK);
    if (p_avi->fJunkNumber == 0) 
    {
      size += addHalfWord(4); // wLongsPerEntry
      size += addHalfWord(0); // bIndexSubType + bIndexType
      size += addWord(0); // nEntriesInUse #####
      size += addWord(p_avi->fAVISubsessionTag); // dwChunkId
      size += addZeroWords(2); // dwReserved
      size += addZeroWords(AVI_MASTER_INDEX_SIZE*4);
    }
    else
    {
      size += add4ByteString("odml");
      size += add4ByteString("dmlh");
      unsigned wtfCount = 248;
      size += addWord(wtfCount); // ??? #####
      size += addZeroWords(wtfCount/4);
    }
addFileHeaderEnd;

addFileHeader(LIST, movi);
    p_avi->fMoviSizePosition = headerSizePosn;
    p_avi->fMoviSizeValue = size-ignoredSize;
addFileHeaderEnd;

addFileHeader(RIFF,AVI);
	size += addFileHeader_hdrl();
    size += addFileHeader_movi();
    p_avi->fRIFFSizePosition = headerSizePosn;
    p_avi->fRIFFSizeValue = size-ignoredSize;
addFileHeaderEnd;














