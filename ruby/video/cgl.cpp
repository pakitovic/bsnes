#define GL_ALPHA_TEST 0x0bc0
#include "opengl/opengl.hpp"

struct VideoCGL;

@interface RubyVideoCGL : NSView {
@public
  VideoCGL* video;
}
-(id) initWith:(VideoCGL*)video;
-(BOOL) isFlipped;
-(void) drawRect:(NSRect)rect;
-(void) reshape;
-(BOOL) acceptsFirstResponder;
@end

@interface RubyWindowCGL : NSWindow <NSWindowDelegate> {
@public
  VideoCGL* video;
}
-(id) initWith:(VideoCGL*)video;
-(BOOL) canBecomeKeyWindow;
-(BOOL) canBecomeMainWindow;
@end

struct VideoCGL : VideoDriver, OpenGL {
  VideoCGL& self = *this;
  VideoCGL(Video& super) : VideoDriver(super) {}
  ~VideoCGL() { terminate(); }

  auto create() -> bool override {
    return initialize();
  }

  auto driver() -> string override { return "OpenGL 3.2"; }
  auto ready() -> bool override { return _ready; }

  auto hasFullScreen() -> bool override { return true; }
  auto hasContext() -> bool override { return true; }
  auto hasBlocking() -> bool override { return true; }
  auto hasFlush() -> bool override { return true; }
  auto hasShader() -> bool override { return true; }

  auto setFullScreen(bool fullScreen) -> bool override {
    return initialize();
  }

  auto setContext(uintptr context) -> bool override {
    return initialize();
  }

  auto setBlocking(bool blocking) -> bool override {
    return true;
  }

  auto setFlush(bool flush) -> bool override {
    return true;
  }

  auto setShader(string shader) -> bool override {
    return true;
  }

  auto focused() -> bool override {
    return true;
  }

  auto clear() -> void override {
    @autoreleasepool {
      if(view) [view setNeedsDisplay:YES];
    }
  }

  auto size(uint& width, uint& height) -> void override {
    @autoreleasepool {
      syncViewFrame();
      auto area = [view convertRectToBacking:[view bounds]];
      width = area.size.width;
      height = area.size.height;
    }
  }

  auto acquire(uint32_t*& data, uint& pitch, uint width, uint height) -> bool override {
    if(this->width != width || this->height != height) {
      this->width = width, this->height = height;
      if(buffer) delete[] buffer;
      buffer = new uint32_t[width * height]();
    }
    pitch = width * sizeof(uint32_t);
    return data = buffer;
  }

  auto release() -> void override {
  }

  auto output(uint width, uint height) -> void override {
    @autoreleasepool {
      if(!view || !buffer) return;
      syncViewFrame();
      auto bounds = [view bounds];
      auto scale = [[view window] backingScaleFactor];
      if(!scale) scale = [[NSScreen mainScreen] backingScaleFactor];
      drawWidth = width ? (CGFloat)width / scale : bounds.size.width;
      drawHeight = height ? (CGFloat)height / scale : bounds.size.height;
      [view setNeedsDisplay:YES];
      [view display];
    }
  }

  auto draw() -> void {
    auto context = [[NSGraphicsContext currentContext] CGContext];
    auto bounds = [view bounds];
    CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 1.0);
    CGContextFillRect(context, NSRectToCGRect(bounds));
    if(!buffer || !width || !height) return;

    auto outputWidth = drawWidth ? drawWidth : bounds.size.width;
    auto outputHeight = drawHeight ? drawHeight : bounds.size.height;
    auto outputX = (bounds.size.width - outputWidth) / 2.0;
    auto outputY = (bounds.size.height - outputHeight) / 2.0;

    auto rgb = new uint8_t[width * height * 3];
    for(uint y = 0; y < height; y++) {
      for(uint x = 0; x < width; x++) {
        auto source = (height - 1 - y) * width + x;
        auto target = (y * width + x) * 3;
        auto color = buffer[source];
        rgb[target + 0] = color >> 16 & 255;
        rgb[target + 1] = color >>  8 & 255;
        rgb[target + 2] = color >>  0 & 255;
      }
    }

