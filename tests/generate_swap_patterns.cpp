// generate_swap_patterns.cpp
// Generate triangles and triangulations for ring sizes 3..7 and compare to
// doc/swap_patterns.md to ensure the file matches algorithmic generation.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using Tri = std::vector<int>;
using Triangulation = std::vector<Tri>;

// generate all triangles (i<j<k) for k vertices
std::vector<Tri> gen_all_triangles(int k)
{
  std::vector<Tri> res;
  for(int i = 0; i < k - 2; ++i)
    for(int j = i + 1; j < k - 1; ++j)
      for(int l = j + 1; l < k; ++l) res.push_back({i, j, l});
  return res;
}

// recursive generation of all triangulations for polygon vertices [0..k-1]
std::vector<Triangulation> triangulations_rec(int l, int r)
{
  std::vector<Triangulation> out;
  if(r - l < 2) {
    out.push_back(Triangulation());
    return out;
  }
  for(int m = l + 1; m <= r - 1; ++m) {
    auto lefts = triangulations_rec(l, m);
    auto rights = triangulations_rec(m, r);
    for(auto &L : lefts) {
      for(auto &R : rights) {
        Triangulation comb = L;
        comb.insert(comb.end(), R.begin(), R.end());
        comb.push_back({l, m, r});
        out.push_back(comb);
      }
    }
  }
  return out;
}

std::vector<Triangulation> triangulations(int k)
{
  return triangulations_rec(0, k - 1);
}

// parse swap_patterns.md for given k, extracting triangles and trianguls
bool parse_swap_patterns_md(const std::string &path,
                            std::map<int, std::vector<Tri>> &triangles_map,
                            std::map<int, std::vector<std::vector<int>>> &trianguls_map)
{
  std::ifstream in(path);
  if(!in) return false;
  std::string line;
  int curk = -1;
  enum Section { NONE, TRIANGLES, TRIANGULS } section = NONE;
  while(std::getline(in, line)) {
    if(line.find("## ring.size() ==") != std::string::npos) {
      // extract k
      std::istringstream ss(line);
      std::string tmp;
      while(ss >> tmp) {
        if(tmp == "==") { ss >> curk; break; }
      }
      section = NONE;
      continue;
    }
    if(curk == -1) continue;
    if(line.find("triangles (nbr_triangles") != std::string::npos) {
      section = TRIANGLES;
      continue;
    }
    if(line.find("trianguls (nbr_trianguls") != std::string::npos) {
      section = TRIANGULS;
      continue;
    }
    if(section == TRIANGLES) {
      // expect lines like: 0. {0, 1, 2}
      auto p = line.find('{');
      if(p == std::string::npos) continue;
      auto q = line.find('}');
      if(q == std::string::npos) continue;
      std::string inside = line.substr(p + 1, q - p - 1);
      std::istringstream ss(inside);
      Tri t; int v;
      while(ss >> v) {
        t.push_back(v);
        if(ss.peek() == ',') ss.get();
      }
      // store triangles in sorted (canonical) order for comparison
      std::sort(t.begin(), t.end());
      triangles_map[curk].push_back(t);
    }
    if(section == TRIANGULS) {
      auto p = line.find('{');
      if(p == std::string::npos) continue;
      auto q = line.find('}');
      if(q == std::string::npos) continue;
      std::string inside = line.substr(p + 1, q - p - 1);
      std::istringstream ss(inside);
      std::vector<int> row; int v;
      while(ss >> v) {
        row.push_back(v);
        if(ss.peek() == ',') ss.get();
      }
      trianguls_map[curk].push_back(row);
    }
  }
  return true;
}

int main()
{
  std::map<int, std::vector<Tri>> md_triangles;
  std::map<int, std::vector<std::vector<int>>> md_trianguls;
  if(!parse_swap_patterns_md("doc/swap_patterns.md", md_triangles, md_trianguls)) {
    std::cerr << "Failed to open doc/swap_patterns.md" << std::endl;
    return 2;
  }

  bool ok = true;
  for(int k = 3; k <= 7; ++k) {
    auto gen_tri = gen_all_triangles(k);
    auto gen_tris = triangulations(k);

    // build triangle index map from generated triangles for consistent ordering
    std::map<std::vector<int>, int> tri_index;
    for(size_t i = 0; i < gen_tri.size(); ++i) {
      auto tri = gen_tri[i];
      std::sort(tri.begin(), tri.end());
      tri_index[tri] = (int)i;
    }

    // compare triangles count
    if(md_triangles[k].size() != gen_tri.size()) {
      std::cerr << "k=" << k << " triangle count mismatch: md=" << md_triangles[k].size()
                << " gen=" << gen_tri.size() << std::endl;
      ok = false;
    }

    // compare trianguls count
    if(md_trianguls[k].size() != gen_tris.size()) {
      std::cerr << "k=" << k << " triangulation count mismatch: md=" << md_trianguls[k].size()
                << " gen=" << gen_tris.size() << std::endl;
      ok = false;
    }

    // compare content by mapping md triangulations into generated triangle indices
    // First build md index -> gen index mapping
    std::vector<int> md2gen_idx;
    md2gen_idx.resize(md_triangles[k].size(), -1);
    for(size_t j = 0; j < md_triangles[k].size(); ++j) {
      auto tri = md_triangles[k][j];
      std::sort(tri.begin(), tri.end());
      auto it = tri_index.find(tri);
      if(it == tri_index.end()) {
        std::cerr << "k="<<k<<" md triangle not found in generated triangles\n";
        ok = false;
      } else md2gen_idx[j] = it->second;
    }

    // build multiset representations for generated triangulations
    std::multiset<std::multiset<int>> gen_sets;
    for(auto &gt : gen_tris) {
      std::multiset<int> s;
      for(auto &tri : gt) {
        Tri t = tri;
        std::sort(t.begin(), t.end());
        auto it = tri_index.find(t);
        if(it==tri_index.end()) { ok=false; break; }
        s.insert(it->second);
      }
      gen_sets.insert(s);
    }

    // build multiset representations for md triangulations (mapped to gen indices)
    std::multiset<std::multiset<int>> md_sets;
    for(auto &mr : md_trianguls[k]) {
      std::multiset<int> s;
      for(int idx : mr) {
        if(idx == -1) break;
        if(idx < 0 || (size_t)idx >= md2gen_idx.size()) { ok=false; break; }
        s.insert(md2gen_idx[idx]);
      }
      md_sets.insert(s);
    }

    if(gen_sets != md_sets) {
      std::cerr << "k="<<k<<" triangulations set mismatch\n";
      ok = false;
    }
    std::cout << "k="<<k<<" gen triangles="<<gen_tri.size()<<" gen trianguls="<<gen_tris.size() <<"\n";
  }

  if(!ok) {
    std::cerr << "swap patterns mismatch!" << std::endl;
    return 1;
  }
  std::cout << "swap patterns generation matches doc/swap_patterns.md" << std::endl;
  return 0;
}
