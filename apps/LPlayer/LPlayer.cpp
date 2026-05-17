#define LT_INSTANCE_TYPE Instance
#include <light.h>
#include <shared/strings.cpp>

#include "libc_compat.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#define HAVE_MINIMP3

#define LT_DEVICE_CONTROL_AUDIO_SET_FORMAT   ((EsDeviceControlType) 0x4001)
#define LT_DEVICE_CONTROL_AUDIO_WRITE        ((EsDeviceControlType) 0x4002)
#define LT_DEVICE_CONTROL_AUDIO_PLAY         ((EsDeviceControlType) 0x4003)
#define LT_DEVICE_CONTROL_AUDIO_PAUSE        ((EsDeviceControlType) 0x4004)
#define LT_DEVICE_CONTROL_AUDIO_STOP         ((EsDeviceControlType) 0x4005)
#define LT_DEVICE_CONTROL_AUDIO_GET_POSITION ((EsDeviceControlType) 0x4006)
#define LT_DEVICE_CONTROL_AUDIO_SET_VOLUME   ((EsDeviceControlType) 0x4007)

#define TIMER_INTERVAL_MS (500)
#define VIDEO_AREA_HEIGHT (360)
#define VOLUME_DEFAULT    (0.8)

struct AudioFormat {
	uint32_t sampleRate;
	uint16_t channels;
	uint16_t bitsPerSample;
};

// ─── WAV parser ───────────────────────────────────────────────────────────────

struct WAVInfo {
	bool valid; AudioFormat fmt;
	uint32_t dataOffset, dataSize;
	double durationSec;
};

static WAVInfo ParseWAV(const uint8_t *d, size_t sz) {
	WAVInfo info = {};
	if (sz<44) return info;
	if (d[0]!='R'||d[1]!='I'||d[2]!='F'||d[3]!='F') return info;
	if (d[8]!='W'||d[9]!='A'||d[10]!='V'||d[11]!='E') return info;
	uint32_t off=12; uint16_t afmt=0; bool hf=false,hd=false;
	while (off+8<=sz) {
		uint32_t csz=(uint32_t)d[off+4]|((uint32_t)d[off+5]<<8)|((uint32_t)d[off+6]<<16)|((uint32_t)d[off+7]<<24);
		if (d[off]=='f'&&d[off+1]=='m'&&d[off+2]=='t'&&d[off+3]==' ') {
			if (off+8+16>sz) return info;
			afmt=(uint16_t)d[off+8]|((uint16_t)d[off+9]<<8);
			info.fmt.channels=(uint16_t)d[off+10]|((uint16_t)d[off+11]<<8);
			info.fmt.sampleRate=(uint32_t)d[off+12]|((uint32_t)d[off+13]<<8)|((uint32_t)d[off+14]<<16)|((uint32_t)d[off+15]<<24);
			info.fmt.bitsPerSample=(uint16_t)d[off+22]|((uint16_t)d[off+23]<<8);
			hf=true;
		} else if (d[off]=='d'&&d[off+1]=='a'&&d[off+2]=='t'&&d[off+3]=='a') {
			info.dataOffset=off+8; info.dataSize=csz; hd=true;
		}
		off+=8+((csz+1)&~1u);
	}
	if (!hf||!hd||afmt!=1||!info.fmt.channels||!info.fmt.sampleRate||!info.fmt.bitsPerSample) return info;
	uint32_t bps=info.fmt.sampleRate*info.fmt.channels*(info.fmt.bitsPerSample/8);
	if (!bps) return info;
	info.durationSec=(double)info.dataSize/bps; info.valid=true;
	return info;
}

// ─── MP4 box utilities ────────────────────────────────────────────────────────

