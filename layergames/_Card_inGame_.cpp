//
//  _Card_inGame_.cpp
//  iCasino_v2
//
//  Created by Thanhhv on 5/19/14.
//  Layer chứa các quân bài cũng như các thao tác với quân bài
//

#include "_Card_inGame_.h"
#include "mUtils.h"
#include "SceneManager.h"
#include "ChanUtils.h"
#include "orderalgo.h"
#include "PhomMessDef.h"

enum {
	kTagCountCard = 0
};
int disCardAdd = 100;
vector<string> LayerCardInGame::split(string &S, const char &str) {
	vector<string> arrStr;
	string::iterator t, t2;

	for (t = S.begin(); t < S.end();){
		// Lặp từ vị trí bắt đầu
		t2 = find(t, S.end(), str);

		//kể từ vị trí t
		if (t != t2){
			arrStr.push_back(string(t, t2));
		}
		else if (t2 != S.end())
		{
			arrStr.push_back("");
		}


		if (t2 == S.end())
		{
			break;
		}
		t = t2 + 1;
	}

	return arrStr;
}

LayerCardInGame::LayerCardInGame()
{
	mCallBackListener = NULL;
	mCallBack = NULL;
	cardBackLeft = NULL;
	cardBackRight = NULL;
	cardBackTop = NULL;
	cardBackBot = NULL;

	ANIMATION_DURATION = 0.2;
	REFRESH_DURATION = 0.2;
	DELAY_DURATION = 0.03;
	DEAL_DURATION = 0.08;
}
LayerCardInGame::~LayerCardInGame() {
	arrCardOnHand.clear();
	arrIDCardOnHandLeft.clear();
	arrIDCardOnHandRight.clear();
	arrIDCardOnHandTop.clear();
	arrIDCardOnHandBot.clear();
	arrIDCardOnTable.clear();
	log("~~~~LayerCardInGame");
}
/*
void LayerCardInGame::onExit() {
log("onExit: clean LayerCardInGame");

CC_SAFE_DELETE_ARRAY(arrCardOnHand);
CC_SAFE_DELETE_ARRAY(arrAllCard);
//CC_SAFE_RELEASE(arrCardOnHand);
//CC_SAFE_RELEASE(arrAllCard);

arrIDCardOnHandLeft.clear();
arrIDCardOnHandRight.clear();
arrIDCardOnHandTop.clear();
arrIDCardOnHandBot.clear();
arrIDCardOnTable.clear();
}
*/
// mang co dang id:xx:xx:xx:xx;id:xx:xx:xx:xx;id:xx:xx:xx:xx ta chi can tach ra va lay cac id
vector<int> LayerCardInGame::getIDFromString(string pList) {
	vector<string> arrString = split(pList, ';');

	vector<int> arrID;
	int length = arrString.size();

	for (int i = 0; i < length; i++) {
		if (arrString.at(i).length() > 3) {
			vector<string> arrInfo = split(arrString.at(i), ':');
			if (arrInfo.at(0).length() > 0) {
				arrID.push_back(atoi(arrInfo.at(0).c_str()));
			}
		}
	}

	return arrID;
}

vector<int> LayerCardInGame::getIDFromString_Last(string pList) {
	vector<string> arrString = split(pList, ';');

	vector<int> arrID;
	int length = arrString.size();

	for (int i = 0; i < length; i++) {
		if (arrString.at(i).length() > 3) {
			vector<string> arrInfo = split(arrString.at(i), ':');
			if (arrInfo.at(0).length() > 0) {
				arrID.push_back(atoi(arrInfo.at(arrInfo.size() - 1).c_str()));
			}
		}
	}

	return arrID;
}

vector<int> LayerCardInGame::getIDFromString_ByPos(string pListCard, int pos)
{
	vector<string> arrString = split(pListCard, ';');
	vector<int> arrID;
	int length = arrString.size();

	for (int i = 0; i < length; i++) {
		if (arrString.at(i).length() > 3) {
			vector<string> arrInfo = split(arrString.at(i), ':');
			if (arrInfo.size() == 0)
				continue;
			if (arrInfo.at(0).length() > 0) {
				arrID.push_back(atoi(arrInfo.at(pos).c_str()));
			}
		}
	}

	return arrID;
}
// "id, id, id, id"
vector<int> LayerCardInGame::getIDFromString_TienLen(string pList) {
	vector<string> arrString = split(pList, ',');

	vector<int> arrID;
	int length = arrString.size();

	for (int i = 0; i < length; i++) {
		arrID.push_back(atoi(arrString.at(i).c_str()));
	}

	return arrID;
}

int LayerCardInGame::getCountCardOnHand() {
	return arrCardOnHand.size();
}

int LayerCardInGame::getCountCardTaken() {
	return countCardMe_Take;
}

bool LayerCardInGame::init(){
	if (!Layer::init()) {
		return false;
	}
	mTouchFlag = false;
	paramCallBack = Node::create();
	paramCallBack->retain();
	// scaleApp
	// ---- TouchEvent Start
	this->sizeScreen = Director::getInstance()->getVisibleSize();
	float scaleX = sizeScreen.width / WIDTH_DESIGN;
	float scaleY = sizeScreen.height / HEIGHT_DESIGN;
	float scaleMin = (scaleX < scaleY) ? scaleX : scaleY;
	scaleApp = scaleMin;

	startLeft = (sizeScreen.width - WIDTH_DESIGN) / 2;
	startTop = (sizeScreen.height - winSize.height) / 2;

	this->setAnchorPoint(Point(0, 0));
	this->setPosition(Point(0, 0));

	initGame();
	//for recommend card
	// 	if (isRecommendOn())//check config flag
	// 	{
	setMyTurn(false);
	int gameID = SceneManager::getSingleton().getGameID();
	logicTL = new LogicTienLenMN();
	logicTL->setGameID(gameID);
	//}

	return true;
}

float   LayerCardInGame::getDisPoint(Point p1, Point p2) {
	float dis;

	dis = sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));

	return dis;
}

Point LayerCardInGame::convertPoint(Point pPoint) {
	float xCenter = this->sizeScreen.width / 2;
	float yCenter = this->sizeScreen.height / 2;

	float xTap = pPoint.x;
	float yTap = pPoint.y;

	float x1, x2;
	float y1, y2;

	float A = ((powf(xCenter - xTap, 2) + powf(yCenter - yTap, 2))) / powf(scaleApp, 2);
	float B = powf((yCenter - yTap) / (xCenter - xTap), 2) + 1;

	x1 = xCenter + sqrtf(A / B);
	x2 = xCenter - sqrtf(A / B);

	y1 = yCenter + (yCenter - yTap) * (x1 - xCenter) / (xCenter - xTap);
	y2 = yCenter + (yCenter - yTap) * (x2 - xCenter) / (xCenter - xTap);

	// "điểm cần convert" = A
	// Bởi vì A và Tap nằm ở cùng 1 phía so với Center nên nếu xTap < xCenter thì xA < xCenter và ngược lại
	if ((xTap < xCenter && x1 < xCenter) || (xTap > xCenter && x1 > xCenter)) {
		x1 -= startLeft;
		y1 -= startTop;

		pPoint.x = x1;
		pPoint.y = y1;
	}
	else if ((xTap < xCenter && x2 < xCenter) || (xTap > xCenter && x2 > xCenter)) {
		x2 -= startLeft;
		y2 -= startTop;

		pPoint.x = x2;
		pPoint.y = y2;
	}
	else {
		log("No define POINT CONVERT");
	}

	return pPoint;
}

bool LayerCardInGame::isTouchedCard_Tail(Card *card, Point tap) {

	if (tap.x > card->getPositionX() && tap.x < card->getPositionX() + card->getSize().width &&
		tap.y > card->getPositionY() && tap.y < card->getPositionY() + card->getSize().height) return true;

	return false;
}

bool LayerCardInGame::isTouchedCard_notTail(Card *card, Point tap) {

	if (tap.x > card->getPositionX() && tap.x < card->getPositionX() + disCards &&
		tap.y > card->getPositionY() && tap.y < card->getPositionY() + card->getSize().height) return true;

	return false;
}

