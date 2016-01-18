#include "Trajectory.h"

#include <cstdio>

/**
 * @author Clemens Korner
 */

using namespace std;

namespace tld
{

Trajectory::Trajectory() :
    m_length(0)
{
}

Trajectory::~Trajectory()
{
}

void Trajectory::init(std::size_t length)
{
	m_length = length;
	m_track_positions = vector<CvPoint>();
	m_track_colors = vector<CvScalar>();
}

void Trajectory::addPoint(CvPoint point, CvScalar color)
{
	size_t length = m_track_positions.size();

	// vectors aren't full
	if(length < m_length)
	{
		m_track_positions.push_back(point);
		m_track_colors.push_back(color);
	}
	else
	{
		// push element to the end
		m_track_positions.push_back(point);
		m_track_colors.push_back(color);

		// drop first element
		m_track_positions.erase(m_track_positions.begin());
		m_track_colors.erase(m_track_colors.begin());
	}
}

void Trajectory::drawTrajectory(IplImage * image)
{
	CvPoint tempPoint;
	bool needSecondPoint = false;

	for(size_t i = 0; i < m_track_positions.size(); i++)
	{
		// try to find 1. point of the line
		if((!needSecondPoint)&&(m_track_positions[i].x != -1)) {
			tempPoint = m_track_positions[i];
			needSecondPoint = true;
		// try to find 2. point of the line
		}
		else if(needSecondPoint&&(m_track_positions[i].x != -1))
		{
			cvLine(image, tempPoint, m_track_positions[i], m_track_colors[i], 2);
			tempPoint = m_track_positions[i];
		}
	}
}

}
