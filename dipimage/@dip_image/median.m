%MEDIAN   Get the median of an image.
%   VALUE = MEDIAN(B) gets the value of the median of all pixels in
%   the image B.
%
%   VALUE = MEDIAN(B,M), with M a binary image, is the same as MEDIAN(B(M)).
%
%   VALUE = MEDIAN(B,M,DIM) computes the median over the dimensions specified
%   in DIM. For example, if B is a 3D image, MEDIAN(B,[],3) returns an image
%   with 2 dimensions, containing the median over the pixel values along
%   the third dimension (z). DIM can be an array with any number of
%   dimensions. M can be [].
%
%   (TODO) [VALUE,POSITION] = MEDIAN(B,...) returns the position of the found values
%   as well. With this syntax, DIM can specify just one dimension.
%
%   COMPATIBILITY NOTE:
%   In DIPimage 2.x, MEDIAN(B), with B a tensor image, would work over all
%   tensor components, yielding a scalar image of the same size as B. To obtain
%   the old behavior:
%      reshape(median(tensortospatial(B),[],2),imsize(B));

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

function varargout = median(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = percentile(varargin{1},50,varargin{2:end});
