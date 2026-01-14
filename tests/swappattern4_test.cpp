// Minimal unit test for SwapPattern logic for ring.size() == 4
// This test reuses the same pattern data as BuildSwapPattern4 and
// exercises the candidate-selection logic (maximizing min-quality
// under volume-conservation). It's standalone and requires only a C++
// compiler to run.

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
  // BuildSwapPattern4 data (copied from meshGRegionLocalMeshMod.cpp)
  const int nbr_triangles = 4;
  const int triangles[4][3] = {{0, 1, 2}, {2, 3, 0}, {1, 2, 3}, {3, 0, 1}};
  const int nbr_triangles_2 = 2; // number of triangles per triangulation
  const int nbr_trianguls = 2;
  const int trianguls[2][5] = {{0, 1, -1, -1, -1}, {2, 3, -1, -1, -1}};

  // Simulate an edge cavity: we'll provide precomputed qualities and volumes
  // for each triangle when paired with v1 (tetQuality1/volume1) and v2
  // (tetQuality2/volume2). We then implement the candidate-evaluation logic
  // used by edgeSwap and assert expected choices.

  // Scenario A: both candidates conserve volume; candidate 1 (index 1)
  // has strictly higher min-quality and should be selected.
  {
    double tetQuality1[4] = {0.6, 0.7, 0.2, 0.3};
    double tetQuality2[4] = {0.6, 0.7, 0.4, 0.5};
    double volume1[4] = {1.0, 1.0, 1.0, 1.0};
    double volume2[4] = {1.0, 1.0, 1.0, 1.0};

    // compute reference volume (sum of volumes for an example cavity)
    double volumeRef = 0.0;
    for(int i = 0; i < nbr_triangles; ++i) volumeRef += volume1[i] + volume2[i];

    // reference min quality for original cavity (simulate)
    double tetQualityRef = 0.1; // low baseline

    // evaluate candidates
    std::vector<double> minQuality(nbr_trianguls, 1.0);
    for(int i = 0; i < nbr_trianguls; ++i) {
      double volume = 0.0;
      minQuality[i] = 1.0;
      for(int j = 0; j < nbr_triangles_2; ++j) {
        int iT = trianguls[i][j];
        // combine qualities from both halves
        minQuality[i] = std::min(minQuality[i], tetQuality1[iT]);
        minQuality[i] = std::min(minQuality[i], tetQuality2[iT]);
        volume += (volume1[iT] + volume2[iT]);
      }
      if(std::fabs(volume - volumeRef) > 1.e-10 * (volume + volumeRef)) minQuality[i] = -1;
    }

    // choose best
    int iBest = 0; double best = -1.0;
    for(int i = 0; i < nbr_trianguls; ++i) {
      if(minQuality[i] > best) { best = minQuality[i]; iBest = i; }
    }

    // candidate 0 uses triangles {0,1} -> qualities min(0.6,0.7,0.6,0.7)=0.6
    // candidate 1 uses triangles {2,3} -> qualities min(0.2,0.3,0.4,0.5)=0.2
    // Given these inputs, best should be candidate 0 (index 0)
    assert(iBest == 0);
    assert(best > tetQualityRef);
    std::cout << "Scenario A: selected candidate " << iBest << " with minQuality=" << best << "\n";
  }

  // Scenario B: force volume mismatch for candidate 0 so it is discarded
  {
    double tetQuality1[4] = {0.6, 0.7, 0.9, 0.8};
    double tetQuality2[4] = {0.6, 0.7, 0.9, 0.8};
    double volume1[4] = {1.0, 1.0, 1.0, 1.0};
    double volume2[4] = {1.0, 1.0, 0.1, 1.0}; // slight mismatch on triangle 2

    double volumeRef = 0.0;
    for(int i = 0; i < 4; ++i) volumeRef += volume1[i] + volume2[i];

    double tetQualityRef = 0.5;

    std::vector<double> minQuality(nbr_trianguls, 1.0);
    for(int i = 0; i < nbr_trianguls; ++i) {
      double volume = 0.0;
      minQuality[i] = 1.0;
      for(int j = 0; j < nbr_triangles_2; ++j) {
        int iT = trianguls[i][j];
        minQuality[i] = std::min(minQuality[i], tetQuality1[iT]);
        minQuality[i] = std::min(minQuality[i], tetQuality2[iT]);
        volume += (volume1[iT] + volume2[iT]);
      }
      if(std::fabs(volume - volumeRef) > 1.e-10 * (volume + volumeRef)) minQuality[i] = -1;
    }

    int iBest = 0; double best = -1.0;
    for(int i = 0; i < nbr_trianguls; ++i) {
      if(minQuality[i] > best) { best = minQuality[i]; iBest = i; }
    }

    // Because we deliberately introduced a volume mismatch for candidate 0,
    // minQuality[0] should be -1 and candidate 1 should win if its minQuality
    // is greater than tetQualityRef.
    assert(minQuality[0] < 0);
    assert(best >= 0);
    std::cout << "Scenario B: selected candidate " << iBest << " with minQuality=" << best << "\n";
  }

  std::cout << "swappattern4_test: all assertions passed." << std::endl;
  return 0;
}