static inline uint32_t B32(const uint8_t *p) {
	return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static inline uint64_t B64(const uint8_t *p) {
	return ((uint64_t)B32(p)<<32)|(uint64_t)B32(p+4);
}
static inline uint32_t BTYPE(const char *s) {
	return ((uint32_t)(uint8_t)s[0]<<24)|((uint32_t)(uint8_t)s[1]<<16)
	      |((uint32_t)(uint8_t)s[2]<<8)|(uint8_t)s[3];
}

struct Box { uint64_t ds, de; };

static bool BoxFind(const uint8_t *f, size_t fs, uint64_t from, uint64_t to,
		const char *t4, Box *out) {
	uint32_t want = BTYPE(t4);
	uint64_t pos  = from;
	while (pos+8 <= to && pos+8 <= fs) {
		uint32_t s32 = B32(f+pos);
		uint32_t typ = B32(f+pos+4);
		uint64_t hdr, end;
		if (s32==1) {
			if (pos+16>fs) break;
			end=pos+B64(f+pos+8); hdr=16;
		} else if (s32==0) { end=fs; hdr=8; }
		else { end=pos+s32; hdr=8; }
		if (end>fs||end<=pos) break;
		if (typ==want) { out->ds=pos+hdr; out->de=end; return true; }
		pos=end;
	}
	return false;
}

static uint32_t StscSpc(const uint8_t *stsc, uint32_t cnt, uint32_t ci) {
	uint32_t r=1;
	for (uint32_t i=0; i<cnt; i++) {
		uint32_t fc=B32(stsc+i*12)-1;
		if (fc<=ci) r=B32(stsc+i*12+4); else break;
	}
	return r;
}

// Extract all sample bytes from stbl — returns malloc'd buffer, fills *outBytes.
static uint8_t *ExtractSamples(const uint8_t *file, size_t fs, Box stbl, size_t *outBytes) {
	*outBytes=0;
	Box stsz_b, stco_b, stsc_b;
	bool hasCo64=false; Box co64_b;
	if (!BoxFind(file,fs,stbl.ds,stbl.de,"stsz",&stsz_b)||stsz_b.de-stsz_b.ds<12) return nullptr;
	uint32_t defSz=B32(file+stsz_b.ds+4), cnt=B32(file+stsz_b.ds+8);
	if (!cnt) return nullptr;
	uint32_t chCnt=0; const uint8_t *chOff=nullptr;
	if (BoxFind(file,fs,stbl.ds,stbl.de,"stco",&stco_b)&&stco_b.de-stco_b.ds>=8) {
		chCnt=B32(file+stco_b.ds+4); chOff=file+stco_b.ds+8;
	} else if (BoxFind(file,fs,stbl.ds,stbl.de,"co64",&co64_b)&&co64_b.de-co64_b.ds>=8) {
		hasCo64=true; chCnt=B32(file+co64_b.ds+4); chOff=file+co64_b.ds+8;
	}
	if (!chCnt||!chOff) return nullptr;
	if (!BoxFind(file,fs,stbl.ds,stbl.de,"stsc",&stsc_b)||stsc_b.de-stsc_b.ds<8) return nullptr;
	uint32_t stscCnt=B32(file+stsc_b.ds+4);
	const uint8_t *stscD=file+stsc_b.ds+8;

	auto getSz=[&](uint32_t si)->uint32_t {
		if (defSz) return defSz;
		if (stsz_b.de-stsz_b.ds>=12+si*4+4) return B32(file+stsz_b.ds+12+si*4);
		return 0;
	};

	uint64_t total=0; uint32_t si=0;
	for (uint32_t ci=0; ci<chCnt&&si<cnt; ci++) {
		uint32_t spc=StscSpc(stscD,stscCnt,ci);
		for (uint32_t s=0; s<spc&&si<cnt; s++,si++) total+=getSz(si);
	}
	if (!total||total>800*1024*1024) return nullptr;
	uint8_t *buf=(uint8_t *)EsHeapAllocate((size_t)total,false);
	if (!buf) return nullptr;
	size_t pos=0; si=0;
	for (uint32_t ci=0; ci<chCnt&&si<cnt; ci++) {
		uint64_t off=hasCo64?B64(chOff+ci*8):(uint64_t)B32(chOff+ci*4);
		uint32_t spc=StscSpc(stscD,stscCnt,ci);
		uint64_t soff=off;
		for (uint32_t s=0; s<spc&&si<cnt; s++,si++) {
			uint32_t sz=getSz(si);
			if (sz&&soff+sz<=fs&&pos+sz<=(size_t)total) {
				EsMemoryCopy(buf+pos,file+soff,sz); pos+=sz;
			}
			soff+=sz;
		}
	}
	*outBytes=(size_t)total;
	return buf;
}

// ─── Full MP4 parse result ────────────────────────────────────────────────────

struct MP4Result {
	bool hasVideo, hasAudio;
	uint32_t videoW, videoH;
	double durationSec;
	uint8_t *audioBuf; size_t audioBufBytes;
	bool audioBufIsMP3;
	uint32_t audioSampleRate; uint16_t audioChannels, audioBitsPerSample;
	char audioCodec[8], videoCodec[8];
};

static MP4Result ParseMP4Full(const uint8_t *file, size_t fs) {
	MP4Result r={};
	Box moov;
	if (!BoxFind(file,fs,0,fs,"moov",&moov)) return r;

	// mvhd duration
	Box mvhd;
	if (BoxFind(file,fs,moov.ds,moov.de,"mvhd",&mvhd)&&mvhd.de-mvhd.ds>=4) {
		uint8_t ver=file[mvhd.ds];
		if (ver==1&&mvhd.de-mvhd.ds>=28) {
			uint32_t ts=B32(file+mvhd.ds+20); uint64_t du=B64(file+mvhd.ds+24);
			if (ts) r.durationSec=(double)du/ts;
		} else if (mvhd.de-mvhd.ds>=20) {
			uint32_t ts=B32(file+mvhd.ds+12),du=B32(file+mvhd.ds+16);
			if (ts) r.durationSec=(double)du/ts;
		}
	}

	// Walk trak boxes
	uint64_t trakPos=moov.ds; Box trak;
	while (BoxFind(file,fs,trakPos,moov.de,"trak",&trak)) {
		trakPos=trak.de;
		Box mdia;
		if (!BoxFind(file,fs,trak.ds,trak.de,"mdia",&mdia)) continue;
		Box hdlr;
		if (!BoxFind(file,fs,mdia.ds,mdia.de,"hdlr",&hdlr)||hdlr.de-hdlr.ds<12) continue;
		uint32_t handler=B32(file+hdlr.ds+8);

		// Video
		if (handler==BTYPE("vide")) {
			r.hasVideo=true;
			Box tkhd;
			if (BoxFind(file,fs,trak.ds,trak.de,"tkhd",&tkhd)&&tkhd.de-tkhd.ds>=84) {
				uint8_t ver=file[tkhd.ds];
				uint64_t wo=(ver==1)?76:76;
				r.videoW=B32(file+tkhd.ds+wo)>>16;
				r.videoH=B32(file+tkhd.ds+wo+4)>>16;
			}
			Box minf,stbl,stsd;
			if (BoxFind(file,fs,mdia.ds,mdia.de,"minf",&minf)&&
			    BoxFind(file,fs,minf.ds,minf.de,"stbl",&stbl)&&
			    BoxFind(file,fs,stbl.ds,stbl.de,"stsd",&stsd)&&stsd.de-stsd.ds>=12) {
				uint32_t c=B32(file+stsd.ds+12);
				r.videoCodec[0]=(char)(c>>24);r.videoCodec[1]=(char)(c>>16);
				r.videoCodec[2]=(char)(c>>8); r.videoCodec[3]=(char)c; r.videoCodec[4]=0;
			}
			continue;
		}

		// Audio
		if (handler!=BTYPE("soun")) continue;
		r.hasAudio=true;
		Box minf,stbl,stsd;
		if (!BoxFind(file,fs,mdia.ds,mdia.de,"minf",&minf)) continue;
		if (!BoxFind(file,fs,minf.ds,minf.de,"stbl",&stbl)) continue;
		if (!BoxFind(file,fs,stbl.ds,stbl.de,"stsd",&stsd)||stsd.de-stsd.ds<12) continue;

		uint32_t codecType=B32(file+stsd.ds+12);
		r.audioCodec[0]=(char)(codecType>>24);r.audioCodec[1]=(char)(codecType>>16);
		r.audioCodec[2]=(char)(codecType>>8); r.audioCodec[3]=(char)codecType;r.audioCodec[4]=0;

		// Find codec entry box
		Box codecBox;
		if (!BoxFind(file,fs,stsd.ds+8,stsd.de,r.audioCodec,&codecBox)||codecBox.de-codecBox.ds<28) continue;

		r.audioChannels     =(uint16_t)((file[codecBox.ds+16]<<8)|file[codecBox.ds+17]);
		r.audioBitsPerSample=(uint16_t)((file[codecBox.ds+18]<<8)|file[codecBox.ds+19]);
		r.audioSampleRate   = B32(file+codecBox.ds+24)>>16;

		bool isMP3=(codecType==BTYPE("mp3 ")||codecType==BTYPE(".mp3")||
		            codecType==0x6D730055u); // .mp3 variant
		bool isLPCM=(codecType==BTYPE("lpcm")||codecType==BTYPE("sowt")||
		             codecType==BTYPE("twos")||codecType==BTYPE("raw ")||
		             codecType==BTYPE("in16")||codecType==BTYPE("in24")||
		             codecType==BTYPE("in32"));
		if (!isMP3&&!isLPCM) continue;

		size_t bytes=0;
		uint8_t *buf=ExtractSamples(file,fs,stbl,&bytes);
		if (!buf||!bytes) { if (buf) EsHeapFree(buf); continue; }

		r.audioBuf=buf; r.audioBufBytes=bytes; r.audioBufIsMP3=isMP3;
		if (!r.audioSampleRate) r.audioSampleRate=44100;
		if (!r.audioChannels)   r.audioChannels=2;
		if (!r.audioBitsPerSample) r.audioBitsPerSample=16;
	}
	return r;
}

// ─── Styles ───────────────────────────────────────────────────────────────────

static const EsStyle styleControlPanel = {
	.inherit = LT_STYLE_PANEL_WINDOW_BACKGROUND,
	.metrics = {
		.mask=LT_THEME_METRICS_INSETS|LT_THEME_METRICS_GAP_MAJOR,
		.insets=LT_RECT_4(10,10,8,8), .gapMajor=8,
	},
};
static const EsStyle styleTimeLabel = {
	.inherit=LT_STYLE_TEXT_LABEL_SECONDARY,
	.metrics={.mask=LT_THEME_METRICS_MINIMUM_WIDTH,.minimumWidth=90},
};

enum class MediaType  { None, Audio, Video };
enum class FileFormat { Unknown, WAV, MP3, MP4 };

static FileFormat GetFormat(const char *ext, size_t len) {
	if (!EsStringCompare(ext,len,EsLiteral("wav"))) return FileFormat::WAV;
	if (!EsStringCompare(ext,len,EsLiteral("mp3"))) return FileFormat::MP3;
	if (!EsStringCompare(ext,len,EsLiteral("mp4"))) return FileFormat::MP4;
	return FileFormat::Unknown;
}

// ─── Instance ─────────────────────────────────────────────────────────────────

struct Instance : EsInstance {
	MediaType  mediaType; FileFormat fileFormat;
	bool isPlaying, hasVideoTrack, hasAudioTrack, canPlayAudio, canShowVideo;
	double durationSec, positionSec, volume;
	uint8_t *fileData; size_t fileSize;
	uint8_t *pcmBuffer; uint32_t pcmSize; bool pcmOwned;
	uint64_t bytesPlayedBase; AudioFormat audioFmt;
	EsPaintTarget *videoFrame; uint32_t videoFrameW, videoFrameH;
	EsElement *videoArea;
	EsTextDisplay *labelTitle, *labelTime, *labelStatus, *labelVolume;
	EsButton *btnPlayPause;
	EsSlider *sliderSeek, *sliderVolume;
	EsTimer uiTimer; EsHandle audioDevice;
};

// ─── Helpers ──────────────────────────────────────────────────────────────────

static size_t FmtTime(char *buf, size_t sz, double s) {
	if (s<0) s=0; int t=(int)s;
	return EsStringFormat(buf,sz,"%d:%02d",t/60,t%60);
}
static void UpdateTime(Instance *i) {
	char buf[64]; size_t a=FmtTime(buf,sizeof(buf),i->positionSec);
	buf[a++]=' ';buf[a++]='/';buf[a++]=' ';
	a+=FmtTime(buf+a,sizeof(buf)-a,i->durationSec);
	EsTextDisplaySetContents(i->labelTime,buf,a);
}
static void SetStatus(Instance *i, const char *t, ptrdiff_t b=-1) {
	EsTextDisplaySetContents(i->labelStatus,t,b);
}
static void SyncBtn(Instance *i) {
	EsButtonSetIcon(i->btnPlayPause,
		i->isPlaying?LT_ICON_MEDIA_PLAYBACK_PAUSE:LT_ICON_MEDIA_PLAYBACK_START);
}

// ─── Audio control ────────────────────────────────────────────────────────────

static void AStop(Instance *i) {
	if (i->audioDevice==LT_INVALID_HANDLE) return;
	EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_STOP,nullptr,nullptr);
	i->bytesPlayedBase=0;
}
static void ASetFmt(Instance *i) {
	if (i->audioDevice==LT_INVALID_HANDLE||!i->audioFmt.sampleRate) return;
	EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_SET_FORMAT,&i->audioFmt,nullptr);
}
static void AWrite(Instance *i) {
	if (i->audioDevice==LT_INVALID_HANDLE||!i->pcmBuffer||!i->pcmSize) return;
	size_t b=i->pcmSize;
	EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_WRITE,i->pcmBuffer,&b);
}
static void APlay(Instance *i)  { if (i->audioDevice!=LT_INVALID_HANDLE) EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_PLAY,nullptr,nullptr); }
static void APause(Instance *i) { if (i->audioDevice!=LT_INVALID_HANDLE) EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_PAUSE,nullptr,nullptr); }
static void AVol(Instance *i, double vol) {
	if (i->audioDevice==LT_INVALID_HANDLE) return;
	uint32_t g=(uint32_t)(vol*100+0.5); if (g>100) g=100;
	EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_SET_VOLUME,&g,nullptr);
}
static double APos(Instance *i) {
	if (i->audioDevice==LT_INVALID_HANDLE||!i->audioFmt.sampleRate) return i->positionSec;
	uint64_t bd=0;
	EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_GET_POSITION,nullptr,&bd);
	uint32_t bps=i->audioFmt.sampleRate*i->audioFmt.channels*(i->audioFmt.bitsPerSample/8);
	return bps?(double)(i->bytesPlayedBase+bd)/bps:i->positionSec;
}
static void ASeek(Instance *i, double toSec) {
	if (!i->canPlayAudio||!i->audioFmt.sampleRate) return;
	uint32_t bps=i->audioFmt.sampleRate*i->audioFmt.channels*(i->audioFmt.bitsPerSample/8);
	uint64_t sk=(uint64_t)(toSec*bps); if (sk>i->pcmSize) sk=i->pcmSize;
	if (i->audioDevice!=LT_INVALID_HANDLE)
		EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_STOP,nullptr,nullptr);
	i->bytesPlayedBase=sk;
	if (i->pcmBuffer&&sk<i->pcmSize&&i->audioDevice!=LT_INVALID_HANDLE) {
		void *p=i->pcmBuffer+sk; size_t b=i->pcmSize-(uint32_t)sk;
		EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_WRITE,p,&b);
		if (i->isPlaying)
			EsDeviceControl(i->audioDevice,LT_DEVICE_CONTROL_AUDIO_PLAY,nullptr,nullptr);
	}
}

