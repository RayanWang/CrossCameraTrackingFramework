#ifndef BLOB_OPERATORS_H_INCLUDED
#define BLOB_OPERATORS_H_INCLUDED

#include "blob.h"

#define DEGREE2RAD		(CV_PI / 180.0)

//! Interface to derive all blob operations
class COperadorBlob
{
public:
	virtual ~COperadorBlob(){};

	//! Aply operator to blob
	virtual double operator()(CBlob &blob) = 0;
	//! Get operator name
	virtual const char *GetNom() = 0;

	operator COperadorBlob*()
	{
		return (COperadorBlob*)this;
	}
};

typedef COperadorBlob funcio_calculBlob;

#ifdef BLOB_OBJECT_FACTORY
	struct functorComparacioIdOperador
	{
	  bool operator()(const char* s1, const char* s2) const
	  {
		return strcmp(s1, s2) < 0;
	  }
	};

	//! Definition of Object factory type for COperadorBlob objects
	typedef ObjectFactory<COperadorBlob, const char *, functorComparacioIdOperador > t_OperadorBlobFactory;

	void RegistraTotsOperadors( t_OperadorBlobFactory &fabricaOperadorsBlob );

#endif


//! Class to get ID of a blob
class CBlobGetID : public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.GetID(); 
	}
	const char *GetNom()
	{
		return "CBlobGetID";
	}
};


//! Class to get the area of a blob
class CBlobGetArea : public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.Area(); 
	}
	const char *GetNom()
	{
		return "CBlobGetArea";
	}
};

//! Class to get the perimeter of a blob
class CBlobGetPerimeter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{ 
		return blob.Perimeter(); 
	}
	const char *GetNom()
	{
		return "CBlobGetPerimeter";
	}
};

//! Class to get the extern flag of a blob
class CBlobGetExterior: public COperadorBlob
{
public:
	CBlobGetExterior()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExterior(IplImage *mask, bool xBorder = true, bool yBorder = true)
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{ 
		return blob.Exterior(m_mask, m_xBorder, m_yBorder); 
	}
	const char *GetNom()
	{
		return "CBlobGetExterior";
	}
private:
	IplImage *m_mask;
	bool m_xBorder, m_yBorder;
};

//! Class to get the mean grey level of a blob
class CBlobGetMean: public COperadorBlob
{
public:
	CBlobGetMean()
	{
		m_image = NULL;
	}
	CBlobGetMean( IplImage *image )
	{
		m_image = image;
	};

    double operator()(CBlob &blob)
	{ 
		return blob.Mean(m_image); 
	}
	const char *GetNom()
	{
		return "CBlobGetMean";
	}
private:

	IplImage *m_image;
};

//! Class to get the standard deviation of the grey level values of a blob
class CBlobGetStdDev: public COperadorBlob
{
public:
	CBlobGetStdDev()
	{
		m_image = NULL;
	}
	CBlobGetStdDev( IplImage *image )
	{
		m_image = image;
	};
    double operator()(CBlob &blob)
	{ 
		return blob.StdDev(m_image); 
	}
	const char *GetNom()
	{
		return "CBlobGetStdDev";
	}
private:

	IplImage *m_image;

};

//! Class to calculate the compactness of a blob
class CBlobGetCompactness: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetCompactness";
	}
};

//! Class to calculate the length of a blob
class CBlobGetLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetLength";
	}
};

//! Class to calculate the breadth of a blob
class CBlobGetBreadth: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetBreadth";
	}
};

class CBlobGetDiffX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.GetBoundingBox().width;
	}
	const char *GetNom()
	{
		return "CBlobGetDiffX";
	}
};

class CBlobGetDiffY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.GetBoundingBox().height;
	}
	const char *GetNom()
	{
		return "CBlobGetDiffY";
	}
};

