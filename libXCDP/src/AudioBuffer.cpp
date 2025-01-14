#include "AudioBuffer.h"

#include <iostream>
#include <algorithm>

#include <sndfile.h>

using namespace xcdp;

AudioBuffer::AudioBuffer()
	: format()
	, buffer()
	{}
AudioBuffer::AudioBuffer( const Format & other )
	: format( other )
	, buffer( getNumChannels() * getNumSamples() )
	{}
AudioBuffer::AudioBuffer( const std::string & filename )
	: format()
	, buffer()
	{
	load( filename );
	}

//======================================================
//	I/O
//======================================================
void AudioBuffer::load( const std::string & filePath ) 
	{
	//Open file and check validity, save the sample rate
	SF_INFO info;
	SNDFILE * file = sf_open( filePath.data(), SFM_READ, &info ); 
	if( file == nullptr )
		{
		std::cout << filePath << " could not be opened.\n";
		return;
		}

	//Copy file info into format
	format.sampleRate = info.samplerate;
	format.numChannels = info.channels;
	format.numSamples = info.frames;
	*this = AudioBuffer( format );

	//Create temporary buffer for interleaved data in file, read data in, close the file
	std::vector< double > interleavedBuffer( info.frames * info.channels );
	sf_readf_double( file, interleavedBuffer.data(), info.frames );
	if( sf_close( file ) != 0 )
		{
		std::cout << "Error closing " << filePath << ".\n";
		return;
		}

	//Convert interleaved data in
	for( size_t channel = 0; channel < info.channels; ++channel )
		for( size_t sample = 0; sample < size_t(info.frames); ++sample )
			setSample( channel, sample, interleavedBuffer[ sample * info.channels + channel ] );
	}

bool AudioBuffer::save( const std::string & filePath ) const 
	{
	//Check that nothing silly is going on with the file formatting
	SF_INFO info = {};
	info.channels	= int( getNumChannels() );
	info.frames		= int( getNumSamples()	);
	info.samplerate = int( getSampleRate() );
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;
	if( !sf_format_check( &info ) )
		{
		std::cout << "Sound file formatting invalid while attempting to save to " << filePath << ",\n";
		printSummary();
		return false;
		}

	//Create a temporary buffer for interleaved data and copy the buffer in
	std::vector< double > interleavedBuffer( getNumSamples() * getNumChannels() );
	for( size_t channel = 0; channel < getNumChannels(); ++channel )
		for( size_t frame = 0; frame < getNumSamples(); ++frame )
			interleavedBuffer[ frame * info.channels + channel ] = getSample( channel, frame );

	//Clip all samples in the interleaved buffer
	std::for_each( interleavedBuffer.begin(), interleavedBuffer.end(), []( double & s )
		{
		s = std::clamp( s, -1.0, 1.0 );
		});

	//Open the file and write in the interleaved buffer
	SF_INFO outInfo = info;
	SNDFILE * file = sf_open( filePath.data(), SFM_WRITE, &outInfo );
	if( file == nullptr )
		{
		std::cout << filePath << " could not be opened for saving.\n";
		return false;
		}
	if( sf_writef_double( file, interleavedBuffer.data(), info.frames ) != info.frames )	
		{
		std::cout << "Error writing data into " << filePath << ".\n";
		return false;
		}
	sf_close( file );
	
	return true;
	}

void AudioBuffer::printSummary() const
	{
	std::cout << "\n===================================================\n";
	std::cout << "Channels: " << getNumChannels() << " Frames: " << getNumSamples() << " Sample Rate: " << getSampleRate() << "\n";
	std::cout << "===================================================\n\n";
	}

//======================================================
//	Getters
//======================================================
double AudioBuffer::getSample( size_t channel, size_t frame ) const 
	{
	return buffer[getPos( channel, frame )];
	}

AudioBuffer::Format xcdp::AudioBuffer::getFormat() const
	{
	return format;
	}

size_t AudioBuffer::getNumChannels() const
	{
	return format.numChannels;
	}

size_t AudioBuffer::getNumSamples() const 
	{
	return format.numSamples;
	}

size_t AudioBuffer::getSampleRate() const 
	{
	return format.sampleRate;
	}

double AudioBuffer::sampleToTime( size_t sample ) const
	{
	return double( sample ) / double( getSampleRate() );
	}

size_t AudioBuffer::timeToSample( double time ) const
	{
	return size_t( time  * double( getSampleRate() ) );
	}

double AudioBuffer::getLength() const
	{
	return sampleToTime( getNumSamples() );
	}

double AudioBuffer::getMaxSampleMagnitude() const
	{
	return std::abs(*std::max_element( buffer.begin(), buffer.end(), []( double a, double b )
		{
		return std::abs( a ) < std::abs( b );
		}));
	}

//======================================================
//	Setters
//======================================================
void AudioBuffer::setSample( size_t channel, size_t sample, double newValue ) 
	{
	buffer[getPos( channel, sample )] = newValue;
	}

double & AudioBuffer::getSample( size_t channel, size_t sample )
	{
	return buffer[getPos( channel, sample )];
	}

void xcdp::AudioBuffer::clearBuffer()
	{
	std::fill( buffer.begin(), buffer.end(), 0 );
	}

//======================================================
//	Private
//======================================================

size_t xcdp::AudioBuffer::getPos( size_t channel, size_t sample ) const
	{
	return channel * getNumSamples() + sample;
	}
