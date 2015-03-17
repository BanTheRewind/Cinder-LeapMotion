/*
* 
* Copyright (c) 2015, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include "Ribbon.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Ribbon::Point::Point( const vec3& position, float width )
{
	mAlpha		= 1.0f;
	mPosition	= position;
	mWidth		= width;
}

Ribbon::Ribbon( int32_t id, const Colorf& color )
{
	mColor	= color;
	mId		= id;
}

Ribbon::~Ribbon()
{
	mPoints.clear();
}

void Ribbon::addPoint( const vec3& position, float width )
{
	vec3 p( position );
	if ( !mPoints.empty() ) {
		p = glm::mix( mPoints.back().mPosition, position, vec3( 0.15f ) );
	}

	Point point( p, width );
	mPoints.push_back( point );
}

const Colorf& Ribbon::getColor() const
{
	return mColor;
}

int32_t Ribbon::getId() const
{
	return mId;
}

void Ribbon::draw() const
{
	size_t count = mPoints.size();
	if ( count < 2 ) {
		return;
	}

	gl::begin( GL_TRIANGLES );
	for ( size_t i = 0; i < count - 2; ++i ) {
        uint32_t index = i * 2;

		vec3 pos0 = mPositions.at( index + 0 );
		vec3 pos1 = mPositions.at( index + 1 );
		vec3 pos2 = mPositions.at( index + 2 );
		vec3 pos3 = mPositions.at( index + 3 );

		ColorAf color( mColor, mPoints.at( i ).mAlpha );
		gl::color( color );
		
		gl::vertex( pos0 );
		gl::vertex( pos2 );
		gl::vertex( pos1 );
		gl::vertex( pos1 );
		gl::vertex( pos2 );
		gl::vertex( pos3 );
    }
	gl::end();
}

void Ribbon::update()
{
	float e = getElapsedSeconds() * 40.0f;
	float i = 0.0f;
	for ( vector<Point>::iterator iter = mPoints.begin(); iter != mPoints.end(); i += 1.0f ) {
		iter->mAlpha		-= 0.01f;
		iter->mWidth		-= 0.125f;
		float t				= powf( i, 2.0f ) + e;
		iter->mPosition.x	+= cosf( t ) * 0.3f;
		iter->mPosition.y	+= sinf( t ) * 0.5f - 2.5f;

		if ( iter->mAlpha <= 0.0f || iter->mWidth <= 0.0f ) {
			iter = mPoints.erase( iter );
		} else {
			++iter;
		}
	}

	mPositions.clear();
	size_t count = mPoints.size();
	if ( count >= 2 ) {
		for ( size_t i = 0; i < count - 1; ++i ) {
			const Point& a	= mPoints.at( i );
			const Point& b	= mPoints.at( i + 1 );

			vec3 pos0	= a.mPosition;
			vec3 pos1	= b.mPosition;
			vec3 dir0	= pos0 - pos1;
			dir0.z		= 0.0f;
			vec3 dir1	= glm::cross( dir0, vec3( 0.0f, 0.0f, 1.0f ) );
			vec3 dir2	= glm::cross( dir0, dir1 );
			dir1		= glm::normalize( glm::cross( dir0, dir2 ) );
			vec3 offset	= dir1 * a.mWidth;

			mPositions.push_back( pos0 - offset );
			mPositions.push_back( pos0 + offset );
		}
	}
}
