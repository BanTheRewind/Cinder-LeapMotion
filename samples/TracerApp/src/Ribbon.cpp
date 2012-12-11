#include "Ribbon.h"

#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;

Ribbon::Point::Point( const Vec3f &position, float width )
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
	size_t count	= mPoints.size();
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