//! Class to calculate the P,Q moment of a blob
class CBlobGetMoment: public COperadorBlob
{
public:
	//! Standard constructor (gets the 00 moment)
	CBlobGetMoment()
	{
		m_p = m_q = 0;
	}
	//! Constructor: gets the PQ moment
	CBlobGetMoment( int p, int q )
	{
		m_p = p;
		m_q = q;
	};
	double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMoment";
	}

private:
	int m_p, m_q;
};

//! Class to calculate the convex hull perimeter of a blob
class CBlobGetHullPerimeter: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetHullPerimeter";
	}
};

//! Class to calculate the convex hull area of a blob
class CBlobGetHullArea: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetHullArea";
	}
};

//! Class to calculate the minimum x on the minimum y
class CBlobGetMinXatMinY: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMinXatMinY";
	}
};

//! Class to calculate the minimum y on the maximum x
class CBlobGetMinYatMaxX: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMinYatMaxX";
	}
};

//! Class to calculate the maximum x on the maximum y
class CBlobGetMaxXatMaxY: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMaxXatMaxY";
	}
};

//! Class to calculate the maximum y on the minimum y
class CBlobGetMaxYatMinX: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetMaxYatMinX";
	}
};

//! Class to get the minimum x
class CBlobGetMinX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinX();
	}
	const char *GetNom()
	{
		return "CBlobGetMinX";
	}
};

//! Class to get the maximum x
class CBlobGetMaxX: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MaxX();
	}
	const char *GetNom()
	{
		return "CBlobGetMaxX";
	}
};

//! Class to get the minimum y
class CBlobGetMinY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinY();
	}
	const char *GetNom()
	{
		return "CBlobGetMinY";
	}
};

//! Class to get the maximum y
class CBlobGetMaxY: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MaxY();
	}
	const char *GetNom()
	{
		return "CBlobGetMaxY";
	}
};

//! Class to calculate the elongation of the blob
class CBlobGetElongation: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetElongation";
	}
};

//! Class to calculate the roughness of the blob
class CBlobGetRoughness: public COperadorBlob
{
public:
    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetRoughness";
	}
};

//! Class to calculate the euclidean distance between the center of a blob and a given point
class CBlobGetDistanceFromPoint: public COperadorBlob
{
public:
	//! Standard constructor (distance to point 0,0)
	CBlobGetDistanceFromPoint()
	{
		m_x = m_y = 0.0;
	}
	//! Constructor (distance to point x,y)
	CBlobGetDistanceFromPoint( const double x, const double y )
	{
		m_x = x;
		m_y = y;
	}

    double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetDistanceFromPoint";
	}

private:
	double m_x, m_y;
};

//! Class to get the number of extern pixels of a blob
class CBlobGetExternPerimeter: public COperadorBlob
{
public:
	CBlobGetExternPerimeter()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternPerimeter( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternPerimeter";
	}
private:
	IplImage *m_mask;
	bool m_xBorder, m_yBorder;
};

//! Class to calculate the ratio between the perimeter and the number of extern pixels
class CBlobGetExternPerimeterRatio: public COperadorBlob
{
public:
	CBlobGetExternPerimeterRatio()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternPerimeterRatio( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		if( blob.Perimeter() != 0 )
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder) / blob.Perimeter();
		else
			return blob.ExternPerimeter(m_mask,  m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternPerimeterRatio";
	}
private:
	IplImage *m_mask;
	bool  m_xBorder, m_yBorder;
};

//! Class to calculate the ratio between the perimeter and the number of extern pixels
class CBlobGetExternHullPerimeterRatio: public COperadorBlob
{
public:
	CBlobGetExternHullPerimeterRatio()
	{
		m_mask = NULL;
		m_xBorder = false;
		m_yBorder = false;
	}
	CBlobGetExternHullPerimeterRatio( IplImage *mask, bool xBorder = true, bool yBorder = true )
	{
		m_mask = mask;
		m_xBorder = xBorder;
		m_yBorder = yBorder;
	}
    double operator()(CBlob &blob)
	{
		CBlobGetHullPerimeter getHullPerimeter;
		double hullPerimeter;

		if( (hullPerimeter = getHullPerimeter( blob ) ) != 0 )
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder) / hullPerimeter;
		else
			return blob.ExternPerimeter(m_mask, m_xBorder, m_yBorder);
	}
	const char *GetNom()
	{
		return "CBlobGetExternHullPerimeterRatio";
	}