void LayerCardInGame::refreshCardOnHand() {
	float leftTheFirst = (WIDTH_DESIGN - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	for (int i = 0; i < arrCardOnHand.size(); i++) {
		Card *card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setRecommend(false);
		card->setClicked(false);
		card->setScaleCardOriginal();
		card->setLocalZOrder(i + 1000);
		card->runAction(MoveTo::create(ANIMATION_DURATION, Vec2(leftTheFirst + i * disCards, topCard)));
	}
}

void LayerCardInGame::refreshCardOnHand(bool isRefreshTop) {
	if (!allowSortCard){
		log("Khong dc xep bai do allowSortCard=false !");
		return;
	}

	// mỗi lần xếp đều "nghỉ" 3s
	//this->allowSortCard = false;
	//this->scheduleOnce(schedule_selector(LayerCardInGame::delayAllowSortCard), 3);

	float leftTheFirst = (winSize.width - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	float top;

	for (int i = 0; i < arrCardOnHand.size(); i++) {
		Card *card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setLocalZOrder(i + 1000);
		card->stopAllActions();
		if (isRefreshTop){
			top = topCard;
			card->setClicked(false);
		}
		else{
			// nếu quân bài đang được click thì top giữ nguyên
			// ngược lại top cần được đặt lại vị trí khởi tạo topCard
			if (card->getClicked())
				top = topCard + sizeCard.height / 4; /// fix cung size_card quan bai
			else
				top = topCard;
		}
		card->runAction(MoveTo::create(REFRESH_DURATION, Vec2(leftTheFirst + i * disCards, top)));

	}
}
void LayerCardInGame::refreshCardOnHandWhenMove(bool isRefreshTop)
{
	float leftTheFirst = (winSize.width - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	float top = this->topCard;

	for (int i = 0; i < arrCardOnHand.size(); i++) {
		Card *card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setLocalZOrder(i + 1000);
		card->stopAllActions();
		if (isRefreshTop){
			top = topCard;
			card->setClicked(false);
		}
		else{
			// nếu quân bài đang được click thì top giữ nguyên
			// ngược lại top cần được đặt lại vị trí khởi tạo topCard
			if (card->getClicked())
				top = topCard + sizeCard.height / 4; /// fix cung size_card quan bai
			else
				top = topCard;

			//card->runAction(MoveTo::create(DELAY_DURATION, Vec2(leftTheFirst + i * disCards, top)));
		}
		card->runAction(MoveTo::create(DELAY_DURATION, Vec2(leftTheFirst + i * disCards, top)));

	}
}
void LayerCardInGame::refreshCardOnHand(float delay) {

	// mỗi lần xếp đều "nghỉ" 3s
	//this->allowSortCard = false;
	//this->scheduleOnce(schedule_selector(LayerCardInGame::delayAllowSortCard), 3);

	// only Deal card
	float leftTheFirst = (winSize.width - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	float top = topCard;

	playSound(S_DEAL);
	log("length arrCardOnHand = %d", arrCardOnHand.size());
	if (arrCardOnHand.size() == 0) return;
	for (int i = 0; i < arrCardOnHand.size(); i++) {
		Card *card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setClicked(false);
		card->setLocalZOrder(i + 1000);
		card->stopAllActions();
		card->setScaleCardOriginal();
		card->runAction(Sequence::create(DelayTime::create(delay * i),
			MoveTo::create(ANIMATION_DURATION, Vec2(leftTheFirst + i * disCards, top)), nullptr));
	}
}

void LayerCardInGame::callbackRefreshCardOnHand() {
	playSound(S_SORT);
}

void LayerCardInGame::setCardClick(Card* card) {

	bool isClick = card->getClicked();
	bool isRecommend = card->getRecommend();
	float pos_y = card->getPositionY();
	float top;
    if (isClick){
		top = topCard;
        if (this->lastLengthChoose > 0)
            this->setLastLengthChoose(this->lastLengthChoose-1);
        this->isRecommend = true;
    }
	else
		top = topCard + sizeCard.height / 4;

	//top = topCard + card->getSize().height / 4;
	if (!isRecommend)// du di sai them 10px
	{
		card->stopAllActions();
	}
	//card->stopAllActions();
	auto actionMove = MoveTo::create(DELAY_DURATION, Vec2(card->getPositionX(), top));
	card->runAction(actionMove);
	//card->setPosition(Vec2(card->getPositionX(), top));
	card->setClicked(!isClick);
    
}

string LayerCardInGame::getURL_byID(int pID) {
	ostringstream oss;
	///id = 12 * (type-1) + num
	int num = (pID % 13) + 1;
	int type = (pID / 13) + 1;
	string url = "card_";
	switch (num) {
	case 11:
		url += "j";
		break;
	case 12:
		url += "q";
		break;
	case 13:
		url += "k";
		break;
	case 1:
		url += "a";
		break;
	default:
		oss << num;
		url += oss.str();
		break;
	}

	url += "_";

	switch (type) {
	case 1:
		url += "h.png"; // cơ
		break;
	case 2:
		url += "d.png"; // rô
		break;
	case 3:
		url += "c.png"; // tép
		break;
	case 4:
		url += "s.png"; // bích
		break;
	default:
		break;
	}
	return url;
}

string LayerCardInGame::getURL_byID_TienLen(int pID) {
	ostringstream oss;

	int num = (pID / 4);
	int type = (pID % 4);

	if (type == 0) {
		num--;
		type = 3;
	}
	else
		type--;
	if (pID == 0) {
		num = 0;
		type = 0;
	}

	string url = "card_";
	switch (num) {
	case 11:
		url += "j";
		break;
	case 12:
		url += "q";
		break;
	case 13:
		url += "k";
		break;
	case 14:
		url += "a";
		break;
	case 15:
		url += "2";
		break;
	default:
		oss << num;
		url += oss.str();
		break;
	}

	url += "_";

	// [0, 1, 2, 3] = ['s', 'c', 'd', 'h']
	switch (type) {
	case 0:
		url += "s.png";
		break;
	case 1:
		url += "c.png";
		break;
	case 2:
		url += "d.png";
		break;
	case 3:
		url += "h.png";
		break;
	}
	return url;
}

vector<int> LayerCardInGame::getIdCardByClicked() {
	vector<int> arrID;

	for (int i = 0; i < arrCardOnHand.size(); i++) {
		Card* card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		if (card->getClicked()) {
			arrID.push_back(card->GetID());
		}
	}
	return arrID;
}

vector<string> LayerCardInGame::getID_HaPhom() {
	vector<string> arrListID;

	vector<int> arrCardClicked = getIdCardByClicked();

	for (int tag = 0; tag < 3; tag++) {
		string list = "";

		for (int i = 0; i < arrCardClicked.size(); i++) {
			int id = arrCardClicked.at(i);

			Card *card = getCardByID(id);
			if (card == NULL) continue;
			if (card->getTag() == tag) {
				if (list.length() != 0) {
					list += ":";
				}

				ostringstream oss;
				oss << id;
				list += oss.str();
			}
		}

		if (list.length() > 0) {
			arrListID.push_back(list);
		}
	}

	for (int i = 0; i < arrListID.size(); i++) {
		log("getID_HaPhom= %s", arrListID.at(i).c_str());
	}

	return arrListID;
}

void LayerCardInGame::setIdCardCanEat(int id) {
	this->idCardCanEat = id;
	Card* card = getCardByID(id);
	if (card == NULL) return;
	card->showArrowEat();
	//add touch event for sprite card can eat
	listener_forcard = EventListenerTouchOneByOne::create();
	listener_forcard->setSwallowTouches(false);
	listener_forcard->onTouchBegan = [](Touch *touch, Event* event){
		auto target = static_cast<Sprite*> (event->getCurrentTarget());
		Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
		Size s = target->getContentSize();
		Rect rect = Rect(0, 0, s.width, s.height);
		if (rect.containsPoint(locationInNode))
		{
			return true;
		}
		return false;
	};
	listener_forcard->onTouchEnded = [=](Touch *touch, Event* event){
		auto target = static_cast<Sprite*> (event->getCurrentTarget());
		auto parent = target->getParent();
		card->removeArrowEat();
		if (mCallBack && mCallBackListener){
			paramCallBack->setTag(2);
			(mCallBack->*mCallBackListener)(paramCallBack);
			_eventDispatcher->removeEventListenersForTarget(card, true);
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener_forcard, card);
	//card->showArrowEat();
}

int LayerCardInGame::getIdCardCanEat() {
	return this->idCardCanEat;
}

void LayerCardInGame::setGameStarted(bool isStarted){
	this->gameStarted = isStarted;
}

bool LayerCardInGame::getGameStarted() {
	return this->gameStarted;
}

bool LayerCardInGame::getAllowSortCard() {
	return this->allowSortCard;
}

void LayerCardInGame::setAllowSortCard(bool isAllow) {
	this->allowSortCard = isAllow;
}

void LayerCardInGame::initGame() {
	//if (arrAllCard == NULL) {
	//
	sizeAdd = ChanUtils::getSizePos();//cai nay de da man hinh + them 1 khoang x hoac y
	ZORDER_PHOM = 0;
	ZORDER_TAKE = 31;
	ZORDER_HAND = 40;

	// 	AVATAR_L_SPACE = 10 + sizeAdd.width;
	// 	AVATAR_T_SPACE = 10 + sizeAdd.height;
	// 	W_AVATAR = 143 + sizeAdd.height;
	winSize = Director::getInstance()->getVisibleSize();

	Card *card_back = new Card("card_back.png");
	card_back->autorelease();
	sizeCard = card_back->getSize();
	card_back->setAnchorPoint(Point(0, 0));
	card_back->setPosition(getPointCardInCenter());
	card_back->SetID(-1);
	card_back->setLocalZOrder(1000);
	card_back->setVisible(false);
	arrAllCard.push_back(card_back);

	this->addChild(card_back);


	topCard = 190 + sizeAdd.height;
	disCards = 120;

	isClickCard = true;
	gameStarted = false;
	allowSortCard = true;
	requestRecommendHaPhom = false;

	this->lcRecommendHaPhom = "";

	// count
	countCardMe_Take = 0;
	countCardLeft_Take = 0;
	countCardRight_Take = 0;
	countCardTop_Take = 0;
	countCardBot_Take = 0;
	//
	countCardMe_Eat = 0;
	countCardLeft_Eat = 0;
	countCardRight_Eat = 0;
	countCardTop_Eat = 0;
	countCardBot_Eat = 0;
	//
	countCardMe_Phom = 0;
	countCardLeft_Phom = 0;
	countCardRight_Phom = 0;
	countCardTop_Phom = 0;
	countCardBot_Phom = 0;
	//
	countMe_Phom = 0;

	//danh cho tien len, hieu ung danh bai
	arrHitCardOnTable.clear();
	///khoi tao cac vi tri cac trang thai user
	/*
		left->setPosition(Vec2(AVATAR_L_SPACE + W_AVATAR + sizeAdd.width, HEIGHT_DESIGN / 2 + sizeAdd.height));
		right->setPosition(Vec2(WIDTH_DESIGN - AVATAR_L_SPACE - W_AVATAR - sizeAdd.width, HEIGHT_DESIGN / 2 + sizeAdd.height));
		top->setPosition(Vec2(WIDTH_DESIGN / 2, HEIGHT_DESIGN - W_AVATAR + sizeAdd.height));
		bottom->setPosition(Vec2(WIDTH_DESIGN / 2 + sizeAdd.width, 200 + sizeAdd.height));
		me->setPosition(Vec2(W_AVATAR + AVATAR_L_SPACE + sizeAdd.width, 200 + sizeAdd.height));
		*/
	//user me
	int distance_avatar = 200;
	int add_size = 5;
	int add_size_hit = 10;
	startHitBot = Vec2(winSize.width / 2 - sizeCard.width / 1.5, winSize.height / 2 - sizeCard.height + SIZE_ADD - 10);
	startEatBot = Vec2(winSize.width / 2 - W_AVATAR / 2 - sizeCard.width, distance_avatar + sizeAdd.height / 2 - sizeCard.height / 8 + SIZE_ADD);
	///200 + sizeAdd.height
	startHandoffBot = Vec2(winSize.width / 2 - W_AVATAR - sizeCard.width, distance_avatar + sizeAdd.height + SIZE_ADD / 4);
	startHandoffMe = Vec2(W_AVATAR + AVATAR_L_SPACE + sizeAdd.width + sizeCard.width, topCard + sizeCard.height / 2 + 10 + SIZE_ADD / 4);

	startHitTop = Vec2(winSize.width / 2 - sizeCard.width / 1.5, winSize.height / 2 + W_AVATAR);
	startEatTop = Vec2(winSize.width / 2 - W_AVATAR / 2 - sizeCard.width - AVATAR_T_SPACE,
		HEIGHT_DESIGN - W_AVATAR + sizeAdd.height - sizeCard.height / 4);
	startHandoffTop = Vec2(winSize.width / 2 - W_AVATAR - sizeCard.width, winSize.height / 2 + W_AVATAR);

	startHitRight = Vec2(winSize.width - W_AVATAR - AVATAR_L_SPACE * 8 - add_size_hit, winSize.height / 2 + add_size);
	startEatRight = Vec2(WIDTH_DESIGN - AVATAR_L_SPACE - W_AVATAR - sizeAdd.width,
		winSize.height / 2 + sizeCard.height / 4 + SIZE_ADD - 25);
	startHandoffRight = Vec2(winSize.width - AVATAR_L_SPACE * 8 - W_AVATAR - add_size_hit, winSize.height / 2 + add_size);

	startHitLeft = Vec2(AVATAR_L_SPACE * 8 + W_AVATAR + add_size_hit, winSize.height / 2 + add_size);
	startEatLeft = Vec2(W_AVATAR + sizeAdd.width - sizeCard.width / 4 - AVATAR_L_SPACE, winSize.height / 2 + sizeCard.height / 4 + SIZE_ADD - 25);
	startHandoffLeft = Vec2(AVATAR_L_SPACE * 8 + W_AVATAR + add_size_hit, winSize.height / 2 + add_size);
}

void LayerCardInGame::resetGame() {
	// ẩn đi các quân bài trên bàn
	Card *card;
	for (int i = 0; i < arrIDCardOnTable.size(); i++)
	{
		card = getCardByID(arrIDCardOnTable.at(i));
		if (card == NULL) continue;
		card->setVisible(false);
	}

	// card
	arrCardOnHand.clear();
	arrIDCardOnHandLeft.clear();
	arrIDCardOnHandRight.clear();
	arrIDCardOnHandTop.clear();
	arrIDCardOnHandBot.clear();
	arrIDCardOnTable.clear();
	arrHitCardMe.clear();

	int length = arrAllCard.size();

	for (int i = 0; i < length; i++) {
		Card *card = arrAllCard.at(i);
		if (card == NULL) continue;
		card->resetStatus();
		card->setLocalZOrder(0);
		card->setPosition(getPointCardInCenter());
	}

	gameStarted = false;
	requestRecommendHaPhom = false;
	lcRecommendHaPhom = "";

	this->allowSortCard = true;
	// count
	countCardMe_Take = 0;
	countCardLeft_Take = 0;
	countCardRight_Take = 0;
	countCardTop_Take = 0;
	countCardBot_Take = 0;
	//
	countCardMe_Eat = 0;
	countCardLeft_Eat = 0;
	countCardRight_Eat = 0;
	countCardTop_Eat = 0;
	countCardBot_Eat = 0;
	//
	countCardMe_Phom = 0;
	countCardLeft_Phom = 0;
	countCardRight_Phom = 0;
	countCardTop_Phom = 0;
	countCardBot_Phom = 0;
	//
	countMe_Phom = 0;
	if (cardBackLeft != NULL){
		SCALE_RATIO = 1.2;
		cardBackLeft->setVisible(false);
		cardBackRight->setVisible(false);
		cardBackTop->setVisible(false);
		cardBackBot->setVisible(false);
	}
	setMyTurn(false);
	lastLengthChoose = 0;
}

void LayerCardInGame::playSound(int type)
{
	string soundPath;
	switch (type)
	{
	case S_EAT_CARD://an bai
		soundPath = "an_bai.mp3";
		break;
	case S_UPDATE_LIST: //co nguoi moi vao - update list user
		soundPath = "ring_ring.mp3";
		break;
	case S_START_GAME:
		soundPath = "start_game.mp3"; //bat dau game
		break;
	case S_END_GAME:
		soundPath = "end_game.mp3";//ket thuc game
		break;
	case S_WIN:
		soundPath = "thang_roi_1.mp3";//hien thi thang thua
		break;
	case S_HIT_CARD:
		soundPath = "thach_an_1.mp3"; ///danh bai
		break;
	case S_GIVE_CARD:
		soundPath = "boc_bai.mp3";//boc bai
		break;
	case S_HAND_OFF:
		soundPath = "ha_phom.mp3";//ha phom
		break;
	case S_U:
		soundPath = "u_1.mp3";//u
		break;
	case S_DEAL:
		soundPath = "chia_bai.mp3";
		break;
	default:
		break;
	}
	if (mUtils::isSoundOn())
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(soundPath.c_str());
}

void LayerCardInGame::initCardByPhom() {
	// init card
	SCALE_RATIO = 0.9f;
	for (int i = 0; i < 52; i++) {
		Card *card = new Card(getURL_byID(i).c_str());
		card->autorelease();
		card->SetID(i);
		card->setAnchorPoint(Point(0, 0));
		card->setPosition(getPointCardInCenter());

		this->addChild(card);
		arrAllCard.push_back(card);
	}
}

void LayerCardInGame::initCardByTienLen() {
	SCALE_RATIO = 1.2f;
	for (int id = 13; id < 52 + 13; id++) {
		Card *card = new Card(getURL_byID_TienLen(id).c_str());
		card->autorelease();
		card->SetID(id);
		//card->setURL(getURL_byID_TienLen(id));
		card->setAnchorPoint(Point(0, 0));
		card->setPosition(getPointCardInCenter());
		this->addChild(card);
		arrAllCard.push_back(card);
	}
	Color3B colorCountCard = Color3B::WHITE;
	string imgPath = "sl-card-back.png";
	Vec2 pointCenter = Vec2(32, 39);
	if (SceneManager::getSingleton().getGameVersion() == GameVersion::p2)
	{
		colorCountCard = Color3B::BLACK;
		imgPath = "groupCard.png";
		pointCenter = Vec2(47, 55);
	}

	// các quân bài tượng trưng
	cardBackLeft = Sprite::create(imgPath);
	cardBackRight = Sprite::create(imgPath);
	cardBackTop = Sprite::create(imgPath);
	cardBackBot = Sprite::create(imgPath);

	cardBackLeft->setVisible(false);
	cardBackRight->setVisible(false);
	cardBackTop->setVisible(false);
	cardBackBot->setVisible(false);

	cardBackLeft->setAnchorPoint(Point(0, 0));
	cardBackLeft->setPosition(Vec2(startHitLeft.x, startHitLeft.y));
	cardBackLeft->setLocalZOrder(100);

	cardBackRight->setAnchorPoint(Point(0, 0));
	cardBackRight->setPosition(Vec2(startHitRight.x - sizeCard.width / 2,
		startHitRight.y));
	cardBackRight->setLocalZOrder(100);

	cardBackTop->setAnchorPoint(Point(0, 0));
	cardBackTop->setPosition(Vec2(startEatTop.x, startEatTop.y));
	cardBackTop->setLocalZOrder(100);

	cardBackBot->setAnchorPoint(Point(0, 0));
	cardBackBot->setPosition(Vec2(startEatBot.x, startEatBot.y));
	cardBackBot->setLocalZOrder(100);
	//
	this->addChild(cardBackLeft);
	this->addChild(cardBackRight);
	this->addChild(cardBackTop);
	this->addChild(cardBackBot);

	//
	// hien thi so quan bai dang co tren tay
	Label *countCardLeft = Label::createWithTTF("0", kNormalFont, FONT_SIZE_NORMAL);
	Label *countCardRight = Label::createWithTTF("0", kNormalFont, FONT_SIZE_NORMAL);
	Label *countCardTop = Label::createWithTTF("0", kNormalFont, FONT_SIZE_NORMAL);
	Label *countCardBot = Label::createWithTTF("0", kNormalFont, FONT_SIZE_NORMAL);

	countCardLeft->setAnchorPoint(Point(0.5, 0.5));
	countCardRight->setAnchorPoint(Point(0.5, 0.5));
	countCardTop->setAnchorPoint(Point(0.5, 0.5));
	countCardBot->setAnchorPoint(Point(0.5, 0.5));

	countCardLeft->setColor(colorCountCard);
	countCardRight->setColor(colorCountCard);
	countCardTop->setColor(colorCountCard);
	countCardBot->setColor(colorCountCard);



	countCardLeft->setPosition(pointCenter);
	countCardRight->setPosition(pointCenter);
	countCardTop->setPosition(pointCenter);
	countCardBot->setPosition(pointCenter);

	countCardLeft->setTag(kTagCountCard);
	countCardRight->setTag(kTagCountCard);
	countCardTop->setTag(kTagCountCard);
	countCardBot->setTag(kTagCountCard);

	cardBackLeft->addChild(countCardLeft);
	cardBackRight->addChild(countCardRight);
	cardBackTop->addChild(countCardTop);
	cardBackBot->addChild(countCardBot);

}

void LayerCardInGame::actionDealCard(vector<int> arrCardID, vector<int> arrCardEat) {
	arrCardOnHand.clear();
	getCardByID(-1)->setVisible(false);
	string _arrID;
	for (int i = 0; i < arrCardID.size(); i++) {
		int id = arrCardID.at(i);
		Card *card = getCardByID(id);
		if (card == NULL) continue;
		_arrID += StringUtils::format("%d", id) + ";";
		card->setVisible(true);
		card->stopAllActions();
		if (arrCardID.size() == arrCardEat.size() && arrCardEat.at(i) == 1)
			card->setCardAte();
		arrCardOnHand.push_back(card);
	}
	//actionDealCardPro(arrCardOnHand.size());
	//return;

	refreshCardOnHand(DELAY_DURATION);

	//fix loi chia bai bi an 1 quan
	for (Card *card : arrCardOnHand)
	{
		card->setVisible(true);
	}
}

void LayerCardInGame::actionDealCardTL(vector<int> arrCardID, vector<int> arrCardEat)
{
	// only Deal card
	if (arrCardID.size() == 0) return;
	float leftTheFirst = (WIDTH_DESIGN - (arrCardID.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	float top = topCard;
	arrCardOnHand.clear();
	getCardByID(-1)->setVisible(false);
	for (int i = 0; i < arrCardID.size(); i++) {
		int id = arrCardID.at(i);
		Card *card = getCardByID(id);
		if (card == NULL) continue;
		card->setVisible(true);
		card->setClicked(false);
		card->setLocalZOrder(i + 1000);
		card->setPosition(Vec2(leftTheFirst + i * disCards, top));
		if (arrCardID.size() == arrCardEat.size() && arrCardEat.at(i) == 1)
			card->setCardAte();
		arrCardOnHand.push_back(card);
	}
}

void LayerCardInGame::actionSortCard(vector<int> arrCardID) {
	// tạm thôi!

	int length = arrCardID.size();

	Card *card;
	for (int i = 0; i < arrCardOnHand.size(); i++)
	{
		card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setVisible(false);
	}
	arrCardOnHand.clear();

	for (int i = 0; i < length; i++) {
		int id = arrCardID.at(i);
		card = getCardByID(id);
		if (card == NULL) continue;;
		card->setVisible(true);
		arrCardOnHand.push_back(card);
	}
	//bool isRefreshTop = false;
	refreshCardOnHand(true);
}

void LayerCardInGame::actionSortCardByTienLen() {
	lastLengthChoose = 0;
	int length = arrCardOnHand.size();
	vector<int> arrIDCard;
	Card *card;
	int i, j, tg;
	for (i = 0; i < length; i++) {
		card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		arrIDCard.push_back(card->GetID());
	}

	length = arrIDCard.size();
	if (length < 2) return;
	for (i = 0; i < length - 1; i++) {
		for (j = i + 1; j < length; j++)
		{
			if (arrIDCard.at(i) > arrIDCard.at(j)) {
				tg = arrIDCard.at(i);
				arrIDCard.at(i) = arrIDCard.at(j);
				arrIDCard.at(j) = tg;
			}
		}
	}
	length = arrIDCard.size();

	for (int i = 0; i < arrCardOnHand.size(); i++)
	{
		card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setVisible(false);
	}
	arrCardOnHand.clear();

	for (int i = 0; i < length; i++) {
		int id = arrIDCard.at(i);
		card = getCardByID(id);
		if (card == NULL) continue;;
		card->setVisible(true);
		arrCardOnHand.push_back(card);
	}

	refreshCardOnHand(true);

}

void LayerCardInGame::delayAllowSortCard(float dt) {
	this->allowSortCard = true;
}

void LayerCardInGame::actionHitCard(int kUser, int id) {
	int length = arrAllCard.size();
	if (length > 0){
		Point pointEnd;
		Card *card = getCardByID(id);
		if (card == NULL) return;

		bool isHit = true;
		switch (kUser) {
		case kUserMe:
		{
						removeCardByID(id);

						card->stopAllActions();
						card->runAction(Spawn::createWithTwoActions(ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO),
							MoveTo::create(ANIMATION_DURATION,
							Vec2(startHitBot.x + (disCards / 2) * countCardMe_Take, startHitBot.y))));
						card->setLocalZOrder(countCardMe_Take);

						card->setFromUser(kUserMe);
						refreshCardOnHand(true);
						countCardMe_Take++;
						arrHitCardMe.push_back(id);
		}
			break;
		case kUserLeft:
		{
						  if (!card->isVisible()){
							  card->setVisible(true);
							  card->setPosition(getStartPositionCardUserLeft_Hit());
						  }
						  card->setFromUser(kUserLeft);
						  card->stopAllActions();
						  //card->setLocalZOrder(ZORDER_TAKE + countCardLeft_Take);
						  pointEnd.setPoint(startHitLeft.x + (disCards / 2)* (countCardLeft_Take % 5),
							  startHitLeft.y - (countCardLeft_Take / 5) * (sizeCard.height / 3));
						  card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, pointEnd),
							  ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));
						  card->setLocalZOrder(countCardLeft_Take + ZORDER_TAKE);

						  countCardLeft_Take++;
		}
			break;
		case kUserRight:
		{
						   if (!card->isVisible()) {
							   card->setVisible(true);
							   card->setPosition(getStartPositionCardUserRight_Hit());
						   }
						   card->setFromUser(kUserRight);
						   card->stopAllActions();
						   // card->setLocalZOrder(ZORDER_TAKE + countCardRight_Take);
						   pointEnd.setPoint(startHitRight.x - (3 * (disCards / 2) + sizeCard.width / 2) + (disCards / 2) * (countCardRight_Take % 5),
							   startHitRight.y - (countCardRight_Take / 5) * (sizeCard.height / 3));
						   card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, pointEnd),
							   ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));

						   card->setLocalZOrder(countCardRight_Take + ZORDER_TAKE);
						   countCardRight_Take++;
		}
			break;
		case kUserTop:
		{
						 if (!card->isVisible()) {
							 card->setVisible(true);
							 card->setPosition(getStartPositionCardUserTop_Hit());
						 }
						 card->setFromUser(kUserTop);
						 card->stopAllActions();
						 //card->setLocalZOrder(ZORDER_TAKE + countCardTop_Take);
						 pointEnd.setPoint(startHitTop.x + (disCards / 2) * (countCardTop_Take % 5),
							 startHitTop.y - (countCardTop_Take / 5) * (sizeCard.height / 3));
						 card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, pointEnd),
							 ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));

						 card->setLocalZOrder(countCardTop_Take + ZORDER_TAKE);
						 countCardTop_Take++;
		}
			break;
		case kUserBot:
		{
						 if (!card->isVisible()) {
							 card->setVisible(true);
							 card->setPosition(getStartPositionCardUserBot_Hit());
						 }
						 card->setFromUser(kUserBot);
						 //card->setLocalZOrder(ZORDER_TAKE + countCardBot_Take);
						 card->stopAllActions();
						 pointEnd.setPoint(startHitBot.x + (disCards / 2) * (countCardBot_Take % 5),
							 startHitBot.y - (countCardBot_Take / 5)*(sizeCard.height / 3));
						 // 			card->runAction(MoveTo::create(ANIMATION_DURATION, pointEnd));
						 // 			card->runAction(ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO));
						 card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, pointEnd),
							 ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));

						 card->setLocalZOrder(countCardBot_Take + ZORDER_TAKE);

						 countCardBot_Take++;
		}

			break;
		default:
			isHit = false;
			break;
		}

		if (isHit) arrIDCardOnTable.push_back(id);
	}
}

