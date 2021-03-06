/*
 * DIPlib 3.0
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_UNIT_TESTS_H
#define DIP_UNIT_TESTS_H

#include "diplib.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

namespace dip {

DIP_EXPORT int run_unit_tests( int argc, const char* const* argv );

} // namespace dip

#endif // DIP_UNIT_TESTS_H
