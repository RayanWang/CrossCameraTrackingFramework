#include <limits.h>
#include <stdio.h>
#include <functional>
#include <algorithm>
#include "BlobResult.h"

//! Show errors functions: only works for windows releases
#ifdef _SHOW_ERRORS
	#include <afx.h>			//suport per a CStrings
	#include <afxwin.h>			//suport per a AfxMessageBox
#endif

/**************************************************************************
		Constructors / Destructors
**************************************************************************/
CBlobResult::CBlobResult()
{
	m_blobs = Blob_vector();
}

CBlobResult::CBlobResult(IplImage *source, IplImage *mask, uchar backgroundColor )
{
	bool success;

	try
	{
		success = ComponentLabeling( source, mask, backgroundColor, m_blobs );
	}
	catch(...)
	{
		success = false;
	}

	if( !success ) throw EXCEPCIO_CALCUL_BLOBS;
}

CBlobResult::CBlobResult( const CBlobResult &source )
{
	m_blobs = Blob_vector( source.GetNumBlobs() );
	
	m_blobs = Blob_vector( source.GetNumBlobs() );
	Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
	Blob_vector::iterator pBlobsDst = m_blobs.begin();

	while( pBlobsSrc != source.m_blobs.end() )
	{
		// blob original
		*pBlobsDst = new CBlob(**pBlobsSrc);
		pBlobsSrc++;
		pBlobsDst++;
	}
}

CBlobResult::~CBlobResult()
{
	ClearBlobs();
}

/**************************************************************************
		Operadors / Operators
**************************************************************************/
CBlobResult& CBlobResult::operator=(const CBlobResult& source)
{
	if (this != &source)
	{
		for( int i = 0; i < GetNumBlobs(); i++ )
		{
			delete m_blobs[i];
		}
		m_blobs.clear();
		m_blobs = Blob_vector( source.GetNumBlobs() );
		Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
		Blob_vector::iterator pBlobsDst = m_blobs.begin();

		while( pBlobsSrc != source.m_blobs.end() )
		{
			// blob original
			*pBlobsDst = new CBlob(**pBlobsSrc);
			pBlobsSrc++;
			pBlobsDst++;
		}
	}
	return *this;
}

CBlobResult CBlobResult::operator+( const CBlobResult& source ) const
{	
	CBlobResult resultat( *this );
	
	resultat.m_blobs.resize( resultat.GetNumBlobs() + source.GetNumBlobs() );

	Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
	Blob_vector::iterator pBlobsDst = resultat.m_blobs.end();

	while( pBlobsSrc != source.m_blobs.end() )
	{
		pBlobsDst--;
		*pBlobsDst = new CBlob(**pBlobsSrc);
		pBlobsSrc++;
	}
	
	return resultat;
}

/**************************************************************************
		Operacions / Operations
**************************************************************************/
void CBlobResult::AddBlob( CBlob *blob )
{
	if( blob != NULL )
		m_blobs.push_back( new CBlob( blob ) );
}


#ifdef MATRIXCV_ACTIU

/**
- FUNCI�: GetResult
- FUNCIONALITAT: Calcula el resultat especificat sobre tots els blobs de la classe
- PAR�METRES:
	- evaluador: Qualsevol objecte derivat de COperadorBlob
- RESULTAT:
	- Retorna un array de double's amb el resultat per cada blob
- RESTRICCIONS:
- AUTOR: Ricard Borr�s
- DATA DE CREACI�: 25-05-2005.
- MODIFICACI�: Data. Autor. Descripci�.
*/
/**
- FUNCTION: GetResult
- FUNCTIONALITY: Computes the function evaluador on all the blobs of the class
				 and returns a vector with the result
- PARAMETERS:
	- evaluador: function to apply to each blob (any object derived from the 
				 COperadorBlob class )
- RESULT:
	- vector with all the results in the same order as the blobs
- RESTRICTIONS:
- AUTHOR: Ricard Borr�s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
double_vector CBlobResult::GetResult( funcio_calculBlob *evaluador ) const
{
	if( GetNumBlobs() <= 0 )
	{
		return double_vector();
	}

	// definim el resultat
	double_vector result = double_vector( GetNumBlobs() );
	// i iteradors sobre els blobs i el resultat
	double_vector::iterator itResult = result.GetIterator();
	Blob_vector::const_iterator itBlobs = m_blobs.begin();

	// avaluem la funci� en tots els blobs
	while( itBlobs != m_blobs.end() )
	{
		*itResult = (*evaluador)(**itBlobs);
		itBlobs++;
		itResult++;
	}
	return result;
}
#endif

double_stl_vector CBlobResult::GetSTLResult( funcio_calculBlob *evaluador ) const
{
	if( GetNumBlobs() <= 0 )
	{
		return double_stl_vector();
	}

	double_stl_vector result = double_stl_vector( GetNumBlobs() );
	double_stl_vector::iterator itResult = result.begin();
	Blob_vector::const_iterator itBlobs = m_blobs.begin();

	while( itBlobs != m_blobs.end() )
	{
		*itResult = (*evaluador)(**itBlobs);
		itBlobs++;
		itResult++;
	}
	return result;
}

double CBlobResult::GetNumber( int indexBlob, funcio_calculBlob *evaluador ) const
{
	if( indexBlob < 0 || indexBlob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );
	return (*evaluador)( *m_blobs[indexBlob] );
}

/////////////////////////// FILTRAT DE BLOBS ////////////////////////////////////
void CBlobResult::Filter(CBlobResult &dst, 
						 int filterAction, 
						 funcio_calculBlob *evaluador, 
						 int condition, 
						 double lowLimit, double highLimit /*=0*/) const
							
{
	// do the job
	DoFilter(dst, filterAction, evaluador, condition, lowLimit, highLimit );
}