void LayerCardInGame::actionHitCardByTienLen(int posUser, vector<int> arrID) {
	int length = arrID.size();
	float toLeft = WIDTH_DESIGN / 2 - (sizeCard.width / 2 * length) / 2;
	float toTop = getPointCardInCenter().y;

	bool isMe = false;
	for (int i = 0; i < arrHitCardOnTable.size(); ++i){
		Card *card = arrHitCardOnTable.at(i);
		if (card == NULL) continue;
		if (card->getTypePos() < 2){
			card->runAction(MoveBy::create(ANIMATION_DURATION, Vec2(0, sizeCard.height*0.5)));
		}
		else{
			card->setVisible(false);
		}
		int dem = card->getTypePos() + 1;
		card->setTypePos(dem);
		card->setLocalZOrder(-dem);
	}

	for (int i = 0; i < length; i++) {
		int id = arrID.at(i);
		Card *card = getCardByID(id);
		if (card == NULL) continue;

		if (posUser == kUserMe) {
			removeCardByID(id);
			isMe = true;
		}

		if (!card->isVisible()) {
			card->setPosition(getStartPositionCardByPos_TienLen(posUser));
			card->setVisible(true);
		}
		int rdAngle = rand() % 5;
		//card->stopAllActions();
		// 		card->runAction(RotateTo::create(ANIMATION_DURATION, rdAngle));
		// 		card->runAction(MoveTo::create(ANIMATION_DURATION, Vec2(toLeft + i * sizeCard.width / 2, toTop)));
		// 		card->runAction(ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO));
		card->runAction(Sequence::create(RotateTo::create(DELAY_DURATION, rdAngle),
			MoveTo::create(ANIMATION_DURATION, Vec2(toLeft + i * sizeCard.width / 2, toTop)),
			CallFunc::create([=]{
			card->setZOrder(card->getPositionX());
		}),
			ScaleTo::create(DELAY_DURATION, SCALE_RATIO), nullptr));
		// 		int zOder = card->getPositionX();
		// 		card->setLocalZOrder(1);
		card->setTypePos(0);
		arrHitCardOnTable.push_back(card);
	}
	//cheat for hit card
	// 	for (Card *card : arrHitCardOnTable)
	// 	{
	// 		if (card->getTypePos() == 0)
	// 			card->setLocalZOrder(0);
	// 	}
	if (isMe) {
		log("LENGTH ARRCARD ON HAND = %d", arrCardOnHand.size());
		refreshCardOnHand(true);
		//this->actionSortCardByTienLen();
	}
}

