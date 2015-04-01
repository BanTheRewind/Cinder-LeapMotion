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

#pragma once

#include "cinder/Color.h"
#include "cinder/Vector.h"
#include <map>
#include <vector>

typedef std::map<int32_t, class Ribbon> RibbonMap;

class Ribbon
{
public:
	Ribbon( int32_t id = 0, const ci::Colorf& color = ci::Colorf::white() );
	~Ribbon();

	void					addPoint( const ci::vec3& position, float width = 1.0f );
	void					draw() const;
	void					update();
	
	const ci::Colorf&		getColor() const;
	int32_t					getId() const;
private:
	struct Point
	{
		Point( const ci::vec3& position = ci::vec3( 0.0f ), float width = 1.0f );
		float				mAlpha;
		ci::vec3			mPosition;
		float				mWidth;
	};

	ci::Colorf				mColor;
	int32_t					mId;
	std::vector<Point>		mPoints;
	std::vector<ci::vec3>	mPositions;
};
