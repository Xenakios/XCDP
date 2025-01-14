#pragma once

#include "Audio.h"
#include "RealFunc.h"

namespace xcdp {

namespace Synthesis {

//wave is only evaluated from 0 to 2pi, which is its expected period

Audio waveform( RealFunc wave, double length, RealFunc freq, size_t samplerate = 44100, size_t oversample = 16 );

Audio sine( double length, RealFunc freq );
Audio square( double length, RealFunc freq );
Audio saw( double length, RealFunc freq );
Audio triangle( double length, RealFunc freq );

}
}