    auto colorSpace = CGColorSpaceCreateDeviceRGB();
    auto provider = CGDataProviderCreateWithData(nullptr, rgb, width * height * 3, nullptr);
    auto image = CGImageCreate(width, height, 8, 24, width * 3, colorSpace, kCGImageAlphaNone, provider, nullptr, false, kCGRenderingIntentDefault);
    if(image) {
      CGContextSetInterpolationQuality(context, kCGInterpolationNone);
      CGContextSetShouldAntialias(context, false);
      CGContextDrawImage(context, CGRectMake(outputX, outputY, outputWidth, outputHeight), image);
      CGImageRelease(image);
    }
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    delete[] rgb;
  }

  auto syncViewFrame() -> void {
    if(!target || !view) return;
    auto contentView = [[target window] contentView];
    if(!contentView) return;
    auto frame = [target convertRect:[target bounds] toView:contentView];
    [view setFrame:frame];
  }

private:
  auto initialize() -> bool {
    terminate();
    if(!self.fullScreen && !self.context) return false;

    @autoreleasepool {
      if(self.fullScreen) {
        window = [[RubyWindowCGL alloc] initWith:this];
        [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [window toggleFullScreen:nil];
      //[NSApp setPresentationOptions:NSApplicationPresentationFullScreen];
      }

      auto context = self.fullScreen ? [window contentView] : (NSView*)self.context;
      target = context;
      auto contentView = [[context window] contentView];
      view = [[RubyVideoCGL alloc] initWith:this];
      [view setFrame:[context convertRect:[context bounds] toView:contentView]];
      [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
      [contentView addSubview:view positioned:NSWindowAbove relativeTo:nil];
      [[view window] makeFirstResponder:view];
    }

    clear();
    return _ready = true;
  }

  auto terminate() -> void {
    _ready = false;
    if(buffer) { delete[] buffer; buffer = nullptr; }
    width = 0, height = 0;

    @autoreleasepool {
      if(view) {
        [view removeFromSuperview];
        [view release];
        view = nil;
      }

      if(window) {
      //[NSApp setPresentationOptions:NSApplicationPresentationDefault];
        [window toggleFullScreen:nil];
        [window setCollectionBehavior:NSWindowCollectionBehaviorDefault];
        [window close];
        [window release];
        window = nil;
      }
    }
    target = nullptr;
  }

  RubyVideoCGL* view = nullptr;
  NSView* target = nullptr;
  RubyWindowCGL* window = nullptr;
  CGFloat drawWidth = 0;
  CGFloat drawHeight = 0;

  bool _ready = false;
};

@implementation RubyVideoCGL : NSView

-(id) initWith:(VideoCGL*)videoPointer {
  if(self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)]) {
    video = videoPointer;
  }
  return self;
}

-(BOOL) isFlipped {
  return YES;
}

-(void) drawRect:(NSRect)rect {
  video->draw();
}

-(void) reshape {
  [self setNeedsDisplay:YES];
}

-(BOOL) acceptsFirstResponder {
  return YES;
}

-(void) keyDown:(NSEvent*)event {
}

-(void) keyUp:(NSEvent*)event {
}

@end

@implementation RubyWindowCGL : NSWindow

-(id) initWith:(VideoCGL*)videoPointer {
  auto primaryRect = [[[NSScreen screens] objectAtIndex:0] frame];
  if(self = [super initWithContentRect:primaryRect styleMask:0 backing:NSBackingStoreBuffered defer:YES]) {
    video = videoPointer;
    [self setDelegate:self];
    [self setReleasedWhenClosed:NO];
    [self setAcceptsMouseMovedEvents:YES];
    [self setTitle:@""];
    [self makeKeyAndOrderFront:nil];
  }
  return self;
}

-(BOOL) canBecomeKeyWindow {
  return YES;
}

-(BOOL) canBecomeMainWindow {
  return YES;
}

@end
