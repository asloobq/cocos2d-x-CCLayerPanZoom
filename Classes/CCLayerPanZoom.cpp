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

#include "CCLayerPanZoom.h"


USING_NS_CC;

void CCLayerPanZoom::setMaxScale(float maxScale)
{
    _maxScale = maxScale;
    setScale(MIN(getScale(), _maxScale));
}

float CCLayerPanZoom::maxScale()
{
    return _maxScale;
}

void CCLayerPanZoom::setMinScale(float minScale)
{
    _minScale = minScale;
    setScale(MAX(getScale(), minScale));
}

float CCLayerPanZoom::minScale()
{
    return _minScale;
}

void CCLayerPanZoom::setRubberEffectRatio(float rubberEffectRatio)
{
    _rubberEffectRatio = rubberEffectRatio;

    // Avoid turning rubber effect On in frame mode.
    if (_mode == kCCLayerPanZoomModeFrame)
    {
        CCLOGERROR("CCLayerPanZoom#setRubberEffectRatio: rubber effect is not supported in frame mode.");
        _rubberEffectRatio = 0.0f;
    }

}

float CCLayerPanZoom::rubberEffectRatio()
{
    return _rubberEffectRatio;
}


CCLayerPanZoom* CCLayerPanZoom::layer()
{
    // 'layer' is an autorelease object
    CCLayerPanZoom *layer = CCLayerPanZoom::create();

    // return the scene
    return layer;
}

// on "init" you need to initialize your instance
bool CCLayerPanZoom::init()
{
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

    //m_bIsRelativeAnchorPoint = true;
    _touchEnabled = true;

    _maxScale = 3.0f;
    _minScale = 0.7f;

    _touchDistance = 0.0F;
    _maxTouchDistanceToClick = 315.0f;

    _mode = kCCLayerPanZoomModeSheet;
    _minSpeed = 100.0f;
    _maxSpeed = 1000.0f;
    _topFrameMargin = 100.0f;
    _bottomFrameMargin = 100.0f;
    _leftFrameMargin = 100.0f;
    _rightFrameMargin = 100.0f;

    _rubberEffectRatio = 0.0f;
    _rubberEffectRecoveryTime = 0.2f;
    _rubberEffectRecovering = false;
    _rubberEffectZooming = false;

	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(CCLayerPanZoom::onTouchesBegan, this);
	listener->onTouchesMoved = CC_CALLBACK_2(CCLayerPanZoom::onTouchesMoved, this);
	listener->onTouchesEnded = CC_CALLBACK_2(CCLayerPanZoom::onTouchesEnded, this);
	listener->onTouchesCancelled = CC_CALLBACK_2(CCLayerPanZoom::onTouchesCancelled, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
    return true;
}


void CCLayerPanZoom::onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent) {

    for ( auto &item: touches )
    {
        _touches.pushBack(item);
    }

    if (_touches.size() == 1)
    {
        _touchMoveBegan = false;
        time_t seconds;

        seconds = time (NULL);
        _singleTouchTimestamp = seconds/60;
    }
    else
        _singleTouchTimestamp = INFINITY;
}

