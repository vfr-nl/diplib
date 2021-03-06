/*
 * DIPimage 3.0
 * This MEX-file implements the 'percentile' function
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

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;

      dip::Image in;
      dip::Image mask;
      dip::Image out = mi.NewImage();
      // TODO: second (optional) output is position

      // Get images
      in = dml::GetImage( prhs[ 0 ] );
      if( nrhs > 2 ) {
         mask = dml::GetImage( prhs[ 2 ] );
      }

      // Get parameter
      dip::dfloat percentile = dml::GetFloat( prhs[ 1 ] );

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 3 ) {
         process = dml::GetProcessArray( prhs[ 3 ], in.Dimensionality() );
      }

      // Do the thing
      dip::Percentile( in, mask, out, percentile, process );

      // Done
      if(( nrhs > 3 ) || !out.IsScalar() ) {
         plhs[ 0 ] = mi.GetArray( out );
      } else {
         plhs[ 0 ] = dml::GetArray( out.As< dip::dfloat >() );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