// ─── Video frame ──────────────────────────────────────────────────────────────

static void VFrameUpdate(Instance *i, const uint32_t *px, uint32_t w, uint32_t h) {
	if (!px||!w||!h) return;
	if (i->videoFrame&&(i->videoFrameW!=w||i->videoFrameH!=h)) {
		EsPaintTargetDestroy(i->videoFrame); i->videoFrame=nullptr;
	}
	if (!i->videoFrame) {
		i->videoFrame=EsPaintTargetCreate(w,h,false);
		i->videoFrameW=w; i->videoFrameH=h;
	}
	if (!i->videoFrame) return;
	uint32_t *bits; size_t fw,fh,stride;
	EsPaintTargetStartDirectAccess(i->videoFrame,&bits,&fw,&fh,&stride);
	for (uint32_t row=0; row<h&&row<fh; row++)
		EsMemoryCopy(bits+row*stride,px+row*w,(w<fw?w:fw)*sizeof(uint32_t));
	EsPaintTargetEndDirectAccess(i->videoFrame);
	EsElementRepaint(i->videoArea);
}

// ─── MP3 decode ───────────────────────────────────────────────────────────────

static bool DecodeMp3(Instance *ins, const uint8_t *data, size_t size) {
	mp3dec_t mp3d; mp3dec_init(&mp3d);
	size_t max=((size/4)+2)*MINIMP3_MAX_SAMPLES_PER_FRAME;
	int16_t *pcm=(int16_t *)EsHeapAllocate(max*sizeof(int16_t),false);
	if (!pcm) { SetStatus(ins,"Out of memory"); return false; }
	size_t total=0, off=0; mp3dec_frame_info_t last={};
	while (off<size) {
		mp3dec_frame_info_t info;
		int n=mp3dec_decode_frame(&mp3d,data+off,(int)(size-off),pcm+total,&info);
		if (!info.frame_bytes) break;
		off+=info.frame_bytes;
		if (n>0) { total+=(size_t)n*info.channels; last=info; }
	}
	if (!total||!last.hz) { EsHeapFree(pcm); SetStatus(ins,"MP3 decode failed"); return false; }
	ins->pcmBuffer=(uint8_t *)pcm;
	ins->pcmSize=(uint32_t)(total*sizeof(int16_t));
	ins->pcmOwned=true;
	ins->audioFmt={(uint32_t)last.hz,(uint16_t)last.channels,16};
	uint32_t bps=last.hz*last.channels*2;
	ins->durationSec=bps?(double)ins->pcmSize/bps:0;
	return true;
}

