
#include <iostream>
#include <fstream>
#include <cstring>
#include "AL/al.h"
#include "AL/alc.h"
#include "mpg123.h"
#include "sndfile.h"

bool isBigEndian()
{
    int a = 1;
    return !((char*)&a)[0];
}

int convertToInt(char* buffer, int len)
{
    int a = 0;
    if (!isBigEndian())
        for (int i = 0; i<len; i++)
            ((char*)&a)[i] = buffer[i];
    else
        for (int i = 0; i<len; i++)
            ((char*)&a)[3 - i] = buffer[i];
    return a;
}

char* loadWAV(const char* fn, int& chan, int& samplerate, int& bps, int& size)
{
    char buffer[4];
    std::ifstream in(fn, std::ios::binary);
    in.read(buffer, 4);
    if (strncmp(buffer, "RIFF", 4) != 0)
    {
        std::cout << "this is not a valid WAVE file" << std::endl;
        return NULL;
    }
    in.read(buffer, 4);
    in.read(buffer, 4);      //WAVE
    in.read(buffer, 4);      //fmt
    in.read(buffer, 4);      //16
    in.read(buffer, 2);      //1
    in.read(buffer, 2);
    chan = convertToInt(buffer, 2);
    in.read(buffer, 4);
    samplerate = convertToInt(buffer, 4);
    in.read(buffer, 4);
    in.read(buffer, 2);
    in.read(buffer, 2);
    bps = convertToInt(buffer, 2);
    in.read(buffer, 4);      //data
    in.read(buffer, 4);
    size = convertToInt(buffer, 4);
    char* data = new char[size];
    in.read(data, size);
    return data;
}

int playWav(std::string file){
    int channel, sampleRate, bps, size;
    char* data = loadWAV(file.c_str(), channel, sampleRate, bps, size);
    ALCdevice* device = alcOpenDevice(NULL);
    if (device == NULL)
    {
        std::cout << "cannot open sound card" << std::endl;
        return 0;
    }
    ALCcontext* context = alcCreateContext(device, NULL);
    if (context == NULL)
    {
        std::cout << "cannot open context" << std::endl;
        return 0;
    }
    alcMakeContextCurrent(context);

    unsigned int bufferid, format;
    alGenBuffers(1, &bufferid);
    if (channel == 1)
    {
        if (bps == 8)
        {
            format = AL_FORMAT_MONO8;
        }
        else {
            format = AL_FORMAT_MONO16;
        }
    }
    else {
        if (bps == 8)
        {
            format = AL_FORMAT_STEREO8;
        }
        else {
            format = AL_FORMAT_STEREO16;
        }
    }
    alBufferData(bufferid, format, data, size, sampleRate);
    unsigned int sourceid;
    alGenSources(1, &sourceid);
    alSourcei(sourceid, AL_BUFFER, bufferid);
    alSourcePlay(sourceid);

    ALint state = AL_PLAYING;

    while(state == AL_PLAYING){
        alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
    }


    alDeleteSources(1, &sourceid);
    alDeleteBuffers(1, &bufferid);

    alcDestroyContext(context);
    alcCloseDevice(device);
    delete[] data;
    return 0;
}

int MP3toWAV(std::string file, std::string outputFile){
    SNDFILE* sndfile = NULL;
	SF_INFO sfinfo;
	mpg123_handle *mh = NULL;
	unsigned char* buffer = NULL;
	size_t buffer_size = 0;
	size_t done = 0;
	int  channels = 0, encoding = 0;
	long rate = 0;
	int  err  = MPG123_OK;
	off_t samples = 0;


	err = mpg123_init();
	if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
	{
		fprintf(stderr, "Basic setup goes wrong: %s", mpg123_plain_strerror(err));
		mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
		return -1;
	}

	/* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
	if(    mpg123_open(mh, file.c_str()) != MPG123_OK
	    /* Peek into track and get first output format. */
	    || mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
	{
		fprintf( stderr, "Trouble with mpg123: %s\n", mpg123_strerror(mh) );
		mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
		return -1;
	}

	if(encoding != MPG123_ENC_SIGNED_16 && encoding != MPG123_ENC_FLOAT_32)
	{ /* Signed 16 is the default output format anyways; it would actually by only different if we forced it.
	     So this check is here just for this explanation. */
		mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
		fprintf(stderr, "Bad encoding: 0x%x!\n", encoding);
		return -2;
	}
	/* Ensure that this output format will not change (it could, when we allow it). */
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);

	bzero(&sfinfo, sizeof(sfinfo) );
	sfinfo.samplerate = rate;
	sfinfo.channels = channels;
	sfinfo.format = SF_FORMAT_WAV|(encoding == MPG123_ENC_SIGNED_16 ? SF_FORMAT_PCM_16 : SF_FORMAT_FLOAT);
	printf("Creating WAV with %i channels and %liHz.\n", channels, rate);

	sndfile = sf_open(outputFile.c_str(), SFM_WRITE, &sfinfo);
	if(sndfile == NULL){ fprintf(stderr, "Cannot open output file!\n"); mpg123_close(mh); mpg123_delete(mh); mpg123_exit(); return -2; }

	/* Buffer could be almost any size here, mpg123_outblock() is just some recommendation.
	   Important, especially for sndfile writing, is that the size is a multiple of sample size. */
	buffer_size = mpg123_outblock(mh);
	buffer = (unsigned char*)malloc( buffer_size );

	do
	{
		sf_count_t more_samples;
		err = mpg123_read( mh, buffer, buffer_size, &done );
		more_samples = encoding == MPG123_ENC_SIGNED_16
			? sf_write_short(sndfile, (short*)buffer, done/sizeof(short))
			: sf_write_float(sndfile, (float*)buffer, done/sizeof(float));
		if(more_samples < 0 || more_samples*mpg123_encsize(encoding) != done)
		{
			fprintf(stderr, "Warning: Written number of samples does not match the byte count we got from libmpg123: %li != %li\n", (long)(more_samples*mpg123_encsize(encoding)), (long)done);
		}
		samples += more_samples;
		/* We are not in feeder mode, so MPG123_OK, MPG123_ERR and MPG123_NEW_FORMAT are the only possibilities.
		   We do not handle a new format, MPG123_DONE is the end... so abort on anything not MPG123_OK. */
	} while (err==MPG123_OK);

	if(err != MPG123_DONE) fprintf( stderr, "Warning: Decoding ended prematurely because: %s\n",
	         err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) );

	sf_close( sndfile );

	samples /= channels;
	mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
	return 0;
}

void play(std::string file){
	std::string ext = file; 
    ext.replace(ext.begin(),ext.begin()+file.find_last_of('.'),"");
    if (ext == ".mp3"){ 
        std::string wav = file+".wav";
        MP3toWAV(file,wav); file=wav;
    }

    playWav(file);

    if (ext == ".mp3"){
        std::string cmd = "rm "+file;
        system(cmd.c_str());
    }
}