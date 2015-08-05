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
#ifndef __CCLAYERPANZOOM_H__
#define __CCLAYERPANZOOM_H__

#include "cocos2d.h"

#define kCCLayerPanZoomMultitouchGesturesDetectionDelay 0.5

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


class CCLayerPanZoom : public cocos2d::Layer
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
    //CC_SYNTHESIZE(std::vector<cocos2d::Touch*>, _touches, touches);
    CC_SYNTHESIZE(float, _touchDistance, touchDistance);
    CC_SYNTHESIZE(float, _minSpeed, minSpeed);
    CC_SYNTHESIZE(float, _maxSpeed, maxSpeed);
    CC_SYNTHESIZE(float, _topFrameMargin, topFrameMargin);
    CC_SYNTHESIZE(float, _bottomFrameMargin, bottomFrameMargin);
    CC_SYNTHESIZE(float, _leftFrameMargin, leftFrameMargin);
    CC_SYNTHESIZE(float, _rightFrameMargin, rightFrameMargin);

    CC_SYNTHESIZE(cocos2d::Scheduler*, _scheduler, scheduler);
    CC_SYNTHESIZE(float, _rubberEffectRecoveryTime, rubberEffectRecoveryTime);

	cocos2d::Rect _panBoundsRect;
	cocos2d::Vector<cocos2d::Touch*> _touches;
    float _maxScale;
    float _minScale;

    CCLayerPanZoomMode _mode;

    cocos2d::Vec2 _prevSingleTouchPositionInLayer;
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
    virtual void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent);
    virtual void onTouchesMoved(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent);
    virtual void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent);
    virtual void onTouchesCancelled(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent);
	
    // Updates position in frame mode.
    virtual void update(float dt);
    void onEnter();
    void onExit();

    //Scale and Position related
	void setPanBoundsRect(const cocos2d::Rect& rect);
	void setPosition(const cocos2d::Vec2& position);
	void setPosition( float x, float y);
    void setScale(float scale);

    //Ruber Edges related
    void recoverPositionAndScale();
    void recoverEnded();

    //Helpers
    float getTopEdgeDistance();
    float getLeftEdgeDistance();
    float getBottomEdgeDistance();
    float getRightEdgeDistance();
    float minPossibleScale();
    CCLayerPanZoomFrameEdge frameEdgeWithPoint( const cocos2d::Vec2& point);
    float horSpeedWithPosition(const cocos2d::Vec2& pos);
    float vertSpeedWithPosition(const cocos2d::Vec2& pos);
};
#endif