void CCLayerPanZoom::onTouchesMoved(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent){
    bool multitouch = _touches.size() > 1;
    if (multitouch)
    {
        // Get the two first touches
        Touch *touch1 = (Touch*)_touches.at(0);
        Touch *touch2 = (Touch*)_touches.at(1);
        // Get current and previous positions of the touches
        Vec2 curPosTouch1 = Director::getInstance()->convertToGL(touch1->getLocationInView());
        Vec2 curPosTouch2 = Director::getInstance()->convertToGL(touch2->getLocationInView());

        Vec2 prevPosTouch1 = Director::getInstance()->convertToGL(touch1->getPreviousLocationInView());
        Vec2 prevPosTouch2 = Director::getInstance()->convertToGL(touch2->getPreviousLocationInView());


        // Calculate current and previous positions of the layer relative the anchor point
        Vec2 curPosLayer = curPosTouch1.getMidpoint(curPosTouch2);
        Vec2 prevPosLayer = prevPosTouch1.getMidpoint(prevPosTouch2);

        // Calculate new scale
        float prevScale = getScale() * 2;

		float prevDistPos1ToPos2 = prevPosTouch1.getDistance(prevPosTouch2);
		CCASSERT( prevDistPos1ToPos2 != 0, "Dont divide by 0.0" );
        float curScale = prevScale * curPosTouch1.getDistance(curPosTouch2) / prevDistPos1ToPos2;

        setScale( curScale);
        // Avoid scaling out from panBoundsRect when Rubber Effect is OFF.
        if (!_rubberEffectRatio)
        {
            setScale( MAX(getScale(), minPossibleScale())); 
        }
        // If scale was changed -> set new scale and fix position with new scale
        if (getScale() != prevScale)
        {
            if (_rubberEffectRatio)
            {
                _rubberEffectZooming = true;
            }
            Vec2 realCurPosLayer = convertToNodeSpace(curPosLayer);
            float deltaX = (realCurPosLayer.x - getAnchorPoint().x * getContentSize().width) * (getScale() - prevScale);
            float deltaY = (realCurPosLayer.y - getAnchorPoint().y * getContentSize().height) * (getScale() - prevScale);
            setPosition(getPosition().x - deltaX, getPosition().y - deltaY);
            _rubberEffectZooming = false;
        }
        // If current and previous position of the multitouch's center aren't equal -> change position of the layer
        if (!prevPosLayer.equals(curPosLayer))
        {            
            setPosition(getPosition().x + curPosLayer.x - prevPosLayer.x,
                getPosition().y + curPosLayer.y - prevPosLayer.y);
        }
        // Don't click with multitouch
        _touchDistance = INFINITY;
    }
    else
    {	        
        // Get the single touch and it's previous & current position.
		Touch *touch = (Touch*)_touches.at( 0 );
		Vec2 curTouchPosition = Director::getInstance()->convertToGL(touch->getLocationInView());
        Vec2 prevTouchPosition = Director::getInstance()->convertToGL(touch->getPreviousLocationInView());

        // Always scroll in sheet mode.
        if (_mode == kCCLayerPanZoomModeSheet)
        {
            // Set new position of the layer.
            setPosition(getPosition().x + curTouchPosition.x - prevTouchPosition.x,
                getPosition().y + curTouchPosition.y - prevTouchPosition.y);
        }

        // Accumulate touch distance for all modes.
        _touchDistance += curTouchPosition.distance( prevTouchPosition);

        // Inform delegate about starting updating touch position, if click isn't possible.
        if (_mode == kCCLayerPanZoomModeFrame)
        {
            if (_touchDistance > _maxTouchDistanceToClick && !_touchMoveBegan)
            {
                //ToDo add delegate here
                //[self.delegate layerPanZoom: self 
                //  touchMoveBeganAtPosition: [self convertToNodeSpace: prevTouchPosition]];
                _touchMoveBegan = true;
            }
        }
    }	
}

void CCLayerPanZoom::onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent){
    _singleTouchTimestamp = INFINITY;

    // Process click event in single touch.
    //ToDo add delegate
    if (  (_touchDistance < _maxTouchDistanceToClick) /*&& (self.delegate) */
        && (_touches.size() == 1))
    {
        Touch *touch = (Touch*)_touches.at( 0 );
        Vec2 curPos = Director::getInstance()->convertToGL(touch->getLocationInView());
        //ToDo add delegate
        /*[self.delegate layerPanZoom: self
        clickedAtPoint: [self convertToNodeSpace: curPos]
        tapCount: [touch tapCount]];*/
    }

	for ( auto &item: touches )
    {
		auto iterator = std::find( _touches.begin(), _touches.end(), item );
		if( iterator != _touches.end() )
			_touches.erase( iterator );
    }

    if (_touches.size() == 0)
    {
        _touchDistance = 0.0f;
    }

    if (!_touches.size() && !_rubberEffectRecovering)
    {
        recoverPositionAndScale();
    }
}

void CCLayerPanZoom::onTouchesCancelled(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event *pEvent) {

    for ( auto &item: touches )    {
		_touches.erase( std::find( _touches.begin(), _touches.end(), item ) );
    }

    if (_touches.size() == 0)
    {
        _touchDistance = 0.0f;
    }
}

