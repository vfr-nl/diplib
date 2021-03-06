/*
 * DIPlib 3.0
 * This file contains definitions for ICS reading and writing
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

#ifdef DIP__HAS_ICS

#include <cstdlib> // std::strtoul

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

#include "libics.h"

// TODO: Reorder dimensions when reading, to match standard x,y,z,t order.
// TODO: When reading, check to see if image's strides match those in the file.
// TODO: Option "fast" to reorder dimensions when reading, to read without strides.

namespace dip {

namespace {

#define CALL_ICS( function_call, message ) do { Ics_Error error_ = function_call; if( error_ != IcsErr_Ok ) { DIP_THROW( String( message ": " ) + IcsGetErrorText( error_ )); }} while(false)

dip::uint FindTensorDimension(
      ICS* ics,
      UnsignedArray const& sizes,
      String& colorSpace
) {
   dip::uint nDims = sizes.size();
   colorSpace = "";
   dip::uint tensorDim;
   for( tensorDim = 0; tensorDim < nDims; ++tensorDim ) {
      char const* order;
      CALL_ICS( IcsGetOrderF( ics, static_cast< int >( tensorDim ), &order, 0 ), "Couldn't read ICS file" );
      if( strcasecmp( order, "RGB" ) == 0 ) {
         colorSpace = "RGB";
         break;
      } else if( strcasecmp( order, "sRGB" ) == 0 ) {
         colorSpace = "sRGB";
         break;
      } else if( strcasecmp( order, "Lab" ) == 0 ) {
         colorSpace = "Lab";
         break;
      } else if( strcasecmp( order, "Luv" ) == 0 ) {
         colorSpace = "Luv";
         break;
      } else if( strcasecmp( order, "LCH" ) == 0 ) {
         colorSpace = "LCH";
         break;
      } else if( strcasecmp( order, "CMY" ) == 0 ) {
         colorSpace = "CMY";
         break;
      } else if( strcasecmp( order, "CMYK" ) == 0 ) {
         colorSpace = "CMYK";
         break;
      } else if( strcasecmp( order, "XYZ" ) == 0 ) {
         colorSpace = "XYZ";
         break;
      } else if( strcasecmp( order, "Yxy" ) == 0 ) {
         colorSpace = "Yxy";
         break;
      } else if( strcasecmp( order, "HSI" ) == 0 ) {
         colorSpace = "HSI";
         break;
      } else if( strcasecmp( order, "ICH" ) == 0 ) {
         colorSpace = "ICH";
         break;
      } else if( strcasecmp( order, "ISH" ) == 0 ) {
         colorSpace = "ISH";
         break;
      } else if( strcasecmp( order, "HCV" ) == 0 ) {
         colorSpace = "HCV";
         break;
      } else if( strcasecmp( order, "HSV" ) == 0 ) {
         colorSpace = "HSV";
         break;
      } else if( strcasecmp( order, "channel" ) == 0 || strcasecmp( order, "channels" ) == 0 ||
                 strcasecmp( order, "probe"   ) == 0 || strcasecmp( order, "probes"   ) == 0 ||
                 strcasecmp( order, "tensor"  ) == 0 ) {
         break;
      }
   }
   // If the loop above doesn't break, colorDim == nDims, and colorSpace == "".
   if( tensorDim == nDims ) {
      // no color or tensor dimension recognizable from the names, but maybe there's a dimension with few samples?
      dip::uint tensorSize = 100; // initialize to something > 10
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( sizes[ ii ] <= 10 ) && ( sizes[ ii ] < tensorSize )) {
            tensorSize = sizes[ ii ];
            tensorDim = ii;
         }
      }
   }
   return tensorDim;
}

// Finds out how to reorder dimensions as they are written to the ICS file
//    y,x -> x,y
//    t,x,y -> x,y,t
//    x,q,y -> x,y,q
//    x,y,q,t -> x,y,q,t
//    x,y,t,q -> x,y,t,q
//    dim_3,dim_2,dim_1 -> dim_1,dim_2,dim_3
// - x, y, z are always first 3 dimensions
// - dim_N always goes to dimension N, unless there's a conflict with x, y, z
// - t comes after x, y, z, but otherwise is sorted where it was
// - unknown strings (e.g. q) are sorted where they are, but after x, y, z and also displaced by dim_N
// - dim_0 == x, dim_1 == y, dim_2 == z
UnsignedArray FindDimensionOrder( ICS* ics, dip::uint nDims, dip::uint tensorDim ) {
   struct FileDims {
      dip::uint order;  // where to put the dimension
      bool known;       // set if name was recognized
      bool priority;    // set if it's one of x, y, z
   };
   // Find recognized labels
   DimensionArray< FileDims > file( nDims ); // This array contains the destination location for each of the input (file) dimensions
   dip::uint maxDim = 2;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( ii == tensorDim ) {
         //std::cout << "dim " << ii << " is tensorDim\n";
         continue;
      }
      char const* order;
      CALL_ICS( IcsGetOrderF( ics, static_cast< int >( ii ), &order, 0 ), "Couldn't read ICS file" );
      //std::cout << "dim " << ii << " is " << order << std::endl;
      if( strcasecmp( order, "x" ) == 0 ) {
         file[ ii ].order = 0;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if( strcasecmp( order, "y" ) == 0 ) {
         file[ ii ].order = 1;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if( strcasecmp( order, "z" ) == 0 ) {
         file[ ii ].order = 2;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if(( std::tolower( order[ 0 ] ) == 'd' ) &&
                ( std::tolower( order[ 1 ] ) == 'i' ) &&
                ( std::tolower( order[ 2 ] ) == 'm' ) &&
                ( order[ 3 ] == '_' )) { // "dim_%d"
         // TODO: accept also "dimN", "DIM_N", "N", etc.? How many combinations? Anything with a number in it?
         char* end = nullptr;
         dip::uint dim = std::strtoul( order + 4, &end, 10 ); // Note that std::strtoul() sets `end` removing the `const` qualifier!
         if( end > order + 4 ) { // The test on `end` checks to see if std::strtoul() converted some characters.
            file[ ii ].order = dim;
            file[ ii ].known = true;
            file[ ii ].priority = false;
            maxDim = std::max( maxDim, dim );
         }
      }
   }
   // Move tensor dimension to the end
   if( tensorDim < nDims ) {
      ++maxDim;
      file[ tensorDim ].order = maxDim;
      file[ tensorDim ].known = true;
      file[ tensorDim ].priority = false;
   }
   // Create inverse lookup
   std::vector< UnsignedArray > inv( maxDim + 1 ); // This array contains the source location for each of the output dimensions
   UnsignedArray unknown;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( file[ ii ].known ) {
         inv[ file[ ii ].order ].push_back( ii );
      } else {
         unknown.push_back( ii );
      }
   }
   // Create order array
   UnsignedArray order( nDims );
   dip::uint jj = 0;
   // Put all "priority" elements first
   for( auto& list : inv ) {
      for( auto ii : list ) {
         if( file[ ii ].priority ) {
            order[ jj ] = ii;
            ++jj;
         }
      }
   }
   // Next come the non-priority ones
   auto unknownIt = unknown.begin();
   for( auto& list : inv ) {
      for( auto ii : list ) {
         if( !file[ ii ].priority ) {
            dip::uint kk = file[ ii ].order;
            while(( jj < kk ) && ( unknownIt != unknown.end() )) {
               // Put in unknown ones here so that 'dim_6' actually ends up at index 6:
               order[ jj ] = *unknownIt;
               ++unknownIt;
               ++jj;
            }
            order[ jj ] = ii;
            ++jj;
         }
      }
   }
   // Finally take the rest of the unknown ones
   while( unknownIt != unknown.end() ) {
      // Put in unused ones here so that 'dim_6' actually ends up at index 6:
      order[ jj ] = *unknownIt;
      ++unknownIt;
      ++jj;
   }
   //std::cout << "order = " << order << std::endl;
   // Double-check our work
#ifdef DIP__ENABLE_ASSERT
   DIP_ASSERT( jj == nDims );
   UnsignedArray tmp( nDims );
   std::copy( order.begin(), order.end(), tmp.begin() );
   tmp.sort();
   //std::cout << "sorted = " << tmp << std::endl;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_ASSERT( tmp[ ii ] == ii );
   }
#endif
   // Done!
   return order;
}

class IcsFile {
   public:
      // Constructor. `mode` should start with `r` or `w`.
      // When `mode` starts with `r`, don't give any other options.
      IcsFile( String const& filename, char const* mode ) {
         // Open the file. When reading, try with the exact given name first.
         if( !(( mode[ 0 ] == 'r' ) && ( IcsOpen( &ics_, filename.c_str(), "rf" ) == IcsErr_Ok ))) {
            CALL_ICS( IcsOpen( &ics_, filename.c_str(), mode ), "Couldn't open ICS file" );
         }
      }
      IcsFile( IcsFile const& ) = delete;
      IcsFile( IcsFile&& ) = delete;
      IcsFile& operator=( IcsFile const& ) = delete;
      IcsFile& operator=( IcsFile&& ) = delete;
      ~IcsFile() {
         if( ics_ ) {
            IcsClose( ics_ ); // Don't check for failures, we cannot throw here anyway.
            ics_ = nullptr;
         }
      }
      // Always call Close(), don't let the destructor close the file if all is OK -- it won't throw if there's an error.
      void Close() {
         if( ics_ ) {
            // Not using CALL_ICS here, we need to set `ics_` to NULL before throwing, otherwise the destructor is going to try to close again!
            Ics_Error error = IcsClose( ics_ );
            ics_ = nullptr;
            if( error != IcsErr_Ok ) {
               DIP_THROW( String( "Couldn't close ICS file: " ) + IcsGetErrorText( error ) );
            }
         }
      }
      // Implicit cast to ICS*
      operator ICS*() { return ics_; }
   private:
      ICS* ics_ = nullptr;
};

struct GetICSInfoData {
   FileInformation fileInformation;
   UnsignedArray fileSizes;   // Sizes in the order they appear in the file (including the tensor dimension).
   UnsignedArray order;       // How to reorder the dimensions: image dimension ii is file dimension order[ii];
                              // if there is a tensor dimension, then order.back() is tensorDim
};

GetICSInfoData GetICSInfo( IcsFile& icsFile ) {
   GetICSInfoData data;

   data.fileInformation.name = static_cast< ICS* >( icsFile )->filename;
   data.fileInformation.fileType = "ICS";
   data.fileInformation.numberOfImages = 1;

   // get layout of image data
   Ics_DataType dt;
   int ndims_;
   size_t icsSizes[ICS_MAXDIM];
   CALL_ICS( IcsGetLayout( icsFile, &dt, &ndims_, icsSizes ), "Couldn't read ICS file" );
   dip::uint nDims = static_cast< dip::uint >( ndims_ );
   size_t significantBits;
   CALL_ICS( IcsGetSignificantBits( icsFile, &significantBits ), "Couldn't read ICS file" );
   data.fileInformation.significantBits = significantBits;
   // convert ICS data type to DIP data type
   switch( dt ) {
      case Ics_uint8:
         data.fileInformation.dataType = significantBits == 1 ? DT_BIN : DT_UINT8;
         break;
      case Ics_uint16:
         data.fileInformation.dataType = DT_UINT16;
         break;
      case Ics_uint32:
         data.fileInformation.dataType = DT_UINT32;
         break;
      case Ics_sint8:
         data.fileInformation.dataType = DT_SINT8;
         break;
      case Ics_sint16:
         data.fileInformation.dataType = DT_SINT16;
         break;
      case Ics_sint32:
         data.fileInformation.dataType = DT_SINT32;
         break;
      case Ics_real32:
         data.fileInformation.dataType = DT_SFLOAT;
         break;
      case Ics_real64:
         data.fileInformation.dataType = DT_DFLOAT;
         break;
      case Ics_complex32:
         data.fileInformation.dataType = DT_SCOMPLEX;
         break;
      case Ics_complex64:
         data.fileInformation.dataType = DT_DCOMPLEX;
         break;
      default:
         DIP_THROW( "Unknown ICS data type" );
   }
   data.fileSizes.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      data.fileSizes[ ii ] = static_cast< dip::uint >( icsSizes[ ii ] );
   }

   // get pixel size
   PixelSize pixelSize;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      double scale;
      char const* units;
      CALL_ICS( IcsGetPositionF( icsFile, static_cast< int >( ii ), nullptr, &scale, &units ), "Couldn't read ICS file" );
      try {
         PhysicalQuantity ps{ scale, Units{ units }};
         ps.Normalize();
         pixelSize.Set( ii, ps );
      } catch( Error const& ) {
         // `Units` failed to parse the string
         pixelSize.Set( ii, scale );
      }
   }

   // is there a color/tensor dimension?
   dip::uint tensorDim = FindTensorDimension( icsFile, data.fileSizes, data.fileInformation.colorSpace );
   data.fileInformation.tensorElements = ( tensorDim == nDims ) ? 1 : data.fileSizes[ tensorDim ];

   // re-order dimensions
   data.order = FindDimensionOrder( icsFile, nDims, tensorDim );
   data.fileInformation.sizes.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      data.fileInformation.sizes[ ii ] = data.fileSizes[ data.order[ ii ]];
      data.fileInformation.pixelSize.Set( ii, pixelSize[ data.order[ ii ]] );
   }
   if( data.fileInformation.tensorElements > 1 ) {
      data.fileInformation.sizes.pop_back();
      //data.fileInformation.pixelSize.EraseDimension( nDims - 1 ); // doesn't do anything
   }

   // History tags
   int history_lines;
   CALL_ICS( IcsGetNumHistoryStrings( icsFile, &history_lines ), "Couldn't read ICS metadata" );
   data.fileInformation.history.resize( static_cast< dip::uint >( history_lines ));
   if( history_lines > 0 ) {
      Ics_HistoryIterator it;
      CALL_ICS( IcsNewHistoryIterator( icsFile, &it, 0 ), "Couldn't read ICS metadata");
      char const* hist;
      for( dip::uint ii = 0; ii < static_cast< dip::uint >( history_lines ); ++ii ) {
         CALL_ICS( IcsGetHistoryStringIF( icsFile, &it, &hist ), "Couldn't read ICS metadata");
         data.fileInformation.history[ ii ] = hist;
      }
   }

   // done
   return data;
}

} // namespace

FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      RangeArray roi,
      Range channels,
      String const& mode
) {
   bool fast = BooleanFromString( mode, "fast", "" );

   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));

   //std::cout << "[ImageReadICS] fileInformation.sizes = " << data.fileInformation.sizes << std::endl;
   //std::cout << "[ImageReadICS] fileInformation.tensorElements = " << data.fileInformation.tensorElements << std::endl;
   //std::cout << "[ImageReadICS] fileSizes = " << data.fileSizes << std::endl;
   //std::cout << "[ImageReadICS] order = " << data.order << std::endl;

   UnsignedArray sizes = data.fileInformation.sizes;
   UnsignedArray order = data.order;
   dip::uint nDims = sizes.size();

   // check & fix ROI information
   UnsignedArray outSizes( nDims );
   dip::uint outTensor;
   BooleanArray mirror( nDims, false );
   DIP_START_STACK_TRACE
      ArrayUseParameter( roi, nDims, Range{} );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         //std::cout << "roi[" << ii << "] = " << roi[ ii ].start << ":" << roi[ ii ].stop << ":" << roi[ ii ].step;
         roi[ ii ].Fix( sizes[ ii ] );
         //std::cout << " => " << roi[ ii ].start << ":" << roi[ ii ].stop << ":" << roi[ ii ].step << std::endl;
         if( roi[ ii ].start > roi[ ii ].stop ) {
            std::swap( roi[ ii ].start, roi[ ii ].stop );
            mirror[ ii ] = true;
         }
         outSizes[ ii ] = roi[ ii ].Size();
         if( outSizes[ ii ] != sizes[ ii ] ) {
            fast = false;
         }
      }
      //std::cout << "channels = " << channels.start << ":" << channels.stop << ":" << channels.step;
      channels.Fix( data.fileInformation.tensorElements );
      //std::cout << " => " << channels.start << ":" << channels.stop << ":" << channels.step << std::endl;
      if( channels.start > channels.stop ) {
         std::swap( channels.start, channels.stop );
         // We don't read the tensor dimension in reverse order
      }
      outTensor = channels.Size();
      if( outTensor != data.fileInformation.tensorElements ) {
         fast = false;
      }
   DIP_END_STACK_TRACE

   // prepare the strides of the image on file (including tensor dimension)
   UnsignedArray tmp( data.fileSizes.size() );
   tmp[ 0 ] = 1;
   for( dip::uint ii = 1; ii < tmp.size(); ++ii ) {
      tmp[ ii ] = tmp[ ii - 1 ] * data.fileSizes[ ii - 1 ];
   }
   IntegerArray strides( tmp.size() );
   for( dip::uint ii = 0; ii < tmp.size(); ++ii ) {
      strides[ ii ] = static_cast< dip::sint >( tmp[ data.order[ ii ]] );
   }
   // if there's a tensor dimension, it's sorted last in `strides`.
   //std::cout << "[ImageReadICS] strides = " << strides << std::endl;

   // if "fast", try to match strides with those in the file
   if( fast ) {
      IntegerArray reqStrides( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         reqStrides[ ii ] = strides[ ii ];
      }
      dip::sint reqTensorStride = outTensor > 1 ? strides.back() : 1;
      if(   ( out.Strides() != reqStrides )
         || ( out.TensorStride() != reqTensorStride )
         || ( out.Sizes() != outSizes )
         || ( out.TensorElements() != outTensor )
         || ( out.DataType() != data.fileInformation.dataType )) {
         out.Strip();
      }
      if( !out.IsForged() ) {
         out.SetStrides( reqStrides );
         out.SetTensorStride( reqTensorStride );
      }
   }

   // forge the image
   out.ReForge( outSizes, outTensor, data.fileInformation.dataType );
   if( outTensor == data.fileInformation.tensorElements ) {
      out.SetColorSpace( data.fileInformation.colorSpace );
   }
   out.SetPixelSize( data.fileInformation.pixelSize );

   // get tensor shape if necessary
   if(( outTensor > 1 ) && ( outTensor == data.fileInformation.tensorElements )) {
      Ics_HistoryIterator it;
      Ics_Error e = IcsNewHistoryIterator( icsFile, &it, "tensor" );
      if( e == IcsErr_Ok ) {
         char line[ ICS_LINE_LENGTH ];
         e = IcsGetHistoryKeyValueI( icsFile, &it, nullptr, line );
         if( e == IcsErr_Ok ) {
            // parse `value`
            char* ptr = std::strtok( line, "\t" );
            if( ptr != nullptr ) {
               char* shape = ptr;
               ptr = std::strtok( nullptr, "\t" );
               if( ptr != nullptr ) {
                  dip::uint rows = std::stoul( ptr );
                  ptr = std::strtok( nullptr, "\t" );
                  if( ptr != nullptr ) {
                     dip::uint columns = std::stoul( ptr );
                     try {
                        out.ReshapeTensor( Tensor{ shape, rows, columns } );
                     } catch ( Error const& ) {
                        // Let this error slip, we don't really care
                     }
                  }
               }
            }
         }
      }
   }
   //std::cout << "[ImageReadICS] out = " << out << std::endl;

   // make a quick copy and place the tensor dimension at the back
   Image outRef = out.QuickCopy();
   if( data.fileInformation.tensorElements > 1 ) {
      outRef.TensorToSpatial();
      roi.push_back( channels );
      sizes.push_back( outTensor );
      ++nDims;
   }
   //std::cout << "[ImageReadICS] outRef = " << outRef << std::endl;

   if( strides == out.Strides() ) {
      // Fast reading!
      //std::cout << "[ImageReadICS] fast reading!\n";

      CALL_ICS( IcsGetData( icsFile, outRef.Origin(), outRef.NumberOfPixels() ), "Couldn't read pixel data from ICS file" );

   } else {
      // Reading using strides
      //std::cout << "[ImageReadICS] reading with strides\n";
   
      // remove any singleton dimensions (in the input file, not the roi)
      // this should improve reading speed, especially if the first dimension is singleton
      for( dip::uint ii = nDims; ii > 0; ) { // loop backwards, so we don't skip a dimension when erasing
         --ii;
         if( sizes[ ii ] == 1 ) {
            sizes.erase( ii );
            roi.erase( ii );
            order.erase( ii );
            strides.erase( ii );
            outRef.Squeeze( ii );
         }
      }
      nDims = outRef.Dimensionality();

      // re-order dimensions according to strides, so that we only go forward in the file
      auto sort = strides.sortedIndices();
      outRef.PermuteDimensions( sort );
      sizes = sizes.permute( sort );
      roi = roi.permute( sort );
      order = order.permute( sort );
      strides = strides.permute( sort );

      // what is the processing dimension?
      dip::uint procDim = 0;
      for( dip::uint ii = 1; ii < order.size(); ++ii ) {
         if( order[ ii ] < order[ procDim ] ) {
            procDim = ii;
         }
      }

      // prepare the buffer
      dip::uint sizeOf = data.fileInformation.dataType.SizeOf();
      dip::uint bufSize = sizeOf * (( outRef.Size( procDim ) - 1 ) * roi[ procDim ].step + 1 );
      std::vector< uint8 > buffer( bufSize );

      // read the data
      dip::uint cur_loc = 0;
      GenericImageIterator<> it( outRef, procDim );
      do {
         // find location in file to read at
         UnsignedArray const& curipos = it.Coordinates();
         dip::uint new_loc = sizeOf * roi[ procDim ].Offset();
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != procDim ) {
               dip::uint curfpos = curipos[ ii ] * roi[ ii ].step + roi[ ii ].Offset();
               new_loc += sizeOf * curfpos * static_cast< dip::uint >( strides[ ii ] );
            }
         }
         // read line portion into buffer
         DIP_ASSERT( new_loc >= cur_loc ); // we cannot move backwards!
         if( new_loc > cur_loc ) {
            IcsSkipDataBlock( icsFile, new_loc - cur_loc );
            cur_loc = new_loc;
         }
         CALL_ICS( IcsGetDataBlock( icsFile, buffer.data(), bufSize ), "Couldn't read pixel data from ICS file" );
         cur_loc += bufSize;
         // copy buffer to image
         detail::CopyBuffer( buffer.data(), data.fileInformation.dataType, static_cast< dip::sint >( roi[ procDim ].step ), 1,
                             it.Pointer(), outRef.DataType(), outRef.Stride( procDim ), 1,
                             outRef.Size( procDim ), 1 );
      } while( ++it );

   }

   // apply the mirroring to the output image
   out.Mirror( mirror );

   // we're done
   icsFile.Close();
   return data.fileInformation;
}

FileInformation ImageReadICS(
      Image& image,
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing,
      Range const& channels,
      String const& mode
) {
   dip::uint n = origin.size();
   n = std::max( n, sizes.size() );
   n = std::max( n, spacing.size() );
   if( n > 1 ) {
      DIP_THROW_IF(( origin.size() > 1 ) && ( origin.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( sizes.size() > 1 ) && ( sizes.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( spacing.size() > 1 ) && ( spacing.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
   }
   RangeArray roi( n );
   if( origin.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ 0 ] );
      }
   } else if( origin.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ ii ] );
      }
   }
   if( sizes.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ 0 ] ) - 1;
      }
   } else if( sizes.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ ii ] ) - 1;
      }
   }
   if( spacing.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ 0 ];
      }
   } else if( spacing.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ ii ];
      }
   }
   return ImageReadICS( image, filename, roi, channels, mode );
}

FileInformation ImageReadICSInfo( String const& filename ) {
   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));

   // done
   icsFile.Close();
   return data.fileInformation;
}

bool ImageIsICS( String const& filename ) {
   return IcsVersion( filename.c_str(), 1 ) != 0;
}

namespace {

inline bool StridesArePositive( IntegerArray strides ) {
   for( auto s : strides ) {
      if( s < 1 ) {
         return false;
      }
   }
   return true;
}

} // namespace

void ImageWriteICS(
      Image const& c_image,
      String const& filename,
      StringArray const& history,
      dip::uint significantBits,
      StringSet const& options
) {
   // parse options
   bool oldStyle = false; // true if v1
   bool compress = true;
   bool fast = false;
   for( auto& option : options ) {
      if( option == "v1" ) {
         oldStyle = true;
      } else if( option == "v2" ) {
         oldStyle = false;
      } else if( option == "uncompressed" ) {
         compress = false;
      } else if( option == "gzip" ) {
         compress = true;
      } else if( option == "fast" ) {
         fast = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }

   // should we reorder dimensions?
   if( fast ) {
      if( !c_image.HasContiguousData() || !StridesArePositive( c_image.Strides() )) {
         fast = false;
      }
   }

   // find info on image
   Ics_DataType dt;
   dip::uint maxSignificantBits;
   switch( c_image.DataType()) {
      case DT_BIN:      dt = Ics_uint8;     maxSignificantBits = 1;  break;
      case DT_UINT8:    dt = Ics_uint8;     maxSignificantBits = 8;  break;
      case DT_UINT16:   dt = Ics_uint16;    maxSignificantBits = 16; break;
      case DT_UINT32:   dt = Ics_uint32;    maxSignificantBits = 32; break;
      case DT_SINT8:    dt = Ics_sint8;     maxSignificantBits = 8;  break;
      case DT_SINT16:   dt = Ics_sint16;    maxSignificantBits = 16; break;
      case DT_SINT32:   dt = Ics_sint32;    maxSignificantBits = 32; break;
      case DT_SFLOAT:   dt = Ics_real32;    maxSignificantBits = 32; break;
      case DT_DFLOAT:   dt = Ics_real64;    maxSignificantBits = 64; break;
      case DT_SCOMPLEX: dt = Ics_complex32; maxSignificantBits = 32; break;
      case DT_DCOMPLEX: dt = Ics_complex64; maxSignificantBits = 64; break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED ); // Should not happen
   }
   if( significantBits == 0 ) {
      significantBits = maxSignificantBits;
   } else {
      significantBits = std::min( significantBits, maxSignificantBits );
   }

   // Quick copy of the image, with tensor dimension moved to the end
   Image image = c_image.QuickCopy();
   bool isTensor = false;
   if( image.TensorElements() > 1 ) {
      isTensor = true;
      image.TensorToSpatial(); // last dimension
   }

   // open the ICS file
   IcsFile icsFile( filename, oldStyle ? "w1" : "w2" );

   // set info on image
   int nDims = static_cast< int >( image.Dimensionality() );
   CALL_ICS( IcsSetLayout( icsFile, dt, nDims, image.Sizes().data() ), "Couldn't write to ICS file" );
   if( nDims >= 5 ) {
      // By default, 5th dimension is called "probe", but this is turned into a tensor dimension...
      CALL_ICS( IcsSetOrder( icsFile, 4, "dim_4", 0 ), "Couldn't write to ICS file" );
   }
   CALL_ICS( IcsSetSignificantBits( icsFile, significantBits ), "Couldn't write to ICS file" );
   if( c_image.IsColor() ) {
      CALL_ICS( IcsSetOrder( icsFile, nDims - 1, c_image.ColorSpace().c_str(), 0 ), "Couldn't write to ICS file" );
   } else if( isTensor ) {
      CALL_ICS( IcsSetOrder( icsFile, nDims - 1, "tensor", 0 ), "Couldn't write to ICS file" );
   }
   if( c_image.HasPixelSize() ) {
      if( isTensor ) { nDims--; }
      for( int ii = 0; ii < nDims; ii++ ) {
         auto pixelSize = c_image.PixelSize( static_cast< dip::uint >( ii ));
         CALL_ICS( IcsSetPosition( icsFile, ii, 0.0, pixelSize.magnitude, pixelSize.units.String().c_str() ), "Couldn't write to ICS file" );
      }
      if( isTensor ) {
         CALL_ICS( IcsSetPosition( icsFile, nDims, 0.0, 1.0, nullptr ), "Couldn't write to ICS file" );
      }
   }
   if( isTensor ) {
      String tensorShape = c_image.Tensor().TensorShapeAsString() + "\t" +
                           std::to_string( c_image.Tensor().Rows() ) + "\t" +
                           std::to_string( c_image.Tensor().Columns() );
      CALL_ICS( IcsAddHistory( icsFile, "tensor", tensorShape.c_str() ), "Couldn't write metadata to ICS file" );
   }

   // set type of compression
   CALL_ICS( IcsSetCompression( icsFile, compress ? IcsCompr_gzip : IcsCompr_uncompressed, 9 ),
                 "Couldn't write to ICS file" );

   // set the image data
   if( fast ) {
      UnsignedArray order = image.Strides().sortedIndices();
      image.PermuteDimensions( order ); // This is the same as `image.StandardizeStrides()`, but with a lot of redundant checking
      DIP_ASSERT( image.HasNormalStrides() ); // Otherwise things go bad...
      ICS* ics = icsFile;
      Ics_DataRepresentation dim[ ICS_MAXDIM ];
      dip::uint nd = order.size();
      for( dip::uint ii = 0; ii < image.Dimensionality(); ++ii ) {
         std::memcpy( &( dim[ ii ] ), &( ics->dim[ order[ ii ]] ), sizeof( Ics_DataRepresentation ));
      }
      std::memcpy( ics->dim, dim, sizeof( Ics_DataRepresentation ) * nd ); // Copy only the dimensions we've set.
   }
   if( image.HasNormalStrides() ) {
      CALL_ICS( IcsSetData( icsFile, image.Origin(), image.NumberOfPixels() * image.DataType().SizeOf() ), "Couldn't write data to ICS file" );
   } else {
      CALL_ICS( IcsSetDataWithStrides( icsFile, image.Origin(), image.NumberOfPixels() * image.DataType().SizeOf(),
                                       image.Strides().data(), static_cast< int >( image.Dimensionality() )),
                "Couldn't write data to ICS file" );
   }

   // tag the data
   CALL_ICS( IcsAddHistory( icsFile, "software", "DIPlib " DIP_VERSION_STRING ), "Couldn't write metadata to ICS file" );

   // write history lines
   for( auto const& line : history ) {
      auto error = IcsAddHistory( icsFile, 0, line.c_str() );
      if(( error == IcsErr_LineOverflow ) || // history line is too long
         ( error == IcsErr_IllParameter )) { // history line contains illegal characters
         // Ignore these errors, the history line will not be written.
      }
      CALL_ICS( error, "Couldn't write metadata to ICS file" );
   }

   // write everything to file by closing it
   icsFile.Close();
}

} // namespace dip

#else // DIP__HAS_ICS

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

FileInformation ImageReadICS( Image&, String const&, RangeArray ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

FileInformation ImageReadICS( Image&, String const&, UnsignedArray const&, UnsignedArray const&, UnsignedArray const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

FileInformation ImageReadICSInfo( String const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

bool ImageIsICS( String const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

void ImageWriteICS( Image const&, String const&, StringArray const&, dip::uint, StringSet const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

}

#endif // DIP__HAS_ICS