// ─── Free media ───────────────────────────────────────────────────────────────

static void FreeMedia(Instance *i) {
	i->isPlaying=false; i->positionSec=0;
	AStop(i);
	if (i->uiTimer){EsTimerCancel(i->uiTimer);i->uiTimer=0;}
	if (i->pcmOwned&&i->pcmBuffer) EsHeapFree(i->pcmBuffer);
	if (i->fileData) EsHeapFree(i->fileData);
	if (i->videoFrame){EsPaintTargetDestroy(i->videoFrame);i->videoFrame=nullptr;}
	i->pcmBuffer=nullptr;i->pcmSize=0;i->pcmOwned=false;
	i->fileData=nullptr;i->fileSize=0;
	i->bytesPlayedBase=0;i->audioFmt={};
	i->durationSec=0;i->positionSec=0;
	i->hasVideoTrack=false;i->hasAudioTrack=false;
	i->canPlayAudio=false;i->canShowVideo=false;
	i->mediaType=MediaType::None;
	i->videoFrameW=0;i->videoFrameH=0;
	EsSliderSetValue(i->sliderSeek,0.0,false);
	SyncBtn(i);
}

static void FinishLoad(Instance *ins, const char *name, size_t nl) {
	if (ins->canPlayAudio&&ins->audioDevice!=LT_INVALID_HANDLE) {
		ASetFmt(ins); AWrite(ins); AVol(ins,ins->volume);
	}
	size_t ns=nl;
	while (ns>0&&name[ns-1]!='/'&&name[ns-1]!='\\') ns--;
	EsWindowSetTitle(ins->window,name+ns,nl-ns);
	EsTextDisplaySetContents(ins->labelTitle,name+ns,nl-ns);
	UpdateTime(ins); SyncBtn(ins);
	EsElementRepaint(ins->videoArea);
}