// Updates position in frame mode.
void  CCLayerPanZoom::update(float dt){
    // Only for frame mode with one touch.
    if ( _mode == kCCLayerPanZoomModeFrame && _touches.size() == 1 )
    {
        // Do not update position if click is still possible.
        if (_touchDistance <= _maxTouchDistanceToClick)
            return;

        // Do not update position if pinch is still possible.
        time_t seconds;

        seconds = time (NULL);
        seconds /= 60;
        if (seconds - _singleTouchTimestamp < kCCLayerPanZoomMultitouchGesturesDetectionDelay)
            return;

        // Otherwise - update touch position. Get current position of touch.
        Touch *touch = (Touch*)_touches.at( 0 );
        Vec2 curPos = Director::getInstance()->convertToGL(touch->getLocationInView());

        // Scroll if finger in the scroll area near edge.
        if (frameEdgeWithPoint(curPos) != kCCLayerPanZoomFrameEdgeNone)
        {
            setPosition(getPosition().x + dt * horSpeedWithPosition( curPos),
                getPosition().y + dt * vertSpeedWithPosition(curPos));
        }

        // Inform delegate if touch position in layer was changed due to finger or layer movement.
        Vec2 touchPositionInLayer = convertToNodeSpace(curPos);
        if (!_prevSingleTouchPositionInLayer.equals(touchPositionInLayer))
        {
            _prevSingleTouchPositionInLayer = touchPositionInLayer;
            //ToDo add delegate
            //[self.delegate layerPanZoom: self 
            //      touchPositionUpdated: touchPositionInLayer];
        }

    }
}

void  CCLayerPanZoom::onEnter(){
    Layer::onEnter();
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
}

void  CCLayerPanZoom::onExit(){
    Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
    Layer::onExit();
}
void CCLayerPanZoom::setPanBoundsRect(const Rect& rect){
    _panBoundsRect = rect;
    setScale(minPossibleScale());
    setPosition(getPosition());
}

void CCLayerPanZoom::setPosition(const Vec2& position){
    const Vec2& prevPosition = getPosition();
    //May be it wouldnot work
    CCNode::setPosition(position);

	if (!_panBoundsRect.equals(Rect::ZERO) && !_rubberEffectZooming)
    {
        if (_rubberEffectRatio && _mode == kCCLayerPanZoomModeSheet)
        {
            if (!_rubberEffectRecovering)
            {
                float topDistance = getTopEdgeDistance();
                float bottomDistance = getBottomEdgeDistance();
                float leftDistance = getLeftEdgeDistance();
                float rightDistance = getRightEdgeDistance();
                float dx = getPosition().x - prevPosition.x;
                float dy = getPosition().y - prevPosition.y;
                if (bottomDistance || topDistance)
                {
                    CCNode::setPosition(getPosition().x,
                        prevPosition.y + dy * _rubberEffectRatio);
                }
                if (leftDistance || rightDistance)
                {
                    CCNode::setPosition(prevPosition.x + dx * _rubberEffectRatio,
                        getPosition().y);
                }
            }
        }
        else
        {
            Rect boundBox = getBoundingBox();
            if (getPosition().x - boundBox.size.width * getAnchorPoint().x > _panBoundsRect.origin.x)
            {
                CCNode::setPosition(boundBox.size.width * getAnchorPoint().x + _panBoundsRect.origin.x,
                    getPosition().y);
            }	
            if (getPosition().y - boundBox.size.height * getAnchorPoint().y > _panBoundsRect.origin.y)
            {
                CCNode::setPosition(getPosition().x, boundBox.size.height * getAnchorPoint().y +
                    _panBoundsRect.origin.y);
            }
            if (getPosition().x + boundBox.size.width * (1 - getAnchorPoint().x) < _panBoundsRect.size.width +
                _panBoundsRect.origin.x)
            {
                CCNode::setPosition(_panBoundsRect.size.width + _panBoundsRect.origin.x -
                    boundBox.size.width * (1 - getAnchorPoint().x), getPosition().y);
            }
            if (getPosition().y + boundBox.size.height * (1 - getAnchorPoint().y) < _panBoundsRect.size.height + 
                _panBoundsRect.origin.y)
            {
                CCNode::setPosition(getPosition().x, _panBoundsRect.size.height + _panBoundsRect.origin.y -
                    boundBox.size.height * (1 - getAnchorPoint().y));
            }	
        }
    }
}

void CCLayerPanZoom::setScale(float scale){
	float fScale = MIN(MAX(scale, _minScale), _maxScale);
	CCASSERT(fScale > 0.0f, "Scale is busted" );
	Layer::setScale( fScale );
}