void LayerCardInGame::actionGiveCard(int kUser, int id) {
	int length = arrAllCard.size();
	if (id >= length) id = -1;

	Card *card = getCardByID(id);
	if (card == NULL) return;
	card->resetStatus();
	if (kUser != kUserMe)
	{
		card->setPosition(getPointCardInCenter());
		card->setVisible(true);
	}
	switch (kUser) {
	case kUserMe:
	{
					arrCardOnHand.push_back(card);
					float rightThefirst = (WIDTH_DESIGN - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2;
					Card *cardBack = getCardByID(-1);
					if (cardBack != NULL)
					{
						cardBack->setPosition(getPointCardInCenter());
						cardBack->setVisible(true);
						cardBack->runAction(Sequence::create(MoveTo::create(ANIMATION_DURATION,
							Vec2(rightThefirst, topCard)),
							CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callbackGiveCard, this)), NULL));
					}
					card->setPosition(Vec2(rightThefirst, topCard));
					card->setVisible(true);
					refreshCardOnHand();
	}
		break;
	case kUserLeft:
		card->stopAllActions();
		card->runAction(Sequence::create(MoveTo::create(ANIMATION_DURATION, getStartPositionCardUserLeft_Hit())
			, CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callbackGiveCard, this)), NULL));

		break;
	case kUserRight:

		//             card->runAction(Sequence::create(MoveTo::create(0.3, getStartPositionCardUserRight_Hit()), DelayTime::create(SCALE_RATIO), ScaleTo(this, callfuncN_selector(LayerCardInGame::callbackGiveCard)), NULL));
		//             card->runAction(ScaleTo::create(0.3, SCALE_RATIO));
		card->stopAllActions();
		card->runAction(Sequence::create(MoveTo::create(ANIMATION_DURATION, getStartPositionCardUserRight_Hit())
			, CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callbackGiveCard, this)), NULL));
		break;
	case kUserTop:

		//             card->runAction(Sequence::create(MoveTo::create(0.3, getStartPositionCardUserTop_Hit()), DelayTime::create(SCALE_RATIO), ScaleTo(this, callfuncN_selector(LayerCardInGame::callbackGiveCard)), NULL));
		//             card->runAction(ScaleTo::create(0.3, SCALE_RATIO));
		card->stopAllActions();
		card->runAction(Sequence::create(MoveTo::create(ANIMATION_DURATION, getStartPositionCardUserTop_Hit())
			, CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callbackGiveCard, this)), NULL));

		break;
	case kUserBot:

		//             card->runAction(Sequence::create(MoveTo::create(0.3, getStartPositionCardUserBot_Hit()), DelayTime::create(SCALE_RATIO), ScaleTo(this, callfuncN_selector(LayerCardInGame::callbackGiveCard)), NULL));
		//             card->runAction(ScaleTo::create(0.3, SCALE_RATIO));
		card->stopAllActions();
		card->runAction(Sequence::create(MoveTo::create(ANIMATION_DURATION, getStartPositionCardUserBot_Hit())
			, CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callbackGiveCard, this)), NULL));

		break;

	default:
		break;
	}

	playSound(S_GIVE_CARD);
}

void LayerCardInGame::callbackGiveCard(Node *pSender) {

	//     card->setVisible(false);
	pSender->setVisible(false);
	Card *card = getCardByID(-1);
	if (card == NULL) return;
	card->setVisible(false);
}

void LayerCardInGame::actionEatCard(int fromPosUser, int toPosUser, int pId) {
	int length = arrAllCard.size();
	if (pId >= 100) pId = length - 1;

	if (arrIDCardOnTable.size() > 0 && pId == arrIDCardOnTable.at(arrIDCardOnTable.size() - 1))
		arrIDCardOnTable.pop_back();

	Card *card = getCardByID(pId);
	if (card == NULL) return;
	card->setCardAte();

	switch (fromPosUser) {
	case kUserMe:
		countCardMe_Take--;
		break;
	case kUserLeft:
		countCardLeft_Take--;
		break;
	case kUserRight:
		countCardRight_Take--;
		break;
	case kUserTop:
		countCardTop_Take--;
		break;
	case kUserBot:
		countCardBot_Take--;
		break;
	default:
		break;
	}

	switch (toPosUser) {
	case kUserMe:
		arrCardOnHand.push_back(card);
		card->setRecommend(true);
		card->setScaleCardOriginal();
		refreshCardOnHand();
		countCardMe_Eat++;
		break;

	case kUserLeft:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserLeft_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->stopAllActions();
		card->runAction(MoveTo::create(ANIMATION_DURATION,
			Vec2(startEatLeft.x + disCards / 2 * countCardLeft_Eat, startEatLeft.y)));
		card->setLocalZOrder(countCardLeft_Eat);

		countCardLeft_Eat++;

		break;

	case kUserRight:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserRight_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->stopAllActions();
		card->runAction(MoveTo::create(ANIMATION_DURATION,
			Vec2(startEatRight.x - disCards / 2 * countCardRight_Eat, startEatRight.y)));
		card->setLocalZOrder(-countCardRight_Eat);

		countCardRight_Eat++;

		break;

	case kUserTop:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserTop_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->stopAllActions();
		card->runAction(MoveTo::create(ANIMATION_DURATION,
			Vec2(startEatTop.x - disCards / 2 * countCardTop_Eat, startEatTop.y)));
		card->setLocalZOrder(-countCardTop_Eat);

		countCardTop_Eat++;

		break;

	case kUserBot:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserBot_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->stopAllActions();
		card->runAction(MoveTo::create(ANIMATION_DURATION,
			Vec2(startEatBot.x - disCards / 2 * countCardLeft_Eat, startEatBot.y)));
		card->setLocalZOrder(-countCardBot_Eat);

		countCardBot_Eat++;

		break;

	default:
		break;
	}


}