private:
	IplImage *m_mask;
	bool  m_xBorder, m_yBorder;

};

//! Class to calculate the center in the X direction
class CBlobGetXCenter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinX() + (( blob.MaxX() - blob.MinX() ) / 2.0);
	}
	const char *GetNom()
	{
		return "CBlobGetXCenter";
	}
};

//! Class to calculate the center in the Y direction
class CBlobGetYCenter: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		return blob.MinY() + (( blob.MaxY() - blob.MinY() ) / 2.0);
	}
	const char *GetNom()
	{
		return "CBlobGetYCenter";
	}
};

//! Class to calculate the length of the major axis of the ellipse that fits the blob edges
class CBlobGetMajorAxisLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();

		return elipse.size.width;
	}
	const char *GetNom()
	{
		return "CBlobGetMajorAxisLength";
	}
};

class CBlobGetAreaElipseRatio: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		if( blob.Area()==0.0 ) return 0.0;

		CvBox2D elipse = blob.GetEllipse();
		double ratioAreaElipseAreaTaca = ( (elipse.size.width/2.0)
										   *
										   (elipse.size.height/2.0)
							               *CV_PI
						                 )
									     /
									     blob.Area();

		return ratioAreaElipseAreaTaca;
	}
	const char *GetNom()
	{
		return "CBlobGetAreaElipseRatio";
	}
};

//! Class to calculate the length of the minor axis of the ellipse that fits the blob edges
class CBlobGetMinorAxisLength: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();

		return elipse.size.height;
	}
	const char *GetNom()
	{
		return "CBlobGetMinorAxisLength";
	}
};

//! Class to calculate the orientation of the ellipse that fits the blob edges in radians
class CBlobGetOrientation: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CvBox2D elipse = blob.GetEllipse();
/*
		if( elipse.angle > 180.0 )
			return (( elipse.angle - 180.0 )* DEGREE2RAD);
		else
			return ( elipse.angle * DEGREE2RAD);
*/
		return elipse.angle;
	}
	const char *GetNom()
	{
		return "CBlobGetOrientation";
	}
};

//! Class to calculate the cosinus of the orientation of the ellipse that fits the blob edges
class CBlobGetOrientationCos: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		CBlobGetOrientation getOrientation;
		return fabs( cos( getOrientation(blob)*DEGREE2RAD ));
	}
	const char *GetNom()
	{
		return "CBlobGetOrientationCos";
	}
};

//! Class to calculate the ratio between both axes of the ellipse
class CBlobGetAxisRatio: public COperadorBlob
{
public:
    double operator()(CBlob &blob)
	{
		double major,minor;
		CBlobGetMajorAxisLength getMajor;
		CBlobGetMinorAxisLength getMinor;

		major = getMajor(blob);
		minor = getMinor(blob);

		if( major != 0 )
			return minor / major;
		else
			return 0;
	}
	const char *GetNom()
	{
		return "CBlobGetAxisRatio";
	}
};

//! Class to calculate whether a point is inside a blob
class CBlobGetXYInside: public COperadorBlob
{
public:
	//! Standard constructor
	CBlobGetXYInside()
	{
		m_p.x = 0;
		m_p.y = 0;
	}
	//! Constructor: sets the point
	CBlobGetXYInside( CvPoint2D32f p )
	{
		m_p = p;
	};
	double operator()(CBlob &blob);
	const char *GetNom()
	{
		return "CBlobGetXYInside";
	}

private:
	//! point to be considered
	CvPoint2D32f m_p;
};

#endif	//!BLOB_OPERATORS_H_INCLUDED
