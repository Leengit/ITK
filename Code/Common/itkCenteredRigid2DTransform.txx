/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkCenteredRigid2DTransform.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkCenteredRigid2DTransform_txx
#define _itkCenteredRigid2DTransform_txx

#include "itkCenteredRigid2DTransform.h"


namespace itk
{

// Constructor with default arguments
template <class TScalarType>
CenteredRigid2DTransform<TScalarType>
::CenteredRigid2DTransform():Superclass(OutputSpaceDimension, ParametersDimension)
{
  m_Angle = NumericTraits< TScalarType >::Zero;
  m_Center.Fill( 0.0 );
  m_Translation.Fill( 0.0 );

  // note: this virtual function will only
  // call the one defined in this class because 
  // we are in a constructor
  this->ComputeMatrixAndOffset();
}




// Copy Constructor
template <class TScalarType>
CenteredRigid2DTransform<TScalarType>
::CenteredRigid2DTransform( const Self & other )
{
  m_Angle         = other.m_Angle;
  m_Center        = other.m_Center;
  m_Translation   = other.m_Translation;
  this->m_Parameters    = other.m_Parameter;

  // note: this virtual function will only
  // call the one defined in this class because 
  // we are in a constructor
  this->ComputeMatrixAndOffset(); 
}



// Constructor with arguments
template<class TScalarType>
CenteredRigid2DTransform<TScalarType>::
CenteredRigid2DTransform( unsigned int spaceDimension, 
                          unsigned int parametersDimension):
  Superclass(spaceDimension,parametersDimension)
{

  m_Angle = NumericTraits< TScalarType >::Zero;
  m_Center.Fill( 0.0 );
  m_Translation.Fill( 0.0 );

  // note: this virtual function will only
  // call the one defined in this class because 
  // we are in a constructor
  this->ComputeMatrixAndOffset(); 
}
 




//
// Set Parameters
// 
// Parameters are ordered as:
//
// p[0]   = angle
// p[1:2} = center of rotation coordinates
// p[3:4} = translation components
//
//
template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetParameters( const ParametersType & parameters )
{
  itkDebugMacro( << "Setting paramaters " << parameters );

  this->m_Parameters = parameters;

  // Take the angle
  m_Angle = parameters[0];
 
  // Transfer the center 
  for(unsigned int i=0; i < SpaceDimension; i++) 
    {
    m_Center[i] = parameters[i+1];
    }

  // Transfer the translation
  for(unsigned int j=0; j < SpaceDimension; j++) 
    {
    m_Translation[j] = parameters[j+1+SpaceDimension];
    }

  this->ComputeMatrixAndOffset();

  itkDebugMacro(<<"After setting parameters ");
}

//
// Get Parameters
// 
// Parameters are ordered as:
//
// p[0]   = angle
// p[1:2} = center of rotation coordinates
// p[3:4} = translation components
//

template <class TScalarType>
const typename CenteredRigid2DTransform<TScalarType>::ParametersType &
CenteredRigid2DTransform<TScalarType>
::GetParameters( void ) const
{
  itkDebugMacro( << "Getting parameters ");

  // Set angles with parameters
  this->m_Parameters[0] = m_Angle;
 
  // Transfer the center of rotation 
  for(unsigned int i=0; i < SpaceDimension; i++) 
    {
    this->m_Parameters[i+1] = m_Center[i];
    }

  // Transfer the translation
  for(unsigned int j=0; j < SpaceDimension; j++) 
    {
    this->m_Parameters[j+1+SpaceDimension] = this->m_Translation[j];
    }

  itkDebugMacro(<<"After getting parameters " << this->m_Parameters );

  return this->m_Parameters;
}





template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetCenter( const InputPointType & center )
{
  m_Center = center;
  this->ComputeMatrixAndOffset();
}




template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetTranslation( const OutputVectorType & translation )
{
  m_Translation = translation;
  this->ComputeMatrixAndOffset();
}





// Set Identity
template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetIdentity(void)
{
  m_Angle = NumericTraits< TScalarType >::Zero;
  m_Center.Fill( 0.0 );
  m_Translation.Fill( 0.0 );

  this->ComputeMatrixAndOffset();
}



// Set Rotational Part
template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetAngle(TScalarType angle)
{
  m_Angle = angle;
  this->ComputeMatrixAndOffset();
}


// Set Rotational Part
template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::SetAngleInDegrees(TScalarType angle)
{
  const TScalarType angleInRadians = angle * atan(1.0) / 45.0;
  this->SetAngle( angleInRadians );
}


// Compute the matrix
template <class TScalarType>
void
CenteredRigid2DTransform<TScalarType>
::ComputeMatrixAndOffset( void )
{
  const double ca = cos(m_Angle);
  const double sa = sin(m_Angle);

  const double cx = m_Center[0];
  const double cy = m_Center[1];

  const double tx = m_Translation[0];
  const double ty = m_Translation[1];

  this->m_RotationMatrix[0][0]= ca; this->m_RotationMatrix[0][1]=-sa;
  this->m_RotationMatrix[1][0]= sa; this->m_RotationMatrix[1][1]= ca;

  this->m_RotationMatrixMTime.Modified();

  OffsetType offset;

  offset[0] = tx + sa * cy + ( 1.0 - ca ) * cx;
  offset[1] = ty - sa * cx + ( 1.0 - ca ) * cy;

  this->SetOffset( offset );

  this->Modified();
}


// Set parameters
template<class TScalarType>
const typename CenteredRigid2DTransform<TScalarType>::JacobianType &
CenteredRigid2DTransform<TScalarType>::
GetJacobian( const InputPointType & p ) const
{
  // need to check if angles are in the right order
  const double ca = cos(m_Angle);
  const double sa = sin(m_Angle);

  this->m_Jacobian.Fill(0.0);

  const double cx = m_Center[0];
  const double cy = m_Center[1];

  // derivatives with respect to the angle
  this->m_Jacobian[0][0] = -sa * ( p[0] - cx ) - ca * ( p[1] - cy );
  this->m_Jacobian[1][0] =  ca * ( p[0] - cx ) - sa * ( p[1] - cy ); 

  // compute derivatives with respect to the center part
  // first with respect to cx
  this->m_Jacobian[0][1] = 1.0 - ca;
  this->m_Jacobian[1][1] =     - sa;  
  // then with respect to cy
  this->m_Jacobian[0][2] =       sa;
  this->m_Jacobian[1][2] = 1.0 - ca;


  // compute derivatives with respect to the translation part
  // first with respect to tx
  this->m_Jacobian[0][3] = 1.0;
  this->m_Jacobian[1][3] = 0.0;
  // first with respect to ty
  this->m_Jacobian[0][4] = 0.0;
  this->m_Jacobian[1][4] = 1.0;

  return this->m_Jacobian;

}
  
// Create and return an inverse transformation
template<class TScalarType>
void
CenteredRigid2DTransform<TScalarType>::
CloneInverseTo( Pointer & result ) const
{
  result = New();
  result->SetCenter( m_Center );  // inverse have the same center
  result->SetAngle( -m_Angle );
  result->SetTranslation( -( this->GetInverseMatrix() * m_Translation ) );
}

// Print self
template<class TScalarType>
void
CenteredRigid2DTransform<TScalarType>::
PrintSelf(std::ostream &os, Indent indent) const
{

  Superclass::PrintSelf(os,indent);
  
  os << indent << "Angle       = " << m_Angle        << std::endl;
  os << indent << "Center      = " << m_Center       << std::endl;
  os << indent << "Translation = " << m_Translation  << std::endl;
}

} // namespace

#endif