void LayerCardInGame::showEatCard(int posUser, int pId)
{
	int length = arrAllCard.size();
	if (pId >= 100) pId = length - 1;

	if (arrIDCardOnTable.size() > 0 && pId == arrIDCardOnTable.at(arrIDCardOnTable.size() - 1))
		arrIDCardOnTable.pop_back();

	Card *card = getCardByID(pId);
	if (card == NULL) return;
	card->setCardAte();
	log("Nhay vao show eat card");

	switch (posUser) {
	case kUserMe:

		card->setScaleCardOriginal();
		arrCardOnHand.push_back(card);
		refreshCardOnHand();

		countCardMe_Eat++;

		break;

	case kUserLeft:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserLeft_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->setPosition(Vec2(startEatLeft.x + disCards / 2 * countCardLeft_Eat, startEatLeft.y));
		card->setLocalZOrder(countCardLeft_Eat);

		countCardLeft_Eat++;

		break;

	case kUserRight:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserRight_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->setPosition(Vec2(startEatRight.x - disCards / 2 * countCardRight_Eat, startEatRight.y));
		card->setLocalZOrder(-countCardRight_Eat);

		countCardRight_Eat++;

		break;

	case kUserTop:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserTop_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->setPosition(Vec2(startEatTop.x - disCards / 2 * countCardTop_Eat, startEatTop.y));
		card->setLocalZOrder(-countCardTop_Eat);
		countCardTop_Eat++;

		break;

	case kUserBot:

		if (!card->isVisible()) {
			card->setVisible(true);
			card->setPosition(getStartPositionCardUserBot_Hit());
		}

		card->setScale(SCALE_RATIO);
		card->setPosition(Vec2(startEatBot.x - disCards / 2 * countCardBot_Eat, startEatBot.y));
		card->setLocalZOrder(-countCardBot_Eat);

		countCardBot_Eat++;

		break;

	default:
		break;
	}
}
void LayerCardInGame::actionTransferedCard(int fromPosUser, int toPosUser, int pId) {
	int length = arrAllCard.size();
	if (pId >= length) pId = length - 1;

	Card *card = getCardByID(pId);
	if (card == NULL) return;
	card->setVisible(true);
	switch (fromPosUser) {
	case kUserMe:
		countCardMe_Take--;
		break;

	case kUserLeft:
		countCardLeft_Take--;
		break;

	case kUserRight:
		countCardRight_Take--;
		break;

	case kUserTop:
		countCardTop_Take--;
		break;

	case kUserBot:
		countCardBot_Take--;
		break;

	default:
		break;
	}

	switch (toPosUser) {
	case kUserMe:
	{
					card->stopAllActions();
					card->setLocalZOrder(countCardMe_Take);
					card->runAction(MoveTo::create(ANIMATION_DURATION, Vec2(startHitBot.x + disCards / 2 * countCardMe_Take, startHitBot.y)));
					countCardMe_Take++;
	}
		break;

	case kUserLeft: case kUserRight: case kUserTop: case kUserBot:

		actionHitCard(toPosUser, pId);

		break;

	default:
		break;
	}
}

void LayerCardInGame::actionPushCard(int fromPosUser, int toPosUser, int pId) {
	int length = arrAllCard.size();
	if (pId >= length) pId = length - 1;
	log("actionPushCard");
	Card *card = getCardByID(pId);
	if (card == NULL) return;
	switch (fromPosUser) {
	case kUserMe:
	{
					removeCardByID(pId);
					refreshCardOnHand();
					//refreshCardOnHandWhenMove(true);
	}
		break;
	case kUserLeft:
		if (!card->isVisible())
			card->setPosition(getStartPositionCardUserLeft_Hit());
		break;

	case kUserRight:
		if (!card->isVisible())
			card->setPosition(getStartPositionCardUserRight_Hit());
		break;

	case kUserTop:
		if (!card->isVisible())
			card->setPosition(getStartPositionCardUserTop_Hit());
		break;

	case kUserBot:
		if (!card->isVisible())
			card->setPosition(getStartPositionCardUserBot_Hit());
		break;

	default:
		break;
	}
	card->setVisible(true);
}

void LayerCardInGame::actionRecommendCard() {
	//this->allowSortCard = true;
	//log("LIST RECOMMEND CARD %s va size : %d", lcRecommendHaPhom.c_str(), lcRecommendHaPhom.size());

    if (this->isRecommend) return;
    
	if (SceneManager::getSingleton().getGameVersion() == GameVersion::p2)
		return;

	if (lcRecommendHaPhom.length() <= 2) {
		return;
	}
	if (mCallBackListener && mCallBack){
		paramCallBack->setTag(0);
		(mCallBack->*mCallBackListener)(paramCallBack);
	}
	vector<string> listID; // moi ptu co dang id:id:id
	listID = split(lcRecommendHaPhom, ';');
	log("list recommend = %s", lcRecommendHaPhom.c_str());

	for (int i = 0; i < listID.size(); i++) {
		string list = listID.at(i);

		if (list.length() > 1) {
			vector<string> arrID = split(list, ':');

			for (int j = 0; j < arrID.size(); j++) {
				int id = (atoi)(arrID.at(j).c_str());

				Card *card = getCardByID(id);
				if (card == NULL) continue;
				//card->setRecommend(true);
				setCardClick(card);
				card->setTag(i);
			}
		}
	}
	refreshCardOnHand(false);
	this->requestRecommendHaPhom = false;
	this->lcRecommendHaPhom = "";
}

void LayerCardInGame::actionHaPhom(vector<int> arrID, vector<int> arrIDPhom) {
	//danh cho me
	if (arrID.size() != arrIDPhom.size()) return;

	int dem = 0;
	int currentPhom = 0;
	for (int i = 0; i < arrID.size(); i++) {
		int id = arrID.at(i);
		int orderPhom = arrIDPhom.at(i);
		if (currentPhom != orderPhom){
			currentPhom = orderPhom;
			dem = 0;
		}

		Card *card = getCardByID(id);
		if (card == NULL) continue;
		removeCardByID(id);
		card->setPhom(true);
		card->stopAllActions();
		int left = startHandoffMe.x + (dem *disCards) / 2;
		int top = startHandoffMe.y - (orderPhom - 1) * sizeCard.height / 3;
		card->setLocalZOrder(ZORDER_PHOM + ((dem + 1) * currentPhom) + 1000);
		card->setVisible(true);
		card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, Vec2(left, top)),
			ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));
		dem++;
	}

}
void LayerCardInGame::actionHaPhomClient(string listCardID)
{
	int dem = 0;
	int currentPhom = 0;
	vector<string> arrIDPhom = mUtils::splitString(listCardID, ';');
	for (int j = 0; j < arrIDPhom.size(); ++j){
		vector<string> arrID = mUtils::splitString(arrIDPhom.at(j), ':');
		for (int i = 0; i < arrID.size(); i++) {
			int id = atoi(arrID.at(i).c_str());
			int orderPhom = j;
			if (currentPhom != orderPhom){
				currentPhom = orderPhom;
				dem = 0;
			}

			Card *card = getCardByID(id);
			if (card == NULL) continue;
			removeCardByID(id);
			card->setPhom(true);
			card->stopAllActions();
			//startHandoffBot = Vec2(WIDTH_DESIGN / 2 - W_AVATAR - sizeCard.width, topCard);
			int left = startHandoffMe.x + (dem *disCards) / 2;
			int top = startHandoffMe.y - (orderPhom - 1) * sizeCard.height / 3;
			card->setLocalZOrder(ZORDER_PHOM + ((dem + 1) * currentPhom) + 1000);
			card->setVisible(true);
			card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, Vec2(left, top)),
				ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));
			dem++;

		}
	}
	refreshCardOnHand(true);
}
void LayerCardInGame::actionHaPhomByPos(int pos, vector<int> arrID, vector<int> arrIDPhom) {
	log("ha phom kieu moi! arrID.size()=%d, arrIDPhom.size()=%d Pos = %d",
		arrID.size(), arrIDPhom.size(), pos);
	if (arrID.size() != arrIDPhom.size()) return;

	int dem = 0;
	int currentPhom = 1;
	int zOrder = 0;
	Vec2 pointStart;
	Vec2 pointEnd;

	for (int i = 0; i < arrID.size(); i++) {
		int id = arrID.at(i);
		int orderPhom = arrIDPhom.at(i);
		if (currentPhom != orderPhom){
			currentPhom = orderPhom;
			dem = 0;
		}

		Card *card = getCardByID(id);
		if (card == NULL) continue;
		card->setPhom(true);

		switch (pos) {
		case kUserLeft:
			pointStart = getStartPositionCardUserLeft_Hit();
			pointEnd = Vec2(startHandoffLeft.x + dem * disCards / 2,
				startHandoffLeft.y + currentPhom * sizeCard.height / 3);
			zOrder = (dem + 1)*(4 - currentPhom);
			break;

		case kUserRight:
			pointStart = getStartPositionCardUserRight_Hit();
			pointEnd = Vec2(startHandoffRight.x - ((dem + 1) * disCards / 2),
				startHandoffRight.y + currentPhom  * sizeCard.height / 3);
			zOrder = (10 - dem)*(4 - currentPhom);
			break;

		case kUserTop:
			pointStart = getStartPositionCardUserTop_Hit();
			pointEnd = Vec2(startHandoffTop.x - dem * disCards / 2,
				startHandoffTop.y + (currentPhom - 1) * sizeCard.height / 3);
			zOrder = (10 - dem)*(4 - currentPhom);
			break;

		case kUserBot:
			pointStart = getStartPositionCardUserBot_Hit();
			pointEnd = Vec2(startHandoffBot.x - dem * disCards / 2,
				startHandoffBot.y - (currentPhom - 1)* sizeCard.height / 3);
			zOrder = (10 - dem)*(4 + currentPhom);
			break;
		}
		card->setLocalZOrder(zOrder);
		if (!card->isVisible()) {
			card->setPosition(pointStart);
			card->setVisible(true);
		}
		// Chỉ thực hiện khi Card thỏa mãn: vị trí của nó khác so với vị trí cần tới/
		if (card->getPositionX() != pointEnd.x || card->getPositionY() != pointEnd.y) {
			card->stopAllActions();
			card->runAction(Spawn::createWithTwoActions(ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO),
				MoveTo::create(ANIMATION_DURATION, pointEnd)));
		}

		dem++;
	}
}

void LayerCardInGame::removeCardOnHandByID(int pID) {
	Card *card = getCardByID(pID);
	if (card != NULL) {
		removeCardByID(pID);
	}
}

Card* LayerCardInGame::getCardByID(int pID) {
	//     int length = arrAllCard.size();
	//     int indexCard = -1;
	//     for (int i = 0; i < length; i++) {
	//         Card *card =  arrAllCard.at(i);
	//         if (card->getID() == pID) {
	//             indexCard = i;
	//             break; 
	//         }
	//     } 
	vector<Card*>::iterator it;
	for (it = arrAllCard.begin(); it != arrAllCard.end();){
		if ((*it)->GetID() == pID){
			//delete *it;
			return *it;
		}
		else{
			++it;
		}
	}
	return NULL;
	// 	if (indexCard < 0) return NULL;
	// 	return arrAllCard.at(indexCard); 
}

