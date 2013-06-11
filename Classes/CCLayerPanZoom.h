/*
* CCLayerPanZoom
*
* Copyright (c) 2011 Alexey Lang
* Copyright (c) 2011 Pavel Guganov
*
* http://www.cocos2d-x.org
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include "cocos2d.h"
USING_NS_CC;

#define kCCLayerPanZoomMultitouchGesturesDetectionDelay 0.5

#ifndef INFINITY
#ifdef _MSC_VER
union MSVC_EVIL_FLOAT_HACK
{
    unsigned __int8 Bytes[4];
    float Value;
};
static union MSVC_EVIL_FLOAT_HACK INFINITY_HACK = {{0x00, 0x00, 0x80, 0x7F}};
#define INFINITY (INFINITY_HACK.Value)
#endif

#ifdef __GNUC__
#define INFINITY (__builtin_inf())
#endif

#ifndef INFINITY
#define INFINITY (1e1000)
#endif
#endif


typedef enum
{
    /** Standard mode: swipe to scroll */
    kCCLayerPanZoomModeSheet,
    /** Frame mode (i.e. drag inside objects): hold finger at edge of the screen to the sroll in this direction */
    kCCLayerPanZoomModeFrame  
} CCLayerPanZoomMode;


typedef enum
{
    kCCLayerPanZoomFrameEdgeNone,
    kCCLayerPanZoomFrameEdgeTop,
    kCCLayerPanZoomFrameEdgeBottom,
    kCCLayerPanZoomFrameEdgeLeft,
    kCCLayerPanZoomFrameEdgeRight,
    kCCLayerPanZoomFrameEdgeTopLeft,
    kCCLayerPanZoomFrameEdgeBottomLeft,
    kCCLayerPanZoomFrameEdgeTopRight,
    kCCLayerPanZoomFrameEdgeBottomRight
} CCLayerPanZoomFrameEdge;


class CCLayerPanZoom : public cocos2d::CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend to return the exactly class pointer
    static CCLayerPanZoom* layer();

    // implement the "static node()" method manually
    CREATE_FUNC(CCLayerPanZoom);

    void setMaxScale(float maxScale);
    float maxScale();
    void setMinScale(float minScale);
    float minScale(); 
    void setRubberEffectRatio(float rubberEffectRatio);
    float rubberEffectRatio();

    //ToDo add delegate
    CC_SYNTHESIZE(float, _maxTouchDistanceToClick, maxTouchDistanceToClick);
    CC_SYNTHESIZE(CCArray*, _touches, touches);
    CC_SYNTHESIZE(float, _touchDistance, touchDistance);
    CC_SYNTHESIZE(float, _minSpeed, minSpeed);
    CC_SYNTHESIZE(float, _maxSpeed, maxSpeed);
    CC_SYNTHESIZE(float, _topFrameMargin, topFrameMargin);
    CC_SYNTHESIZE(float, _bottomFrameMargin, bottomFrameMargin);
    CC_SYNTHESIZE(float, _leftFrameMargin, leftFrameMargin);
    CC_SYNTHESIZE(float, _rightFrameMargin, rightFrameMargin);

    CC_SYNTHESIZE(CCScheduler*, _scheduler, scheduler);
    CC_SYNTHESIZE(float, _rubberEffectRecoveryTime, rubberEffectRecoveryTime);

    CCRect _panBoundsRect;
    float _maxScale;
    float _minScale;

    CCLayerPanZoomMode _mode;

    CCPoint _prevSingleTouchPositionInLayer; 
    //< previous position in layer if single touch was moved.

    // Time when single touch has began, used to wait for possible multitouch 
    // gestures before reacting to single touch.
    double _singleTouchTimestamp; 

    // Flag used to call touchMoveBeganAtPosition: only once for each single touch event.
    bool _touchMoveBegan;

    float _rubberEffectRatio;
    bool _rubberEffectRecovering;
    bool _rubberEffectZooming;

    //CCStandartTouchDelegate
    void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    void ccTouchesMoved(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    void ccTouchesEnded(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    void ccTouchesCancelled(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

    // Updates position in frame mode.
    virtual void update(float dt);
    void onEnter();
    void onExit();

    //Scale and Position related
    void setPanBoundsRect(CCRect rect);
    void setPosition(CCPoint  position);
    void setScale(float scale);

    //Ruber Edges related
    void recoverPositionAndScale();
    void recoverEnded();

    //Helpers
    float topEdgeDistance();
    float leftEdgeDistance();
    float bottomEdgeDistance();    
    float rightEdgeDistance();
    float minPossibleScale();
    CCLayerPanZoomFrameEdge frameEdgeWithPoint( cocos2d::CCPoint point);
    float horSpeedWithPosition(CCPoint pos);
    float vertSpeedWithPosition(CCPoint pos);
};