/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkPathFunctions_h
#define itkPathFunctions_h

#include "itkChainCodePath.h"
#include "itkFourierSeriesPath.h"
#include <cmath>

namespace itk
{
/** Make a chain code trace another path of same dimensionality.
 * If restrictMovement is true, then individual steps are allowed to move
 * through only one dimension at a time; for 2D paths this results in an
 * 8-connected chain code. */
template <typename TChainCodePath, typename TPathInput>
void
MakeChainCodeTracePath(TChainCodePath & chainPath, const TPathInput & inPath, bool restrictMovement = false)
{
  using OffsetType = typename TChainCodePath::OffsetType;
  using ChainInputType = typename TChainCodePath::InputType;
  using InPathInputType = typename TPathInput::InputType;

  const int dimension = OffsetType::GetOffsetDimension();

  constexpr OffsetType zeroOffset{};

  chainPath.Clear();
  InPathInputType inPathInput = inPath.StartOfInput();
  chainPath.SetStart(inPath.EvaluateToIndex(inPathInput));

  for (ChainInputType chainInput = 0;;)
  {
    OffsetType offset = inPath.IncrementInput(inPathInput);
    if (zeroOffset == offset)
    {
      break;
    }

    if (!restrictMovement)
    {
      chainPath.InsertStep(chainInput++, offset);
    }
    else
    {
      for (int d = 0; d < dimension; ++d)
      {
        OffsetType tempOffset{};
        tempOffset[d] = offset[d];
        chainPath.InsertStep(chainInput++, tempOffset);
      }
    }
  }
}

/** Make a Fourier series path trace a chain code path of same dimensionality.
 * numHarmonics is the number of harmonics (frequency coefficients, which
 * include the "DC" term) to compute.  If chainPath has too few steps to
 * calculate numHarmonics (due to the Nyquist criterion), then as many harmonics
 * as possible (chainPath->NumberOfSteps()/2) will be calculated.  No fewer than
 * 2 harmonics will be calculated. */
template <typename TFourierSeriesPath, typename TChainCodePath>
void
MakeFourierSeriesPathTraceChainCode(TFourierSeriesPath &   FSPath,
                                    const TChainCodePath & chainPath,
                                    unsigned int           numHarmonics = 8)
{
  using IndexType = typename TFourierSeriesPath::IndexType;
  using OffsetType = typename TFourierSeriesPath::OffsetType;
  using VectorType = typename TFourierSeriesPath::VectorType;

  using FSInputType = typename TFourierSeriesPath::InputType;
  using ChainInputType = typename TChainCodePath::InputType;

  const int    dimension = OffsetType::GetOffsetDimension();
  const size_t numSteps = chainPath.NumberOfSteps();

  const double PI = 4.0 * std::atan(1.0);

  FSPath.Clear();

  // Adjust our private copy of numHarmonics if necessary
  if (numHarmonics <= 1)
  {
    numHarmonics = 2;
  }
  else if (numHarmonics * 2 > numSteps)
  {
    numHarmonics = numSteps / 2;
  }

  for (unsigned int n = 0; n < numHarmonics; ++n)
  {
    IndexType  index = chainPath.GetStart();
    VectorType cosCoefficient{};
    VectorType sinCoefficient{};

    for (ChainInputType step = 0; step < numSteps; ++step)
    {
      index += chainPath.Evaluate(step);
      const FSInputType theta = 2 * n * PI * (static_cast<double>(step + 1)) / numSteps;

      // turn the current index into a vector
      VectorType indexVector;
      for (int d = 0; d < dimension; ++d)
      {
        indexVector[d] = index[d];
      }
      cosCoefficient += indexVector * (std::cos(theta) / numSteps);
      sinCoefficient += indexVector * (std::sin(theta) / numSteps);
    }

    FSPath.AddHarmonic(cosCoefficient, sinCoefficient);
  }
}
} // end namespace itk

#endif