void LayerCardInGame::eventListcardNTF(int posUser, string lc) {
	// lc = numberCardOnHand/id:xx:xx:xx:xx;id:xx:xx:xx:xx;...../
	log("eventListcardNTF");

	// ở đây chỉ thực hiện 2 nhiệm vụ nếu posUser=kUserMe
	// 1. Chia bài (gameStarted = false)
	// 2. Xếp bài (gameStarted = true)
	vector<string> arrString = split(lc, '/');
	int length = arrString.size();

	vector<int> arrID_OnHand; /// mang id cac quan bai tren tay
	vector<int> arrID_OnTable; // mang id cac quan bai tren ban
	vector<int> arrID_Phom; // mang id cac quan bai
	vector<int> arrID_PhomID;//mang quan bai de gui
	vector<int> arrID_CardEat; // mang id cac quan bai an duoc

	if (length >= 2) {
		arrID_OnHand = getIDFromString(arrString.at(1));
		arrID_CardEat = getIDFromString_ByPos(arrString.at(1), 4);
	}

	if (length >= 3) {
		arrID_OnTable = getIDFromString(arrString.at(2));
		arrID_OnHand = getIDFromString(arrString.at(1));
		arrID_CardEat = getIDFromString_ByPos(arrString.at(1), 4);
	}

	if (length >= 4) {
		arrID_OnHand = getIDFromString(arrString.at(1));
		arrID_CardEat = getIDFromString_ByPos(arrString.at(2), 4);
		arrID_Phom = getIDFromString(arrString.at(3));
		arrID_PhomID = getIDFromString_Last(arrString.at(3));
	}


	int lengArrPhom = arrID_Phom.size();

	switch (posUser) {
	case kUserMe:
		if (this->gameStarted == false) {
			// nhảy vào đây để chia bài (chỉ 1 lần)
			log("LISTCARD  - Chia bai!!!");
			if (isResume)// vao lai ban thi khong co animation
				actionDealCardTL(arrID_OnHand, arrID_CardEat);
			else
				actionDealCard(arrID_OnHand, arrID_CardEat);

			this->gameStarted = true;
		}
		else {
			//for using tool phom - develop
			// 			actionSortCard(arrID_OnHand);
			// 			this->allowSortCard = false;
		}

		// phỏm
		if (lengArrPhom > countCardMe_Phom){
			log("ha phom theo sever");
			countCardMe_Phom = lengArrPhom;
			actionHaPhom(arrID_Phom, arrID_PhomID);
			refreshCardOnHand(true);
		}

		// chỉ dành cho 1 lần khi người dùng vào lại bàn đang chơi
		if (this->countCardMe_Take == 0) {
			// bài đã đánh
			if (mCallBackListener && mCallBack){
				paramCallBack->setTag(1);
				(mCallBack->*mCallBackListener)(paramCallBack);
			}
			for (int i = 0; i < arrID_OnTable.size(); i++) {
				int id = arrID_OnTable.at(i);
				removeCardByID(id);

				Card *card = getCardByID(id);
				if (card == NULL) continue;
				card->setVisible(true);
				card->setScale(SCALE_RATIO);
				card->setLocalZOrder(countCardMe_Take);
				card->setPosition(Point(startHitBot.x + (disCards / 2) * countCardMe_Take, startHitBot.y));
				countCardMe_Take++;
			}
		}
		break;

	case kUserLeft:

		// phỏm
		if (lengArrPhom > countCardLeft_Phom){
			countCardLeft_Phom = lengArrPhom;
			actionHaPhomByPos(kUserLeft, arrID_Phom, arrID_PhomID);
		}

		// bài đã đánh
		if (this->countCardLeft_Take == 0) {
			for (int i = 0; i < arrID_OnTable.size(); i++) {
				int id = arrID_OnTable.at(i);
				actionHitCard(kUserLeft, id);
			}
		}


		arrIDCardOnHandLeft.clear();
		arrIDCardOnHandLeft = arrID_OnHand;
		///bài đã ăn được
		if (this->countCardLeft_Eat == 0)
		for (int i = 0; i < arrIDCardOnHandLeft.size(); ++i){
			if (arrIDCardOnHandLeft.size() == arrID_CardEat.size() &&
				arrID_CardEat.at(i) == 1)
				showEatCard(kUserLeft, arrIDCardOnHandLeft.at(i));
		}
		break;

	case kUserRight:

		// ha phom
		log("kUserRight: lengArrPhom=%d, countCardRight_Phom=%d countCardRight_Take = %d",
			lengArrPhom, countCardRight_Phom, countCardRight_Take);
		if (lengArrPhom > countCardRight_Phom){
			countCardRight_Phom = lengArrPhom;
			actionHaPhomByPos(kUserRight, arrID_Phom, arrID_PhomID);
		}
		// bài đã đánh
		if (this->countCardRight_Take == 0) {
			log("bai tren tay");
			for (int i = 0; i < arrID_OnTable.size(); i++) {
				int id = arrID_OnTable.at(i);
				actionHitCard(kUserRight, id);
			}
		}

		// bài trên tay (các quân bài đã ăn được)
		arrIDCardOnHandRight.clear();
		arrIDCardOnHandRight = arrID_OnHand;

		///bài đã ăn được
		if (this->countCardRight_Eat == 0)
		for (int i = 0; i < arrIDCardOnHandRight.size(); ++i){
			if (arrIDCardOnHandRight.size() == arrID_CardEat.size() &&
				arrID_CardEat.at(i) == 1)
				showEatCard(kUserRight, arrIDCardOnHandRight.at(i));
		}
		break;

	case kUserTop:

		// ha phom
		if (lengArrPhom > countCardTop_Phom){
			countCardTop_Phom = lengArrPhom;
			actionHaPhomByPos(kUserTop, arrID_Phom, arrID_PhomID);
		}

		// bài đã đánh
		if (this->countCardTop_Take == 0) {
			for (int i = 0; i < arrID_OnTable.size(); i++) {
				int id = arrID_OnTable.at(i);
				actionHitCard(kUserTop, id);
			}
		}

		arrIDCardOnHandTop.clear();
		arrIDCardOnHandTop = arrID_OnHand;

		///bài đã ăn được
		if (this->countCardTop_Eat == 0)
		for (int i = 0; i < arrIDCardOnHandTop.size(); ++i){
			if (arrIDCardOnHandTop.size() == arrID_CardEat.size() &&
				arrID_CardEat.at(i) == 1)
				showEatCard(kUserTop, arrIDCardOnHandTop.at(i));
		}
		break;

	case kUserBot:

		// ha phom
		if (lengArrPhom > countCardBot_Phom){
			countCardBot_Phom = lengArrPhom;
			actionHaPhomByPos(kUserBot, arrID_Phom, arrID_PhomID);
		}

		// bài đã đánh
		if (this->countCardBot_Take == 0) {
			for (int i = 0; i < arrID_OnTable.size(); i++) {
				int id = arrID_OnTable.at(i);
				actionHitCard(kUserBot, id);
			}
		}

		// bai tren tay
		arrIDCardOnHandBot.clear();
		arrIDCardOnHandBot = arrID_OnHand;
		///bài đã ăn được
		if (this->countCardBot_Eat == 0)
		for (int i = 0; i < arrIDCardOnHandBot.size(); ++i){
			if (arrIDCardOnHandBot.size() == arrID_CardEat.size() &&
				arrID_CardEat.at(i) == 1)
				showEatCard(kUserBot, arrIDCardOnHandBot.at(i));
		}
		break;

	default:
		break;
	}
	playSound(S_HAND_OFF);
}

void LayerCardInGame::event_EXT_SRVNTF_RECOMMENDED_CARDSETS(string lc) {
	// lưu trữ để chờ đợi hàm refresfCardOnHand() thực hiện runAction xong,
	// khi đó ở đó sẽ làm tiếp cv :).
	lcRecommendHaPhom = lc;
	requestRecommendHaPhom = true;
}

Point LayerCardInGame::getPointCardInCenter() {
	return Vec2(winSize.width / 2, winSize.height / 2 - sizeCard.height / 2);
}

Point LayerCardInGame::getStartPositionCardUserLeft_Hit(){
	return Point(startHitLeft.x, startHitLeft.y);
}

Point LayerCardInGame::getStartPositionCardUserRight_Hit() {
	return Point(startHitRight.x, startHitRight.y);
}

Point LayerCardInGame::getStartPositionCardUserTop_Hit() {
	return Point(startHitTop.x, startHitTop.y + W_AVATAR);
}

Point LayerCardInGame::getStartPositionCardUserBot_Hit() {
	return Point(startHitBot.x, startHitBot.y - W_AVATAR);
}

Point LayerCardInGame::getStartPositionCardByPos(int pos) {
	switch (pos) {
	case kUserLeft:
		return getStartPositionCardUserLeft_Hit();
		break;

	case kUserRight:
		return getStartPositionCardUserRight_Hit();
		break;

	case kUserTop:
		return getStartPositionCardUserTop_Hit();
		break;

	case kUserBot:
		return getStartPositionCardUserBot_Hit();
		break;

	default: return getPointCardInCenter(); break;
	}
}

Point LayerCardInGame::getStartPositionCardUserLeft_Hit_TienLen() {
	return Vec2(AVATAR_L_SPACE + W_AVATAR + sizeAdd.width, winSize.height / 2 - sizeAdd.height);
}

Point LayerCardInGame::getStartPositionCardUserRight_Hit_TienLen() {
	return Vec2(WIDTH_DESIGN - AVATAR_L_SPACE - W_AVATAR - sizeAdd.width,
		winSize.height / 2 - sizeAdd.height);
}

Point LayerCardInGame::getStartPositionCardUserTop_Hit_TienLen() {
	return Vec2(WIDTH_DESIGN / 2 + sizeAdd.width,
		winSize.height - W_AVATAR);
}

Point LayerCardInGame::getStartPositionCardUserBot_Hit_TienLen() {
	return Vec2(WIDTH_DESIGN / 2 + sizeAdd.width, W_AVATAR * 2);
}

Point LayerCardInGame::getStartPositionCardByPos_TienLen(int pos) {
	switch (pos) {
	case kUserLeft:
		return getStartPositionCardUserLeft_Hit_TienLen();
		break;

	case kUserRight:
		return getStartPositionCardUserRight_Hit_TienLen();
		break;

	case kUserTop:
		return getStartPositionCardUserTop_Hit_TienLen();
		break;

	case kUserBot:
		return getStartPositionCardUserBot_Hit_TienLen();
		break;

	default: return getPointCardInCenter(); break;
	}
}

void LayerCardInGame::setCountCardByPos(int pos, int count) {
	// 	int gameID = SceneManager::getSingleton().getGameID();
	// 	if (gameID != kGameTLMNDemLa) return;
	string countString = StringUtils::format("%d", count);
	Label *label;

	switch (pos) {
	case kUserLeft:
		cardBackLeft->setVisible(count > 0);
		label = (Label*)cardBackLeft->getChildByTag(kTagCountCard);
		break;

	case kUserRight:
		cardBackRight->setVisible(count > 0);
		label = (Label*)cardBackRight->getChildByTag(kTagCountCard);
		break;

	case kUserTop:
		cardBackTop->setVisible(count > 0);
		label = (Label*)cardBackTop->getChildByTag(kTagCountCard);
		break;

	case kUserBot:
		cardBackBot->setVisible(count > 0);
		label = (Label*)cardBackBot->getChildByTag(kTagCountCard);
		break;

	default: label = NULL; break;
	}

	if (label != NULL) {
		label->setString(countString.c_str());
		// 		Size sizeLabel = label->getContentSize();
		// 		Size sizeCard = cardBackLeft->getContentSize();
		// 		Point pointCenter = Point((sizeCard.width - sizeLabel.width) / 2, (sizeCard.height - sizeLabel.height) / 2);
		// 		label->setPosition(pointCenter);
	}
    if (SceneManager::getSingleton().isNagaWin){
        if (label != NULL)
            label->setColor(Color3B::BLACK);
    }
}

void LayerCardInGame::showCardOnHandAll() {
	countCardLeft_Take = 5;
	countCardRight_Take = 5;
	countCardTop_Take = 5;
	countCardBot_Take = 5;
	showCardOnHandByPos_Arr(kUserLeft, arrIDCardOnHandLeft);
	showCardOnHandByPos_Arr(kUserRight, arrIDCardOnHandRight);
	showCardOnHandByPos_Arr(kUserTop, arrIDCardOnHandTop);
	showCardOnHandByPos_Arr(kUserBot, arrIDCardOnHandBot);
}

