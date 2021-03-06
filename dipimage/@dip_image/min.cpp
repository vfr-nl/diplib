/*
 * DIPimage 3.0
 * This MEX-file implements the 'min' function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/statistics.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;

      dip::Image in1;
      dip::Image in2; // either 2nd image or mask image
      dip::Image out = mi.NewImage();
      // TODO: second (optional) output is position

      // Get images
      in1 = dml::GetImage( prhs[ 0 ] );
      if( nrhs > 1 ) {
         in2 = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      bool hasProcess = false;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in1.Dimensionality() );
         hasProcess = true;
      }

      // Operation mode
      if( !in2.IsForged() || in2.DataType().IsBinary() ) {
         // Maximum pixel projection
         dip::Minimum( in1, in2, out, process );
         if(( hasProcess ) || !out.IsScalar() ) {
            plhs[ 0 ] = mi.GetArray( out );
         } else {
            plhs[ 0 ] = dml::GetArray( out.As< dip::dfloat >() );
         }
         if( nlhs > 1 ) {
            // Compute position also
            if( hasProcess ) {
               // TODO: minimum position projection
               DIP_THROW( dip::E::NOT_IMPLEMENTED );
            } else {
               plhs[ 1 ] = dml::GetArray( dip::MinimumPixel( in1, in2 ));
            }
         }
      } else {
         // Maximum over two images
         dip::Infimum( in1, in2, out );
         plhs[ 0 ] = mi.GetArray( out );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
