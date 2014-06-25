/*
* 
* Copyright (c) 2014, Ban the Rewind
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

#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;

Ribbon::Point::Point( const Vec3f& position, float width )
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

void Ribbon::addPoint( const Vec3f& position, float width )
{
	Point point( position, width );
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

	glBegin( GL_TRIANGLES );
	for ( size_t i = 0; i < count - 2; ++i ) {
        uint32_t index = i * 2;

		Vec3f pos0 = mPositions.at( index + 0 );
		Vec3f pos1 = mPositions.at( index + 1 );
		Vec3f pos2 = mPositions.at( index + 2 );
		Vec3f pos3 = mPositions.at( index + 3 );

		ColorAf color( mColor, mPoints.at( i ).mAlpha );
		gl::color( color );

		gl::vertex( pos0 );
		gl::vertex( pos2 );
		gl::vertex( pos1 );
		gl::vertex( pos1 );
		gl::vertex( pos2 );
		gl::vertex( pos3 );
    }
	glEnd();
}

void Ribbon::update()
{
	for ( vector<Point>::iterator iter = mPoints.begin(); iter != mPoints.end(); ) {
		iter->mAlpha		-= 0.01f;
		iter->mWidth		-= 0.3f;
		iter->mPosition.y	+= 1.0f;
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

			Vec3f pos0		= a.mPosition;
			Vec3f pos1		= b.mPosition;
			Vec3f dir		= pos0 - pos1;
			dir.z			= 0.0f;
			Vec3f tan		= dir.cross( Vec3f::zAxis() );
			tan				= dir.cross( tan );
			tan				= dir.cross( tan ).normalized();
			Vec3f offset	= tan * a.mWidth;

			mPositions.push_back( pos0 - offset );
			mPositions.push_back( pos1 + offset );
		}
	}
}