void LayerCardInGame::showCardOnHandByPos_Arr(int kUser, vector<int> arrID) {
	// 	try{
	// 		std::sort(arrID.begin(), arrID.end());
	// 	}
	// 	catch (std::exception &e)
	// 	{
	// 		log("Exceptions");
	// 	}

	for (int i = 0; i < arrID.size(); i++)
	{
		// 		if (kUser == kUserBot)
		// 			actionShowLastCardBot(arrID.at(i));
		// 		else
		actionHitCard(kUser, arrID.at(i));
	}
}

void LayerCardInGame::showCardOnHandByPos_List(int kUser, string lc) {
	// o day danh cho tien len
	this->countCardLeft_Take = 0;
	this->countCardRight_Take = 0;
	this->countCardTop_Take = 0;
	SCALE_RATIO = 0.9;
	if (cardBackLeft != NULL) {
		cardBackLeft->setVisible(false);
		cardBackRight->setVisible(false);
		cardBackTop->setVisible(false);
		cardBackBot->setVisible(false);
	}

	vector<string> arrIDString = split(lc, ',');
	vector<int>	   arrID;

	for (int i = 0; i < arrIDString.size(); i++)
	{
		int id = atoi(arrIDString.at(i).c_str());
		if (id > 0) arrID.push_back(id);
	}

	// xep lai
	int i = 0, j = 0, tg = 0;
	if (arrID.size() != 0){
		for (i = 0; i < arrID.size() - 1; i++)
		for (j = i + 1; j < arrID.size(); j++) {
			if (arrID.at(i) > arrID.at(j)) {
				tg = arrID.at(i);
				arrID.at(i) = arrID.at(j);
				arrID.at(j) = tg;
			}
		}
	}

	showCardOnHandByPos_Arr(kUser, arrID);
}

void LayerCardInGame::hideCardByArr(vector<int> arrID) {
	int leng = arrID.size();
	Card *card;
	for (int i = 0; i < leng; i++) {
		card = getCardByID(arrID.at(i));
		if (card == NULL) continue;
		card->setVisible(false);
	}
}

void LayerCardInGame::setFunCallBack(Ref *target, SEL_CallFuncO listener)
{
	mCallBack = target;
	mCallBackListener = listener;
}

void LayerCardInGame::removeCardByID(int pID)
{
	vector<Card*>::iterator it;
	for (it = arrCardOnHand.begin(); it != arrCardOnHand.end();){
		if ((*it)->GetID() == pID){
			//delete *it;
			it = arrCardOnHand.erase(it);
		}
		else{
			++it;
		}
	}
}

void LayerCardInGame::showLastCard(string lstCard)
{
	vector<string> arr = split(lstCard, '|');
	if (strcmp(arr.at(3).c_str(), "") == 0)
		return;
	vector<int> arr_OnTable;

	arr_OnTable = getIDFromString_TienLen(arr.at(3));
	arrIDCardOnTable.clear();
	arrIDCardOnTable = arr_OnTable;
	arr_OnTable.clear();
	//actionHitCardByTienLen(kUserMe, arrIDCardOnTable);

	int length = arrIDCardOnTable.size();
	float toLeft = WIDTH_DESIGN / 2 - (sizeCard.width / 2 * length) / 2;
	float toTop = getPointCardInCenter().y;

	for (int i = 0; i < length; i++) {
		int id = arrIDCardOnTable.at(i);
		Card *card = getCardByID(id);
		if (card == NULL) continue;;
		removeCardByID(id);
		if (!card->isVisible()) {
			card->setPosition(getPointCardInCenter());
			card->setVisible(true);
		}
		card->setLocalZOrder(1);
		card->setScale(SCALE_RATIO);
		card->setPosition(Vec2(toLeft + i * sizeCard.width / 2, getPointCardInCenter().y));
		arrHitCardOnTable.push_back(card);
	}
}

void LayerCardInGame::hideLastCard()
{
	arrHitCardOnTable.clear();
	hideCardByArr(arrIDCardOnTable);
}

bool LayerCardInGame::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	CC_UNUSED_PARAM(pEvent);
	if (mTouchFlag) return false;
	mTouchFlag = true;
	Point tap;
	Touch *touch = pTouch;
	tap = this->convertToNodeSpace(touch->getLocation());
	this->pointTouchBegan = tap;

	// refresh status
	disTouchBegan.setSize(0, 0);
	indexCardCurrent = -1;
	indexCardTarget = -1;
	isClickCard = true;

	int length = arrCardOnHand.size();
	for (int i = 0; i < length; i++) {
		Card *card = arrCardOnHand.at(i);
		/*if (card == NULL) return false;*/
		if (card->getRecommend()) return false;
		if ((i < length - 1 && isTouchedCard_notTail(card, tap)) || (i == length - 1 && isTouchedCard_Tail(card, tap))){
			countTouched++;

			disTouchBegan.setSize(tap.x - card->getPositionX(), tap.y - card->getPositionY());

			indexCardCurrent = i;
			break;
		}
	}

	return true;
}

void LayerCardInGame::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	CC_UNUSED_PARAM(pEvent);
	mTouchFlag = false;
    
	if (indexCardCurrent >= 0 && indexCardCurrent < arrCardOnHand.size()){

		Card *cardCurrent = arrCardOnHand.at(indexCardCurrent);
		if (cardCurrent == NULL) return;
		Touch *touch;
		touch = pTouch;
		Point tap;
		tap = convertToNodeSpace(touch->getLocation());
		// clicked
		if (getDisPoint(tap, pointTouchBegan) < 20) {
			cardCurrent->setLocalZOrder(indexCardCurrent + 1000);
			setCardClick(cardCurrent);
			//recommend card
            if (isRecommendOn())//check config flag
                this->runAction(Sequence::create(DelayTime::create(DELAY_DURATION),CallFunc::create([this](){this->actionRecommendHitCard();}),NULL));
		}
		// moved
		else {
			if (indexCardCurrent < indexCardTarget) indexCardTarget++;

			if (indexCardTarget >= 0){
				vector<Card*> arrCardCopy;

				for (int i = 0; i < indexCardTarget; i++) {
					Card *card = arrCardOnHand.at(i);
					if (card == NULL) continue;
					if (i != indexCardCurrent) {
						arrCardCopy.push_back(card);
					}
				}

				Card *card = arrCardOnHand.at(indexCardCurrent);
				arrCardCopy.push_back(card);

				for (int i = indexCardTarget; i < arrCardOnHand.size(); i++) {
					Card *card = arrCardOnHand.at(i);
					if (card == NULL) continue;
					if (i != indexCardCurrent) {
						arrCardCopy.push_back(card);
					}
				}

				arrCardOnHand.clear();
				arrCardOnHand.insert(arrCardOnHand.end(), arrCardCopy.begin(), arrCardCopy.end());
			}
			refreshCardOnHandWhenMove(false);
		}

	}
}

void LayerCardInGame::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	Touch *touch = pTouch;
	Point tap;

	if (indexCardCurrent >= 0 && indexCardCurrent < arrCardOnHand.size()) {
		int length = arrCardOnHand.size();

		Card *card = arrCardOnHand.at(indexCardCurrent);
		if (card == NULL) return;
		Card *cardTarget;

		tap = this->convertToNodeSpace(touch->getLocation());

		if (disTouchBegan.width > 0) {
			isClickCard = false;

			// tim quan bai Target
			for (int iT = 0; iT < length; iT++) {
				cardTarget = arrCardOnHand.at(iT);

				if (card != cardTarget){
					if ((iT < length - 1 && isTouchedCard_notTail(cardTarget, tap))
						|| (iT == length - 1 && isTouchedCard_Tail(cardTarget, tap))){
						indexCardTarget = iT;
						break;
					}
				}
			}

			tap.x -= disTouchBegan.width;
			tap.y -= disTouchBegan.height;
			if (tap.y <= topCard)
				tap.y = topCard;

			card->setLocalZOrder(length + 1000);
			card->setPosition(tap);
		}

	}
}

void LayerCardInGame::onTouchCancelled(Touch *pTouch, Event *pEvent)
{
	mTouchFlag = false;
}

bool LayerCardInGame::isRunningAction()
{
	bool isRunning = false;
	for (int i = 0; i<arrAllCard.size(); i++){
		Card* pcard = arrAllCard.at(i);
		if (pcard->getNumberOfRunningActions()>0){
			isRunning = true;
			break;
		}
	}
	return isRunning;
}