// ─── Apply file ───────────────────────────────────────────────────────────────

static void ApplyFile(Instance *ins, const char *name, size_t nl,
		uint8_t *fd, size_t fs, FileFormat fmt) {
	FreeMedia(ins);
	ins->fileData=fd; ins->fileSize=fs; ins->fileFormat=fmt;

	if (fmt==FileFormat::WAV) {
		WAVInfo wav=ParseWAV(fd,fs);
		if (!wav.valid) { EsHeapFree(fd);ins->fileData=nullptr;SetStatus(ins,"Invalid WAV");return; }
		ins->mediaType=MediaType::Audio; ins->hasAudioTrack=ins->canPlayAudio=true;
		ins->audioFmt=wav.fmt; ins->pcmBuffer=fd+wav.dataOffset;
		ins->pcmSize=wav.dataSize; ins->pcmOwned=false;
		ins->durationSec=wav.durationSec;
		SetStatus(ins,"WAV — Ready");
		FinishLoad(ins,name,nl);

	} else if (fmt==FileFormat::MP3) {
		ins->mediaType=MediaType::Audio; ins->hasAudioTrack=true;
		if (DecodeMp3(ins,fd,fs)) {
			ins->canPlayAudio=true; SetStatus(ins,"MP3 decoded — Ready");
		}
		FinishLoad(ins,name,nl);

	} else if (fmt==FileFormat::MP4) {
		MP4Result mp4=ParseMP4Full(fd,fs);
		ins->hasVideoTrack=mp4.hasVideo; ins->hasAudioTrack=mp4.hasAudio;
		ins->mediaType=mp4.hasVideo?MediaType::Video:MediaType::Audio;
		if (mp4.durationSec>0) ins->durationSec=mp4.durationSec;

		// Video placeholder
		if (mp4.hasVideo) {
			ins->canShowVideo=true;
			ins->videoFrameW=mp4.videoW>0?mp4.videoW:320;
			ins->videoFrameH=mp4.videoH>0?mp4.videoH:240;
			uint32_t w=ins->videoFrameW,h=ins->videoFrameH;
			uint32_t *px=(uint32_t *)EsHeapAllocate(w*h*4,false);
			if (px) {
				for (uint32_t n=0;n<w*h;n++) px[n]=0xFF101010;
				VFrameUpdate(ins,px,w,h); EsHeapFree(px);
			}
		}

		// Audio
		bool audioOK=false;
		if (mp4.audioBuf&&mp4.audioBufBytes>0) {
			if (mp4.audioBufIsMP3) {
				if (DecodeMp3(ins,mp4.audioBuf,mp4.audioBufBytes)) {
					ins->canPlayAudio=true; audioOK=true;
				}
				EsHeapFree(mp4.audioBuf);
			} else {
				// Raw LPCM — use directly (don't free, ins owns it)
				ins->pcmBuffer=mp4.audioBuf;
				ins->pcmSize=(uint32_t)mp4.audioBufBytes;
				ins->pcmOwned=true;
				ins->audioFmt={mp4.audioSampleRate,mp4.audioChannels,mp4.audioBitsPerSample};
				if (ins->audioFmt.sampleRate&&ins->audioFmt.channels&&ins->audioFmt.bitsPerSample) {
					uint32_t bps=ins->audioFmt.sampleRate*ins->audioFmt.channels*(ins->audioFmt.bitsPerSample/8);
					if (bps&&ins->durationSec<=0) ins->durationSec=(double)ins->pcmSize/bps;
					ins->canPlayAudio=true; audioOK=true;
				} else {
					EsHeapFree(mp4.audioBuf);
					ins->pcmBuffer=nullptr; ins->pcmOwned=false;
				}
			}
		} else if (mp4.audioBuf) {
			EsHeapFree(mp4.audioBuf);
		}

		// Status
		char st[160]; size_t slen=0;
		if (mp4.hasVideo) {
			if (mp4.videoW&&mp4.videoH)
				slen=EsStringFormat(st,sizeof(st),"Video: %s %dx%d",mp4.videoCodec,mp4.videoW,mp4.videoH);
			else
				slen=EsStringFormat(st,sizeof(st),"Video: %s",mp4.videoCodec);
		}
		if (mp4.hasAudio) {
			const char *sep=slen?" | ":"";
			if (audioOK)
				slen+=EsStringFormat(st+slen,sizeof(st)-slen,"%sAudio OK (%dHz %dch)",sep,mp4.audioSampleRate,mp4.audioChannels);
			else
				slen+=EsStringFormat(st+slen,sizeof(st)-slen,"%sAudio: %s (no decoder)",sep,mp4.audioCodec);
		}
		if (!mp4.hasVideo&&!mp4.hasAudio)
			slen=EsStringFormat(st,sizeof(st),"MP4: no tracks found");
		SetStatus(ins,st,slen);
		FinishLoad(ins,name,nl);
	}
}

