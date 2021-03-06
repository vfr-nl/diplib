/*
 * DIPimage 3.0
 * This MEX-file implements the `colorspacemanager` private function, which is used by `dip_image/colorspace`
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

/*
 * Interface:
 *
 * out = colorspacemanager(in,col)
 *
 * in = input image
 * col = color space name
 */

// TODO: Add ability to query color space (e.g. get number of required tensor elements)

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/color.h"

dip::ColorSpaceManager* csm = nullptr;

// Destroy CSM object when MEX-file is removed from memory
static void AtExit(void) {
   if( csm ) {
      //mexPrintf( "Deleting CSM.\n" );
      delete csm;
      csm = nullptr;
   }
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 2 );

      // Create CSM object
      if( !csm ) {
         //mexPrintf( "Creating CSM.\n" );
         csm = new dip::ColorSpaceManager;
         mexAtExit( AtExit );
      }

      dip::Image in = dml::GetImage( prhs[ 0 ] );
      dip::String col = dml::GetString( prhs[ 1 ] );

      if( !in.IsColor() && in.TensorElements() > 1 ) {
         // Set the color space, if correct number of tensor elements
         DIP_THROW_IF( csm->NumberOfChannels( col ) != in.TensorElements(), dip::E::INCONSISTENT_COLORSPACE );
         plhs[ 0 ] = mxCreateSharedDataCopy( prhs[ 0 ] ); // TODO: This is broken, we're changing the input also!
         mxSetPropertyShared( plhs[ 0 ], 0, dml::colspPropertyName, dml::GetArray( csm->CanonicalName( col )));
      } else {
         // Convert the color space
         dml::MatlabInterface mi;
         dip::Image out = mi.NewImage();
         csm->Convert( in, out, col );
         plhs[ 0 ] = mi.GetArray( out );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
