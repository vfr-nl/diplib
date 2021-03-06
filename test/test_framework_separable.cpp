#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/framework.h"
#include "diplib/testing.h"

// Testing the separable framework and the line-by-line iterators.

class LineFilter : public dip::Framework::SeparableLineFilter {
   public:
      virtual void Filter( dip::Framework::SeparableLineFilterParameters const& params ) override {
         float const* filter = &( filter_[ N ] );
         dip::ConstSampleIterator< dip::sfloat > in(
               static_cast< dip::sfloat* >(params.inBuffer.buffer),
               params.inBuffer.stride );
         dip::SampleIterator< dip::sfloat > out(
               static_cast< dip::sfloat* >(params.outBuffer.buffer),
               params.outBuffer.stride );
         for( dip::uint ii = 0; ii < params.inBuffer.length; ++ii ) {
            float res = 0;
            for( dip::sint jj = -N; jj <= N; ++jj ) {
               res += in[ jj ] * filter[ jj ];
            }
            *out = res;
            ++in;
            ++out;
         }
      }
   private:
      static constexpr dip::sint N = 2;
      std::array< float, 2 * N + 1 > filter_{
            {
                  1.0f / 9.0f, 2.0f / 9.0f, 3.0f / 9.0f, 2.0f / 9.0f, 1.0f / 9.0f
            }
      };
};

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      img.Fill( 9563 );
      dip::Random random( 0 );
      dip::GaussianNoise( img, img, random, 500.0 );

      dip::testing::PrintPixelValues< dip::uint16 >( img );

      {
         // Copied from src/documentation/iterators.md
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip::ImageIterator< dip::uint16 > it( img, 0 );
         do {
            auto lit = it.GetLineIterator();
            dip::uint sum = 0;
            do {
               sum += *lit;
            } while( ++lit );
            dip::uint mean = sum / lit.Length();
            lit = it.GetLineIterator();
            do {
               dip::uint res = mean == 0 ? 0 : ( *lit * 1000u ) / mean;
               *lit = dip::clamp_cast< dip::uint16 >( res );
            } while( ++lit );
         } while( ++it );
      }

      dip::testing::PrintPixelValues< dip::uint16 >( img );

      dip::Image out = img.Similar( dip::DT_SFLOAT );
      {
         // Copied from src/documentation/iterators.md
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         DIP_THROW_IF( out.DataType() != dip::DT_SFLOAT, "Expecting single-precision float image" );
         constexpr dip::uint N = 2;
         std::array< double, 2 * N + 1 > filter{ { 1.0 / 9.0, 2.0 / 9.0, 3.0 / 9.0, 2.0 / 9.0, 1.0 / 9.0 } };
         dip::JointImageIterator< dip::uint16, dip::sfloat > it( { img, out }, 0 );
         do {
            auto iit = it.GetLineIterator< 0 >();
            auto oit = it.GetLineIterator< 1 >();
            // At the beginning of the line the filter has only partial support within the image
            for( dip::uint ii = N; ii > 0; --ii, ++oit ) {
               *oit = std::inner_product( filter.begin() + ii, filter.end(), iit, 0.0f );
            }
            // In the middle of the line the filter has full support
            for( dip::uint ii = N; ii < oit.Length() - N; ++ii, ++iit, ++oit ) {
               *oit = std::inner_product( filter.begin(), filter.end(), iit, 0.0f );
            }
            // At the end of the line the filter has only partial support
            for( dip::uint ii = 1; ii <= N; ++ii, ++iit, ++oit ) {
               *oit = std::inner_product( filter.begin(), filter.end() - ii, iit, 0.0f );
            }
         } while( ++it );
      }

      dip::testing::PrintPixelValues< dip::sfloat >( out );

      LineFilter lineFilter;
      dip::Framework::Separable( img, out, dip::DT_SFLOAT, dip::DT_SFLOAT,
            dip::BooleanArray{ true, false }, { 2 },
            dip::BoundaryConditionArray{ dip::BoundaryCondition::ADD_ZEROS },
            lineFilter, dip::Framework::Separable_AsScalarImage);

      dip::testing::PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
