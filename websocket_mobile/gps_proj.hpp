#pragma once

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H

#include <proj_api.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

class PJTransformer {
 public:
  explicit PJTransformer(double lon_0, double lat_0) {
    // init projPJ
    std::stringstream stream;
    // stream << "+proj=utm +zone=" << zone_id << " +ellps=WGS84" << std::endl;
    // stream << "+proj=tmerc +k=0.9996 +lon_0=" << lon_0 << "e +ellps=WGS84"
    //        << std::endl;
    stream << "+proj=tmerc +k=0.9996 +lon_0=" << lon_0 << " +lat_0= " << lat_0
           << "e +ellps=WGS84" << std::endl;
    pj_utm_ = pj_init_plus(stream.str().c_str());
    // pj_utm_ = pj_init_plus_ctx();
    if (pj_utm_ == nullptr) {
      std::cout << "proj4 init failed!" << stream.str() << std::endl;
      return;
    }
    pj_latlong_ = pj_init_plus("+proj=latlong +ellps=WGS84");
    if (pj_latlong_ == nullptr) {
      std::cout << "proj4 pj_latlong init failed!";
      return;
    }
    std::cout << "proj4 init success" << std::endl;
  }

  ~PJTransformer() {
    if (pj_latlong_) {
      pj_free(pj_latlong_);
      pj_latlong_ = nullptr;
    }
    if (pj_utm_) {
      pj_free(pj_utm_);
      pj_utm_ = nullptr;
    }
  }

  int LLAToUTM(int64_t point_count, int point_offset, double *x, double *y,
               double *z) {
    if (!pj_latlong_ || !pj_utm_) {
      std::cout << "pj_latlong_:" << pj_latlong_ << "pj_utm_:" << pj_utm_
                << std::endl;
      return -1;
    }
    return pj_transform(pj_latlong_, pj_utm_, point_count, point_offset, x, y,
                        z);
  }

 private:
  projPJ pj_latlong_;
  projPJ pj_utm_;
};