void CCLayerPanZoom::recoverPositionAndScale(){
	if (!_panBoundsRect.equals(Rect::ZERO))
    {
        Size winSize = Director::getInstance()->getWinSize();
        float rightEdgeDistance = getRightEdgeDistance();
        float leftEdgeDistance = getLeftEdgeDistance();
        float topEdgeDistance = getTopEdgeDistance();
        float bottomEdgeDistance = getBottomEdgeDistance();
        float scale = minPossibleScale();

        if (!rightEdgeDistance && !leftEdgeDistance && !topEdgeDistance && !bottomEdgeDistance)
        {
            return;
        }

        if (getScale() < scale)
        {
            _rubberEffectRecovering = true;
			Vec2 newPosition = Vec2::ZERO;
            if (rightEdgeDistance && leftEdgeDistance && topEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);
                newPosition = Vec2(winSize.width * 0.5f + dx, winSize.height * 0.5f + dy);
            }
            else if (rightEdgeDistance && leftEdgeDistance && topEdgeDistance)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * (1.0f - getAnchorPoint().y);            
                newPosition = Vec2(winSize.width * 0.5f + dx, winSize.height - dy);
            }
            else if (rightEdgeDistance && leftEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * getAnchorPoint().y;            
                newPosition = Vec2(winSize.width * 0.5f + dx, dy);
            }
            else if (rightEdgeDistance && topEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * (1.0f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);            
                newPosition = Vec2(winSize.width  - dx, winSize.height  * 0.5f + dy);
            }
            else if (leftEdgeDistance && topEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);            
                newPosition = Vec2(dx, winSize.height * 0.5f + dy);
            }
            else if (leftEdgeDistance && topEdgeDistance)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * (1.0f - getAnchorPoint().y);            
                newPosition = Vec2(dx, winSize.height - dy);
            } 
            else if (leftEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * getAnchorPoint().y;            
                newPosition = Vec2(dx, dy);
            } 
            else if (rightEdgeDistance && topEdgeDistance)
            {
                float dx = scale * getContentSize().width * (1.0f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * (1.0f - getAnchorPoint().y);            
                newPosition = Vec2(winSize.width - dx, winSize.height - dy);
            } 
            else if (rightEdgeDistance && bottomEdgeDistance)
            {
                float dx = scale * getContentSize().width * (1.0f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * getAnchorPoint().y;            
                newPosition = Vec2(winSize.width - dx, dy);
            } 
            else if (topEdgeDistance || bottomEdgeDistance)
            {
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);            
                newPosition = Vec2(getPosition().x, winSize.height * 0.5f + dy);
            }
            else if (leftEdgeDistance || rightEdgeDistance)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                newPosition = Vec2(winSize.width * 0.5f + dx, getPosition().y);
            } 

            MoveTo *moveToPosition = MoveTo::create( _rubberEffectRecoveryTime,newPosition);
            ScaleTo *scaleToPosition = ScaleTo::create( _rubberEffectRecoveryTime,scale);
            FiniteTimeAction *sequence = Spawn::create(scaleToPosition, moveToPosition, CallFunc::create( CC_CALLBACK_0(CCLayerPanZoom::recoverEnded, this ) ), NULL );
            runAction(sequence);

        }
        else
        {
            _rubberEffectRecovering = false;
            MoveTo *moveToPosition = MoveTo::create(_rubberEffectRecoveryTime,Vec2(getPosition().x + rightEdgeDistance - leftEdgeDistance,
                getPosition().y + topEdgeDistance - bottomEdgeDistance));
            FiniteTimeAction *sequence = Spawn::create(moveToPosition, CallFunc::create( CC_CALLBACK_0(CCLayerPanZoom::recoverEnded, this ) ), NULL );
            runAction(sequence);

        }
    }
}

void CCLayerPanZoom::recoverEnded(){
    _rubberEffectRecovering = false;
}

float CCLayerPanZoom::getTopEdgeDistance(){
    Rect boundBox = getBoundingBox();
    return (int)(MAX(_panBoundsRect.size.height + _panBoundsRect.origin.y - getPosition().y - 
        boundBox.size.height * (1 - getAnchorPoint().y), 0));
}

float CCLayerPanZoom::getLeftEdgeDistance(){
    Rect boundBox = getBoundingBox();
    return (int)(MAX(getPosition().x - boundBox.size.width * getAnchorPoint().x - _panBoundsRect.origin.x, 0));
}    

float CCLayerPanZoom::getBottomEdgeDistance(){
    Rect boundBox = getBoundingBox();
    return (int)(MAX(getPosition().y - boundBox.size.height * getAnchorPoint().y - _panBoundsRect.origin.y, 0));
}

float CCLayerPanZoom::getRightEdgeDistance(){
    Rect boundBox = getBoundingBox();
    return (int)(MAX(_panBoundsRect.size.width + _panBoundsRect.origin.x - getPosition().x - 
        boundBox.size.width * (1 - getAnchorPoint().x), 0));
}