void CBlobResult::Filter(CBlobResult &dst, 
						 int filterAction, 
						 funcio_calculBlob *evaluador, 
						 int condition, 
						 double lowLimit, double highLimit /*=0*/)
							
{
	int numBlobs = GetNumBlobs();

	// do the job
	DoFilter(dst, filterAction, evaluador, condition, lowLimit, highLimit );

	// inline operation: remove previous blobs
	if( &dst == this ) 
	{
		Blob_vector::iterator itBlobs = m_blobs.begin();
		for( int i = 0; i < numBlobs; i++ )
		{
			delete *itBlobs;
			itBlobs++;
		}
		m_blobs.erase( m_blobs.begin(), itBlobs );
	}
}


//! Does the Filter method job
void CBlobResult::DoFilter(CBlobResult &dst, int filterAction, funcio_calculBlob *evaluador, 
						   int condition, double lowLimit, double highLimit/* = 0*/) const
{
	int i, numBlobs;
	bool resultavaluacio;
	double_stl_vector avaluacioBlobs;
	double_stl_vector::iterator itavaluacioBlobs;

	if( GetNumBlobs() <= 0 ) return;
	if( !evaluador ) return;
	avaluacioBlobs = GetSTLResult(evaluador);
	itavaluacioBlobs = avaluacioBlobs.begin();
	numBlobs = GetNumBlobs();
	switch(condition)
	{
		case B_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs == lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}				
			}
			break;
		case B_NOT_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio = *itavaluacioBlobs != lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_GREATER:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs > lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_LESS:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs < lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_GREATER_OR_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs>= lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_LESS_OR_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs <= lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_INSIDE:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio=( *itavaluacioBlobs >= lowLimit) && ( *itavaluacioBlobs <= highLimit); 
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_OUTSIDE:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio=( *itavaluacioBlobs < lowLimit) || ( *itavaluacioBlobs > highLimit); 
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
	}
}

CBlob CBlobResult::GetBlob(int indexblob) const
{	
	if( indexblob < 0 || indexblob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );

	return *m_blobs[indexblob];
}
CBlob *CBlobResult::GetBlob(int indexblob)
{	
	if( indexblob < 0 || indexblob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );

	return m_blobs[indexblob];
}

void CBlobResult::GetNthBlob( funcio_calculBlob *criteri, int nBlob, CBlob &dst ) const
{
	if( nBlob < 0 || nBlob >= GetNumBlobs() )
	{
		//RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );
		dst = CBlob();
		return;
	}

	double_stl_vector avaluacioBlobs, avaluacioBlobsOrdenat;
	double valorEnessim;

	avaluacioBlobs = GetSTLResult(criteri);

	avaluacioBlobsOrdenat = double_stl_vector( GetNumBlobs() );

	std::partial_sort_copy( avaluacioBlobs.begin(), 
						    avaluacioBlobs.end(),
						    avaluacioBlobsOrdenat.begin(), 
						    avaluacioBlobsOrdenat.end(),
						    std::greater<double>() );

	valorEnessim = avaluacioBlobsOrdenat[nBlob];

	double_stl_vector::const_iterator itAvaluacio = avaluacioBlobs.begin();

	bool trobatBlob = false;
	int indexBlob = 0;
	while( itAvaluacio != avaluacioBlobs.end() && !trobatBlob )
	{
		if( *itAvaluacio == valorEnessim )
		{
			trobatBlob = true;
			dst = CBlob( GetBlob(indexBlob));
		}
		itAvaluacio++;
		indexBlob++;
	}
}

void CBlobResult::ClearBlobs()
{
	Blob_vector::iterator itBlobs = m_blobs.begin();
	while( itBlobs != m_blobs.end() )
	{
		delete *itBlobs;
		itBlobs++;
	}

	m_blobs.clear();
}

void CBlobResult::RaiseError(const int errorCode) const
{
//! Do we need to show errors?
#ifdef _SHOW_ERRORS
	CString msg, format = "Error en CBlobResult: %s";

	switch (errorCode)
	{
	case EXCEPTION_BLOB_OUT_OF_BOUNDS:
		msg.Format(format, "Intentant accedir a un blob no existent");
		break;
	default:
		msg.Format(format, "Codi d'error desconegut");
		break;
	}

	AfxMessageBox(msg);

#endif
	throw errorCode;
}



/**************************************************************************
		Auxiliars / Auxiliary functions
**************************************************************************/
void CBlobResult::PrintBlobs( char *nom_fitxer ) const
{
	double_stl_vector area, /*perimetre,*/ exterior, compacitat, longitud, 
					  externPerimeter, perimetreConvex, perimetre;
	int i;
	FILE *fitxer_sortida;

 	area      = GetSTLResult( CBlobGetArea());
	perimetre = GetSTLResult( CBlobGetPerimeter());
	exterior  = GetSTLResult( CBlobGetExterior());
	compacitat = GetSTLResult(CBlobGetCompactness());
	longitud  = GetSTLResult( CBlobGetLength());
	externPerimeter = GetSTLResult( CBlobGetExternPerimeter());
	perimetreConvex = GetSTLResult( CBlobGetHullPerimeter());

	fitxer_sortida = fopen( nom_fitxer, "w" );

	for(i=0; i<GetNumBlobs(); i++)
	{
		fprintf( fitxer_sortida, "blob %d ->\t a=%7.0f\t p=%8.2f (%8.2f extern)\t pconvex=%8.2f\t ext=%.0f\t m=%7.2f\t c=%3.2f\t l=%8.2f\n",
				 i, area[i], perimetre[i], externPerimeter[i], perimetreConvex[i], exterior[i], compacitat[i], longitud[i] );
	}
	fclose( fitxer_sortida );

}
