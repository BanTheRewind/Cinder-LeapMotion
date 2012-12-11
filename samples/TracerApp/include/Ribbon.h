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

	void					addPoint( const ci::Vec3f& position, float width = 1.0f );
	void					draw() const;
	void					update();

	
	const ci::Colorf&		getColor() const;
	int32_t					getId() const;
private:
	struct Point
	{
		Point( const ci::Vec3f &position = ci::Vec3f::zero(), float width = 1.0f );
		float				mAlpha;
		ci::Vec3f			mPosition;
		float				mWidth;
	};

	ci::Colorf				mColor;
	int32_t					mId;
	std::vector<Point>		mPoints;
	std::vector<ci::Vec3f>	mPositions;
};
