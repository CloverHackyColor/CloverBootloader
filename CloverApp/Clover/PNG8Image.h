//
//  PNG8Image.h
//  Clover
//
//  Created by vector sigma on 07/03/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "lodepng.h"
#import "libimagequant.h"

//NS_ASSUME_NONNULL_BEGIN

@interface PNG8Image : NSImage
- (nullable NSData *)png8ImageDataAtPath:(NSString *_Nonnull)imagePath
                                   error:(NSError *_Nullable*_Nullable)errorPtr;

@end

//NS_ASSUME_NONNULL_END
