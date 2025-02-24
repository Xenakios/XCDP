#pragma once

#include <functional>

#include "Utility.h"

namespace xcdp {

/*
 * Utility function abstraction for allowing doubles to be implicitly callable
 * also some other stuff
 */
struct RealFunc 
{
	template< typename Callable >
	RealFunc( Callable f_ )
		: f( f_ )
		{}
	RealFunc( double t0 ) 
		: f( [t0]( double t ){ return t0; } ) 
		{}
	RealFunc( int t0 ) 
		: f( [t0]( double t ){ return double(t0); } ) 
		{}
	double operator()( double t ) const { return f(t); }
	double operator[]( double t ) const { return pow( 2, f(t) ); }
	RealFunc logify() const { return [this]( double t ){ return operator[](t); }; }

	RealFunc operator*( const RealFunc r ) const { return [this, r](double t){ return operator()(t) * r(t); }; }
	RealFunc operator/( const RealFunc r ) const { return [this, r](double t){ return operator()(t) / r(t); }; }
	RealFunc operator+( const RealFunc r ) const { return [this, r](double t){ return operator()(t) + r(t); }; }
	RealFunc operator-( const RealFunc r ) const { return [this, r](double t){ return operator()(t) - r(t); }; }
	RealFunc operator-() const { return [this](double t){ return -operator()(t); }; }
	
	void graph( const std::string & filename,
		double left = -1, double right = 10, double bottom = -1, double top = 3.0, size_t resolution = 128 ) const;

	static RealFunc ADSR( double a, double d, double s, double r, double sLvl,
		double aExp = 1, double dExp = 1, double rExp = 1 );

	static RealFunc oscillate( RealFunc min, RealFunc max, RealFunc period = 2.0 * 3.14159265359, 
		RealFunc wave = []( double t ){ return sin(t); } );

	static RealFunc interpolatePoints( const std::vector< std::pair< double, double > > points, 
		Interpolator = Interpolators::linear );

private:
	std::function< double ( double ) > f;
};

struct Surface
{
	template< typename Callable >
	Surface( Callable f_ )
		: f( f_ )
		{}
	Surface( RealFunc f_ )
		: f( [f_]( double t, double ){ return f_( t ); } )
		{}
	Surface( double t0 ) 
		: f( [t0]( double, double ){ return t0; } ) 
		{}
	Surface( int t0 ) 
		: f( [t0]( double, double ){ return double( t0 ); } ) 
		{}
	double operator()( double time, double freq ) const { return f( time, freq ); }

private:
	std::function< double ( double, double ) > f;
};

} // End namespace xcdp