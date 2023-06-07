#include "get_app_support_dir.h"

#import <Foundation/Foundation.h>

std::string GetAppSupportDir() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString *applicationSupportDirectory = [paths firstObject];
  
  return std::string([applicationSupportDirectory UTF8String]);
}