// ─── Open dialog ──────────────────────────────────────────────────────────────

struct ODS { Instance *ins; EsDialog *dlg; EsTextbox *tbox; };
static ODS gOD={};

static uint8_t *ReadFile(const char *path, size_t pb, size_t *out) {
	*out=0;
	EsFileInformation info=EsFileOpen(path,(ptrdiff_t)pb,LT_FILE_READ);
	if (info.error!=LT_SUCCESS) return nullptr;
	EsError err=LT_SUCCESS;
	uint8_t *data=(uint8_t *)EsFileReadAllFromHandle(info.handle,out,&err);
	EsHandleClose(info.handle);
	if (err!=LT_SUCCESS||!data) return nullptr;
	return data;
}

static void ODCancel(Instance *, EsElement *, EsCommand *) {
	if (gOD.dlg){EsDialogClose(gOD.dlg);gOD={};}
}
static void ODConfirm(Instance *ins, EsElement *, EsCommand *) {
	if (!gOD.tbox) return;
	size_t pb=0; char *path=EsTextboxGetContents(gOD.tbox,&pb);
	EsDialogClose(gOD.dlg); gOD={};
	if (!path||!pb){EsHeapFree(path);return;}
	size_t xo=pb;
	while (xo>0&&path[xo-1]!='.'&&path[xo-1]!='/'&&path[xo-1]!='\\') xo--;
	FileFormat fmt=(xo>0&&path[xo-1]=='.')?GetFormat(path+xo,pb-xo):FileFormat::Unknown;
	if (fmt==FileFormat::Unknown){SetStatus(ins,"Unknown format");EsHeapFree(path);return;}
	size_t fs=0; uint8_t *fd=ReadFile(path,pb,&fs);
	if (!fd){char m[256];size_t n=EsStringFormat(m,sizeof(m),"Cannot open: %s",path);SetStatus(ins,m,n);EsHeapFree(path);return;}
	ApplyFile(ins,path,pb,fd,fs,fmt); EsHeapFree(path);
}
static void CmdOpen(Instance *ins, EsElement *, EsCommand *) {
	if (gOD.dlg){EsDialogClose(gOD.dlg);gOD={};}
	EsDialog *dlg=EsDialogShow(ins->window,EsLiteral("Open File"),
		EsLiteral("Path: e.g. 0:/Music/song.wav"),LT_ICON_FOLDER_OPEN,LT_FLAGS_DEFAULT);
	EsElement *area=EsDialogGetContentArea(dlg);
	EsTextbox *tbox=EsTextboxCreate(area,LT_CELL_H_FILL);
	EsDialogAddButton(dlg,LT_FLAGS_DEFAULT,0,EsLiteral("Cancel"),ODCancel);
	EsDialogAddButton(dlg,LT_FLAGS_DEFAULT,0,EsLiteral("Open"),ODConfirm);
	gOD={ins,dlg,tbox};
}

// ─── Playback ─────────────────────────────────────────────────────────────────

static void StopPb(Instance *i) {
	i->isPlaying=false; i->positionSec=0; AStop(i);
	if (i->uiTimer){EsTimerCancel(i->uiTimer);i->uiTimer=0;}
	EsSliderSetValue(i->sliderSeek,0.0,false); UpdateTime(i); SyncBtn(i);
}

static void OnTimer(EsGeneric ctx) {
	Instance *i=(Instance *)ctx.p;
	if (i->isPlaying&&i->canPlayAudio&&i->durationSec>0) {
		double pos=APos(i);
		if (pos>=i->durationSec) {
			i->positionSec=i->durationSec; i->isPlaying=false;
			AStop(i); SyncBtn(i); SetStatus(i,"Stopped");
			EsElementRepaint(i->videoArea);
		} else {
			i->positionSec=pos;
			EsSliderSetValue(i->sliderSeek,pos/i->durationSec,false);
			UpdateTime(i);
		}
	}
	if (i->isPlaying) {
		i->uiTimer=EsTimerSet(TIMER_INTERVAL_MS,[](EsGeneric g){OnTimer(g);},ctx);
	} else { i->uiTimer=0; }
}

