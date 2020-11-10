#ifndef _3D_VIEWER_CONFIG_H
#define _3D_VIEWER_CONFIG_H

#include <string>

class Config final {
 public:
  std::string exe_dir;
  std::string resource_dir;

  static Config& Instance() {
    static Config config;
    return config;
  }

 private:
  Config() = default;
  ~Config() = default;
  Config(const Config&) = delete;
  Config(Config&&) = delete;
  Config& operator=(const Config&) = delete;
  Config& operator=(Config&&) = delete;
};

#endif  // _3D_VIEWER_CONFIG_H