float CCLayerPanZoom::minPossibleScale(){
	float retVal = _minScale;
	if (!_panBoundsRect.equals(Rect::ZERO))
    {
        retVal = MAX(_panBoundsRect.size.width / getContentSize().width, _panBoundsRect.size.height /getContentSize().height);
		CCASSERT( retVal == retVal, "NAN...");
    }
    else 
    {
        retVal = _minScale;
    }
	CCASSERT( retVal == retVal, "NAN...");
	return retVal;
}

CCLayerPanZoomFrameEdge CCLayerPanZoom::frameEdgeWithPoint( const Vec2& point){
    bool isLeft = point.x <= _panBoundsRect.origin.x + _leftFrameMargin;
    bool isRight = point.x >= _panBoundsRect.origin.x + _panBoundsRect.size.width - _rightFrameMargin;
    bool isBottom = point.y <= _panBoundsRect.origin.y + _bottomFrameMargin;
    bool isTop = point.y >= _panBoundsRect.origin.y + _panBoundsRect.size.height - _topFrameMargin;

    if (isLeft && isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottomLeft;
    }
    if (isLeft && isTop)
    {
        return kCCLayerPanZoomFrameEdgeTopLeft;
    }
    if (isRight && isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottomRight;
    }
    if (isRight && isTop)
    {
        return kCCLayerPanZoomFrameEdgeTopRight;
    }

    if (isLeft)
    {
        return kCCLayerPanZoomFrameEdgeLeft;
    }
    if (isTop)
    {
        return kCCLayerPanZoomFrameEdgeTop;
    }
    if (isRight)
    {
        return kCCLayerPanZoomFrameEdgeRight;
    }
    if (isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottom;
    }

    return kCCLayerPanZoomFrameEdgeNone;
}

float CCLayerPanZoom::horSpeedWithPosition(const Vec2& pos){
    CCLayerPanZoomFrameEdge edge = frameEdgeWithPoint(pos);
    float speed = 0.0f;
    if (edge == kCCLayerPanZoomFrameEdgeLeft)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) * 
            (_panBoundsRect.origin.x + _leftFrameMargin - pos.x) / _leftFrameMargin;
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomLeft || edge == kCCLayerPanZoomFrameEdgeTopLeft)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) * 
            (_panBoundsRect.origin.x + _leftFrameMargin - pos.x) / (_leftFrameMargin * sqrt(2.0f));
    }
    if (edge == kCCLayerPanZoomFrameEdgeRight)
    {
        speed = - (_minSpeed + (_maxSpeed - _minSpeed) * 
            (pos.x - _panBoundsRect.origin.x - _panBoundsRect.size.width + 
            _rightFrameMargin) / _rightFrameMargin);
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomRight || edge == kCCLayerPanZoomFrameEdgeTopRight)
    {
        speed = - (_minSpeed + (_maxSpeed - _minSpeed) * 
            (pos.x - _panBoundsRect.origin.x - _panBoundsRect.size.width + 
            _rightFrameMargin) / (_rightFrameMargin * sqrt(2.0f)));
    }
    return speed;
}

float CCLayerPanZoom::vertSpeedWithPosition(const Vec2& pos){
    CCLayerPanZoomFrameEdge edge = frameEdgeWithPoint(pos);
    float speed = 0.0f;
    if (edge == kCCLayerPanZoomFrameEdgeBottom)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) * 
            (_panBoundsRect.origin.y + _bottomFrameMargin - pos.y) / _bottomFrameMargin;
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomLeft || edge == kCCLayerPanZoomFrameEdgeBottomRight)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) * 
            (_panBoundsRect.origin.y + _bottomFrameMargin - pos.y) / (_bottomFrameMargin * sqrt(2.0f));
    }
    if (edge == kCCLayerPanZoomFrameEdgeTop)
    {
        speed = - (_minSpeed + (_maxSpeed - _minSpeed) * 
            (pos.y - _panBoundsRect.origin.y - _panBoundsRect.size.height + 
            _topFrameMargin) / _topFrameMargin);
    }
    if (edge == kCCLayerPanZoomFrameEdgeTopLeft || edge == kCCLayerPanZoomFrameEdgeTopRight)
    {
        speed = - (_minSpeed + (_maxSpeed - _minSpeed) * 
            (pos.y - _panBoundsRect.origin.y - _panBoundsRect.size.height + 
            _topFrameMargin) / (_topFrameMargin * sqrt(2.0f)));
    }
    return speed;
} 