static void CmdPlayPause(Instance *i, EsElement *, EsCommand *) {
	if (i->mediaType==MediaType::None){SetStatus(i,"No file");return;}
	if (!i->canPlayAudio) {
		SetStatus(i,i->canShowVideo?"Video only (H.264 not supported)":"Codec not supported");
		return;
	}
	i->isPlaying=!i->isPlaying;
	if (i->isPlaying) {
		APlay(i); SetStatus(i,"Playing");
		i->uiTimer=EsTimerSet(TIMER_INTERVAL_MS,[](EsGeneric g){OnTimer(g);},(void *)i);
	} else {
		APause(i); SetStatus(i,"Paused");
		if (i->uiTimer){EsTimerCancel(i->uiTimer);i->uiTimer=0;}
	}
	SyncBtn(i); EsElementRepaint(i->videoArea);
}
static void CmdStop(Instance *i, EsElement *, EsCommand *) {
	if (i->mediaType==MediaType::None) return;
	StopPb(i); SetStatus(i,"Stopped"); EsElementRepaint(i->videoArea);
}

// ─── Video area paint ─────────────────────────────────────────────────────────

static int VideoMsg(EsElement *el, EsMessage *msg) {
	Instance *i=(Instance *)el->instance;
	if (msg->type==LT_MSG_PAINT) {
		EsPainter *p=msg->painter; EsRectangle b=EsPainterBoundsInset(p);
		EsDrawBlock(p,b,0xFF111111);
		if (i->videoFrame&&i->canShowVideo) {
			EsRectangle src=LT_RECT_4(0,(int32_t)i->videoFrameW,0,(int32_t)i->videoFrameH);
			EsDrawPaintTarget(p,i->videoFrame,EsRectangleFit(b,src,false),src,0xFF);
		} else {
			EsDrawContent(p,el,EsRectangleFit(b,LT_RECT_1S(80),false),nullptr,0,
				i->canShowVideo?LT_ICON_MULTIMEDIA_VIDEO_PLAYER:LT_ICON_MULTIMEDIA_AUDIO_PLAYER,
				LT_FLAGS_DEFAULT);
		}
		if (i->canShowVideo&&!i->isPlaying&&i->videoFrame)
			EsDrawContent(p,el,EsRectangleFit(b,LT_RECT_1S(48),false),
				nullptr,0,LT_ICON_MEDIA_PLAYBACK_PAUSE,LT_FLAGS_DEFAULT);
	} else if (msg->type==LT_MSG_GET_HEIGHT) {
		float sc=EsElementGetScaleFactor(el);
		msg->measure.height=(int32_t)(VIDEO_AREA_HEIGHT*sc);
	} else if (msg->type==LT_MSG_MOUSE_LEFT_DOWN) {
		CmdPlayPause(i,el,nullptr);
	} else return 0;
	return LT_HANDLED;
}

// ─── File Manager open ────────────────────────────────────────────────────────

static void OpenMedia(Instance *ins, EsMessage *msg) {
	const char *name=msg->instanceOpen.nameOrPath; size_t nl=msg->instanceOpen.nameOrPathBytes;
	size_t xo=nl;
	while (xo>0&&name[xo-1]!='.'&&name[xo-1]!='/'&&name[xo-1]!='\\') xo--;
	FileFormat fmt=(xo>0&&name[xo-1]=='.')?GetFormat(name+xo,nl-xo):FileFormat::Unknown;
	if (fmt==FileFormat::Unknown) {
		EsInstanceOpenComplete(ins,msg->instanceOpen.file,false,EsLiteral("Unsupported format."));return;
	}
	size_t fs=0; uint8_t *fd=(uint8_t *)EsFileStoreReadAll(msg->instanceOpen.file,&fs);
	if (!fd){EsInstanceOpenComplete(ins,msg->instanceOpen.file,false,EsLiteral("Read failed."));return;}
	ApplyFile(ins,name,nl,fd,fs,fmt);
	EsInstanceOpenComplete(ins,msg->instanceOpen.file,true);
}

// ─── Instance callback ────────────────────────────────────────────────────────

static int InstanceCB(Instance *ins, EsMessage *msg) {
	if (msg->type==LT_MSG_INSTANCE_OPEN){OpenMedia(ins,msg);return LT_HANDLED;}
	if (msg->type==LT_MSG_INSTANCE_DESTROY){
		if (ins->uiTimer) EsTimerCancel(ins->uiTimer);
		AStop(ins);
		if (ins->pcmOwned&&ins->pcmBuffer) EsHeapFree(ins->pcmBuffer);
		if (ins->fileData) EsHeapFree(ins->fileData);
		if (ins->videoFrame) EsPaintTargetDestroy(ins->videoFrame);
		return LT_HANDLED;
	}
	if (msg->type==LT_MSG_DEVICE_CONNECTED&&msg->device.type==LT_DEVICE_AUDIO&&
			ins->audioDevice==LT_INVALID_HANDLE) {
		ins->audioDevice=msg->device.handle;
		if (ins->canPlayAudio&&ins->pcmBuffer&&ins->audioFmt.sampleRate) { ASetFmt(ins);AWrite(ins); }
		AVol(ins,ins->volume); return LT_HANDLED;
	}
	if (msg->type==LT_MSG_DEVICE_DISCONNECTED&&msg->device.handle==ins->audioDevice) {
		ins->audioDevice=LT_INVALID_HANDLE;
		if (ins->uiTimer){EsTimerCancel(ins->uiTimer);ins->uiTimer=0;}
		ins->isPlaying=false; SyncBtn(ins); SetStatus(ins,"Device disconnected");
		return LT_HANDLED;
	}
	return 0;
}

// ─── Sliders ──────────────────────────────────────────────────────────────────

