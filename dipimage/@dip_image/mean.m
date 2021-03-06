%MEAN   Mean of all pixels in an image.
%   VALUE = MEAN(B) returns the mean intensity of all pixels in image B.
%
%   VALUE = MEAN(B,M) only computes the mean of the pixels within the
%   mask specified by the binary image M, and is equivalent to MEAN(B(M)).
%
%   VALUE = MEAN(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of dimensions.
%   M can be [].
%
%   COMPATIBILITY NOTE:
%   In DIPimage 2.x, MEAN(B), with B a tensor image, would work over all tensor
%   components, yielding a scalar image of the same size as B. To obtain
%   the old behavior:
%      reshape(mean(tensortospatial(B),[],2),imsize(B));

% (c)2017, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.
