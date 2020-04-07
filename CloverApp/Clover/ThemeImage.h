//
//  ThemeImage.h
//  Clover
//
//  Created by vector sigma on 07/03/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "lodepng.h"
#import "libimagequant.h"

//NS_ASSUME_NONNULL_BEGIN

@interface ThemeImage : NSImage
@property (nonatomic, strong) NSData * _Nonnull pngData;
- (id _Nullable)initWithThemeImageAtPath:(nonnull NSString *)path error:(NSError *_Nullable*_Nullable)errorPtr;
@end

//NS_ASSUME_NONNULL_END