void LayerCardInGame::onEnter()
{
	Layer::onEnter();
	//auto dispatcher = Director::getInstance()->getEventDispatcher();
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = CC_CALLBACK_2(LayerCardInGame::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerCardInGame::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerCardInGame::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(LayerCardInGame::onTouchCancelled, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	//_touchListener = listener;
}

void LayerCardInGame::onExit()
{
	// 	auto dispatcher = Director::getInstance()->getEventDispatcher();
	// 	dispatcher->removeEventListener(_touchListener);
	_eventDispatcher->removeEventListener(listener_forcard);
	_eventDispatcher->removeEventListenersForTarget(this);
	Layer::onExit();
}

bool LayerCardInGame::checkCardIsRunning(int cardID)
{
	bool isAction = false;
	for (int i = 0; i < arrAllCard.size(); ++i){
		Card *card = arrAllCard.at(i);
		if (card->GetID() == cardID && card->getNumberOfRunningActions() > 0){
			log("card animate");
			isAction = true;
			break;
		}
	}
	return isAction;
}

bool LayerCardInGame::checkCardIsRunning(string listCard)
{
	bool isAction = false;
	vector<int> arrID = this->getIDFromString_TienLen(listCard);
	for (int i = 0; i < arrID.size(); ++i){
		for (int j = 0; j < arrAllCard.size(); ++j){
			Card * card = arrAllCard.at(j);
			if (card->GetID() == arrID.at(i) && card->getNumberOfRunningActions() > 0){
				isAction = true;
				break;
			}
		}
	}
	return isAction;
}

void LayerCardInGame::actionHitCardRollBack(int cardID)
{
	Card *card = this->getCardByID(cardID);
	if (card == NULL) return; //lay quan bai da danh tren ban
	//xoa quan bai tren ban 

	this->countCardMe_Take--;
	this->removeCardByArray(cardID, arrIDCardOnTable);
	card->stopAllActions();
	arrCardOnHand.push_back(card);
	card->setScaleCardOriginal();
	card->resetStatus();
	card->setVisible(true);

}

void LayerCardInGame::removeCardByArray(int cardID, vector<int> arrIDCard)
{
	vector<int>::iterator it;
	for (it = arrIDCard.begin(); it != arrIDCard.end();){
		if (*it == cardID){
			it = arrIDCard.erase(it);
		}
		else{
			++it;
		}
	}
}

void LayerCardInGame::actionSortCardPhom()
{
	//sort mang arrIDCardOnHandBot, dau vao la arrCardOnHand, dau ra la mot mangid de thuc hien sort
	//CardDeck::instance();//khoi tao instance carddeck
	//sau khi da sap xep mang pCards xong thi get id tu mang pCards;
	int dem = 0; //bien nay de dem so bai tren tay, tranh crash khi card bi null
	Card *card;
	int length_arr = arrCardOnHand.size();
	for (int i = 0; i < arrCardOnHand.size(); i++)
	{
		card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		card->setVisible(false);
		dem++;
	}

	CardOrderAlgo algo;//khoi tao object de thuc hien sap xep
	CardOrder* pCards = new CardOrder[dem];
	if (arrCardOnHand.size() < 1) return;

	for (int i = 0; i < dem; ++i){
		Card* card = arrCardOnHand.at(i);
		//if (card == NULL) continue;
		pCards[i].index = card->GetID();
		if (card->getEat()){
			pCards[i].bTaken = 1;
		}

	}
	algo.order(pCards, dem);
	arrCardOnHand.clear();

	for (int i = 0; i < length_arr; i++) {
		int id = pCards[i].index;
		card = getCardByID(id);
		if (card == NULL) continue;;
		card->setVisible(true);
		arrCardOnHand.push_back(card);
	}

	bool isRefreshTop = false;
	refreshCardOnHand(isRefreshTop);
}

void LayerCardInGame::actionHitCardTLRollBack(vector<int> arrIDCardRollBack, bool isRefresh)
{
	//kiem tra quan rollback co o tren ban hay khong thi moi rollback
	if (arrIDCardRollBack.size() == 0) return;

	for (int i = 0; i < arrIDCardRollBack.size(); ++i){
		Card* card = getCardByID(arrIDCardRollBack.at(i));
		if (card == NULL) continue;
		// 		bool isCanRollback = true;
		// 		for (int j = 0; j < arrHitCardOnTable.size(); ++j)
		// 		{
		// 			if (card->GetID() != arrHitCardOnTable.at(j)->GetID())
		// 				isCanRollback = false;
		// 		}
		// 		if (isCanRollback) //kiem tra id neu co o tren ban thi moi rollback
		// 		{
		card->setRotation(0);
		card->setScaleCardOriginal();
		arrCardOnHand.push_back(card);
	}
	arrHitCardOnTable.clear();
	//if (isRefresh)
	refreshCardOnHand(true);
}

vector<int> LayerCardInGame::getListPhomByListID(string listID)
{
	vector<int> arr;
	return arr;
}

int LayerCardInGame::getCountCardOfMe()
{
	return countCardMe_Take;
}

bool LayerCardInGame::checkCardWhenSort(vector<int> arrID, vector<int> arrIDTable)
{
	for (int i = 0; i < arrID.size(); ++i){
		for (int j = 0; j < arrIDTable.size(); ++j){
			if (arrID.at(i) == arrIDTable.at(j)){
				//neu id quan xep co tren ban
				return true;
			}
		}
	}
	return false;
}

void LayerCardInGame::actionShowLastCardBot(int pID)
{
	int length = arrAllCard.size();
	if (length > 0){
		Vec2 pointEnd;
		Card* card = this->getCardByID(pID);
		if (card == NULL) return;
		card->setVisible(true);
		card->setFromUser(kUserBot);
		card->setLocalZOrder(countCardBot_Take);
		pointEnd = Vec2(startHitBot.x + sizeCard.width * 2 + W_AVATAR / 2 + disCards / 2 * (countCardBot_Take % 5),
			startHitBot.y - (countCardBot_Take / 5) * (sizeCard.height / 3));
		card->runAction(Spawn::createWithTwoActions(MoveTo::create(ANIMATION_DURATION, pointEnd),
			ScaleTo::create(ANIMATION_DURATION, SCALE_RATIO)));
		countCardBot_Take++;
	}
}

void LayerCardInGame::hideArrowCardCanEat()
{
	Card* cardeat = getCardByID(idCardCanEat);
	if (cardeat == NULL) return;
	_eventDispatcher->removeEventListener(listener_forcard);
	cardeat->removeArrowEat();
}

void LayerCardInGame::actionRecommendPushCard(string _listCard)
{
	//b:{"cmd":"ntfpucs","params":{"encrypts":"","uid":497,"pcrds":"14:18:22","usrn":"hoadai"},"sourceRoom":242}
	vector<string> listID = mUtils::splitString(_listCard, PHOM_CHAR_SPLIT_2);
	for (int i = 0; i < listID.size(); ++i)
	{
		Card* card = this->getCardByID(atoi(listID.at(i).c_str()));
		if (card == NULL) continue;
		this->setCardClick(card);
	}
	this->refreshCardOnHand(false);
}

std::string LayerCardInGame::getListCardOnHand()
{
	string list;
	for (int i = 0; i < arrCardOnHand.size(); ++i)
	{
		Card* card = arrCardOnHand.at(i);
		if (card == NULL) continue;
		list.append(StringUtils::format("%d,", card->GetID()));
	}
	log("getListCardOnHand = %s", list.c_str());
	return list;
}

void LayerCardInGame::setPreviousCard(string _list)
{
	// 	if (isRecommendOn())//check config flag
	// 	{
	log("previous card in game = %s", _list.c_str());
	logicTL->setPreviousCard(_list);
	//reset game neu chuoi = rong
	if (0 == _list.compare(""))
	{
		logicTL->resetGame();
	}
	//	}
}

void LayerCardInGame::setCardRecommend(vector<CardInfo*> cards)
{
    this->setLastLengthChoose(cards.size());
	for (int i = 0; i < cards.size(); i++)
	{
		int id = cards.at(i)->getIdCard();
		Card* card = getCardOnHandByID(id);
		if (card == NULL) continue;
        card->removeChildByTag(120);

		//setCardClick(card);
		if (card->getClicked()) continue;
		float top = topCard + sizeCard.height / 4;
		auto actionMove = MoveTo::create(DELAY_DURATION, Vec2(card->getPositionX(), top));
		card->runAction(actionMove);
		//card->setPositionY(top);
		card->setClicked(true);
	}
}

void LayerCardInGame::flipAction2()
{
	Sprite *card = (Sprite*)this->getChildByTag(246);
	Show* show = Show::create();
	OrbitCamera *camera = OrbitCamera::create(ANIMATION_DURATION, 1, 0, 270, 90, 0, 0);
	ActionInterval *action = Sequence::create(show, camera, nullptr);
	card->runAction(action);
}

void LayerCardInGame::callBackDealCard(Node* pSender)
{
	int tag = pSender->getTag();
	// 	OrbitCamera *cameraBack = OrbitCamera::create(ANIMATION_DURATION, 1, 0, 0, 90, 0, 0);
	// 	Hide* hide = Hide::create();
	// 	Sprite* cardBack = (Sprite*)pSender;
	// 	cardBack->runAction(Sequence::create(cameraBack, hide));

	Show* show = Show::create();
	OrbitCamera *camera = OrbitCamera::create(DEAL_DURATION, 1, 0, 270, 90, 0, 0);
	ActionInterval *action = Sequence::create(show, camera, nullptr);

	float leftTheFirst = (WIDTH_DESIGN - (arrCardOnHand.size() - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
	Card *card = arrCardOnHand.at(tag);
	if (card == NULL){
		return;
	}
	//card->setVisible(true);
	card->setRecommend(false);
	card->setClicked(false);
	card->setScaleCardOriginal();
	card->setLocalZOrder(tag + 1000);
	card->setPosition(Vec2(leftTheFirst + tag * disCards, topCard));
	card->runAction(action);
}

void LayerCardInGame::actionRecommendHitCard()
{
	//check for recommend
	log("check recommend");
	if (_my)//den luot minh moi check
	{
        
        if (this->isRecommend){
            if (mCallBack && mCallBackListener){
                paramCallBack->setTag(5);
                (mCallBack->*mCallBackListener)(paramCallBack);
            }
            return;
        }
        
        this->removeArrow();
		vector<int> arrClicked = getIdCardByClicked();
		string myListCard = getListCardOnHand();
		if (_recommend)
		{
			//co the do duoc
            if (logicTL->priviousplaycard.size() >= 2 && arrClicked.size() > logicTL->priviousplaycard.size() && logicTL->priviousplaycard[0]->getValueCard()!= 15){
                this->setLastLengthChoose(0);

                if (mCallBack && mCallBackListener){
                    paramCallBack->setTag(5);
                    (mCallBack->*mCallBackListener)(paramCallBack);
                }
                return;
            }
			vector<CardInfo*> cards = logicTL->getRecommend(myListCard, arrClicked);
			if (cards.size() > 0)
			{
                if (lastLengthChoose < arrClicked.size()){
					setCardRecommend(cards);
                    if (mCallBack && mCallBackListener){
                        lastLengthChoose = getIdCardByClicked().size();
                        paramCallBack->setTag(4);
                        (mCallBack->*mCallBackListener)(paramCallBack);
                    }
                }else{
                    if (mCallBack && mCallBackListener){
                        paramCallBack->setTag(5);
                        (mCallBack->*mCallBackListener)(paramCallBack);
                    }
                    this->setLastLengthChoose(0);
                }
			}
			else{
				if (mCallBack && mCallBackListener){
					paramCallBack->setTag(5);
					(mCallBack->*mCallBackListener)(paramCallBack);
				}
                this->setLastLengthChoose(0);
			}
           // this->isRecommend = true;
		}
		//}
	}
}

void LayerCardInGame::actionDealCardPro(int countCard)
{
	Card* cardBack = this->getCardByID(-1);
	if (cardBack)
	{
		float leftTheFirst = (WIDTH_DESIGN - (10 - 1) * disCards - sizeCard.width) / 2 + disCardAdd;
		float rightTheFirst = (WIDTH_DESIGN - sizeCard.width) / 2 + disCardAdd;
		for (int i = 0; i < countCard; i++)
		{
			Sprite *cardfordeal = Sprite::createWithSpriteFrame(cardBack->getSpriteFrame());
			cardfordeal->setAnchorPoint(Vec2::ZERO);
			cardfordeal->setPosition(WIDTH_DESIGN / 2, HEIGHT_DESIGN / 2);
			cardfordeal->setTag(i);
			cardfordeal->setScale(sizeCard.width / cardfordeal->getContentSize().width, sizeCard.height / cardfordeal->getContentSize().height);
			this->addChild(cardfordeal);
			Vec2 pointDes;
			pointDes.setPoint(leftTheFirst + i * disCards, topCard);
			cardfordeal->runAction(Sequence::create(
				DelayTime::create(DEAL_DURATION * i),
				MoveTo::create(ANIMATION_DURATION, pointDes),
				// 				camera,
				// 				hide,
				CallFuncN::create(CC_CALLBACK_1(LayerCardInGame::callBackDealCard, this)),
				RemoveSelf::create(true),
				nullptr));
		}
	}
}

Card* LayerCardInGame::getCardOnHandByID(int pID)
{
	vector<Card*>::iterator it;
	for (it = arrCardOnHand.begin(); it != arrCardOnHand.end();){
		if ((*it)->GetID() == pID){
			//delete *it;
			return *it;
		}
		else{
			++it;
		}
	}
	return NULL;
}

void LayerCardInGame::setLastLengthChoose(int _length)
{
	lastLengthChoose = _length;
}
void LayerCardInGame::showArrowInCard(int pID){
    Card* card = this->getCardOnHandByID(pID);
    if (card == NULL) return;
    //setCardClick(card);
    if (card->getClicked()) return;
    card->removeChildByTag(120);
    Sprite* arrow = Sprite::create("arrow_phom.png");
    arrow->setTag(120);
    arrow->setPosition(Vec2(card->getContentSize().width / 2, card->getContentSize().height));
    card->addChild(arrow);
    arrow->runAction(RepeatForever::create(Sequence::create(MoveBy::create(0.5, Vec2(0, 10)), MoveBy::create(0.5, Vec2(0, -10)), nullptr)));
}
void LayerCardInGame::removeArrow(){
    for (int i = 0; i < arrCardOnHand.size(); i++) {
        Card *card = arrCardOnHand.at(i);
        if (card == NULL) continue;
        card->removeChildByTag(120);
    }
}











