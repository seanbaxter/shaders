#pragma once

struct adam7_t {
  int blocksX, blocksY;

  template<typename func_t>
  bool process(int tid, int num_threads, int num_levels, bool interlace,
    func_t& func);
};

template<typename func_t>
bool adam7_t::process(int tid, int num_threads, int num_levels, bool interlace,
  func_t& func) {

  int num_blocks = blocksX * blocksY;

  assert(0 < num_levels && num_levels <= 7);

  static const char points_per_level[7] {
    1, 1, 2, 4, 8, 16, 32,
  };
  static const char scan_points_per_level[8] {
    0, 1, 2, 4, 8, 16, 32, 64,
  };
  static const char block_points_x[64] {
    0,
    4, 
    0, 4,
    2, 6, 2, 6,
    0, 2, 4, 6, 0, 4, 2, 6,
    1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7,
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 
  };
  static const char block_points_y[64] {
    0,
    0, 
    4, 4, 
    0, 0, 4, 4, 
    2, 2, 2, 2, 6, 6, 6, 6, 
    0, 0, 0, 0, 2, 2, 2, 2, 4, 4, 4, 4, 6, 6, 6, 6,
    1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 
    5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 
  };
  static const char section_size_x[7] {
    8, 4, 4, 2, 2, 1, 1,
  };
  static const char section_size_y[7] {
    8, 8, 4, 4, 2, 2, 1,
  };

  for(int level = 0; level < num_levels; ++level) {
    int count = points_per_level[level];
    const char* lx = block_points_x + scan_points_per_level[level];
    const char* ly = block_points_y + scan_points_per_level[level];
    int sx = section_size_x[interlace ? level : num_levels - 1];
    int sy = section_size_y[interlace ? level : num_levels - 1];

    for(int block = tid; block < num_blocks; block += num_threads) {
      int bx = block % blocksX;
      int by = block / blocksX;

      int x0 = 8 * bx;
      int y0 = 8 * by;
      for(int i = 0; i < count; ++i) {
        int x = x0 + lx[i];
        int y = y0 + ly[i];

        // Invoke the function for point (x, y) to fill a section (sx, sy).
        if(!func(x, y, sx, sy))
          return false;
      }
    }
  }

  return true;
}