static int SeekMsg(EsElement *el, EsMessage *msg) {
	Instance *i=(Instance *)el->instance;
	if (msg->type==LT_MSG_SLIDER_MOVED&&i->durationSec>0&&i->canPlayAudio) {
		i->positionSec=msg->sliderMoved.value*i->durationSec;
		ASeek(i,i->positionSec); UpdateTime(i);
	}
	return 0;
}
static int VolMsg(EsElement *el, EsMessage *msg) {
	Instance *i=(Instance *)el->instance;
	if (msg->type==LT_MSG_SLIDER_MOVED) {
		i->volume=msg->sliderMoved.value; AVol(i,i->volume);
		char buf[16]; size_t n=EsStringFormat(buf,sizeof(buf),"%d%%",(int)(i->volume*100+0.5));
		EsTextDisplaySetContents(i->labelVolume,buf,n);
	}
	return 0;
}

// ─── Create instance ──────────────────────────────────────────────────────────

static void InstanceCreate(EsMessage *msg) {
	Instance *i=EsInstanceCreate(msg,EsLiteral("Light Player"));
	i->callback=InstanceCB;
	i->mediaType=MediaType::None; i->fileFormat=FileFormat::Unknown;
	i->isPlaying=i->hasVideoTrack=i->hasAudioTrack=i->canPlayAudio=i->canShowVideo=false;
	i->durationSec=i->positionSec=0; i->volume=VOLUME_DEFAULT;
	i->fileData=nullptr; i->fileSize=0;
	i->pcmBuffer=nullptr; i->pcmSize=0; i->pcmOwned=false;
	i->bytesPlayedBase=0; i->audioFmt={};
	i->videoFrame=nullptr; i->videoFrameW=i->videoFrameH=0;
	i->uiTimer=0; i->audioDevice=LT_INVALID_HANDLE;

	EsWindowSetIcon(i->window,LT_ICON_MULTIMEDIA_AUDIO_PLAYER);
	EsPanel *root=EsPanelCreate(i->window,LT_CELL_FILL,LT_STYLE_PANEL_WINDOW_BACKGROUND);

	i->videoArea=EsCustomElementCreate(root,LT_CELL_H_FILL|LT_ELEMENT_FOCUSABLE);
	i->videoArea->messageUser=VideoMsg;

	i->labelTitle=EsTextDisplayCreate(root,LT_CELL_H_FILL,LT_STYLE_TEXT_HEADING2,EsLiteral("No file loaded"));

	EsPanel *ctrl=EsPanelCreate(root,LT_CELL_H_FILL,EsStyleIntern(&styleControlPanel));
	EsPanel *seekRow=EsPanelCreate(ctrl,LT_CELL_H_FILL|LT_PANEL_HORIZONTAL);
	i->labelTime=EsTextDisplayCreate(seekRow,LT_FLAGS_DEFAULT,EsStyleIntern(&styleTimeLabel),EsLiteral("0:00 / 0:00"));
	i->sliderSeek=EsSliderCreate(seekRow,LT_CELL_H_FILL,0,0.0,0);
	i->sliderSeek->messageUser=SeekMsg;

	EsPanel *btnRow=EsPanelCreate(ctrl,LT_CELL_H_FILL|LT_PANEL_HORIZONTAL);
	EsButton *bOpen=EsButtonCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_PUSH_BUTTON_TOOLBAR_BIG);
	EsButtonSetIcon(bOpen,LT_ICON_FOLDER_OPEN); EsButtonOnCommand(bOpen,CmdOpen);
	EsSpacerCreate(btnRow,LT_FLAGS_DEFAULT,0,6,0);
	EsButton *bStop=EsButtonCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_PUSH_BUTTON_TOOLBAR_BIG);
	EsButtonSetIcon(bStop,LT_ICON_MEDIA_PLAYBACK_STOP); EsButtonOnCommand(bStop,CmdStop);
	EsSpacerCreate(btnRow,LT_FLAGS_DEFAULT,0,6,0);
	i->btnPlayPause=EsButtonCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_PUSH_BUTTON_TOOLBAR_BIG);
	EsButtonSetIcon(i->btnPlayPause,LT_ICON_MEDIA_PLAYBACK_START);
	EsButtonOnCommand(i->btnPlayPause,CmdPlayPause);
	EsSpacerCreate(btnRow,LT_CELL_FILL);
	i->labelStatus=EsTextDisplayCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_TEXT_LABEL_SECONDARY,EsLiteral("No file"));
	EsSpacerCreate(btnRow,LT_CELL_FILL);
	EsButton *volIco=EsButtonCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_PUSH_BUTTON_TOOLBAR_BIG);
	EsButtonSetIcon(volIco,LT_ICON_MEDIA_EQ_SYMBOLIC);
	EsSpacerCreate(btnRow,LT_FLAGS_DEFAULT,0,4,0);
	i->sliderVolume=EsSliderCreate(btnRow,LT_FLAGS_DEFAULT,0,VOLUME_DEFAULT,0);
	i->sliderVolume->messageUser=VolMsg;
	EsSpacerCreate(btnRow,LT_FLAGS_DEFAULT,0,6,0);
	i->labelVolume=EsTextDisplayCreate(btnRow,LT_FLAGS_DEFAULT,LT_STYLE_TEXT_LABEL_SECONDARY,EsLiteral("80%"));

	SyncBtn(i); UpdateTime(i);
}

void _start() {
	_init();
	while (true) {
		EsMessage *msg=EsMessageReceive();
		if (msg->type==LT_MSG_INSTANCE_CREATE) InstanceCreate(msg);
	}
}
