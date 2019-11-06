#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

#include<vector>
#include<cmath>
#include<iostream>
#include<string>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 512;

const double PI = 3.14159265358979323846264388;

class PowerUp {
public:
	PowerUp(int type, Vector2f position);
	void draw(RenderWindow* window);
	Vector2f getPosition();
	float getRadius();
	void collect();
	bool isCollected();
private:
	CircleShape shape;
	Vector2f position;
	int type;
	float radius;
	bool collected;
};


PowerUp::PowerUp(int type, Vector2f position) {
	this->position = position;
	this->type = type;
	this->radius = 10.0f;

	this->collected = false;

	this->shape = CircleShape(this->radius);
	this->shape.setFillColor(Color(200, 0, 255));
}

void PowerUp::collect() {
	this->shape.setFillColor(Color::Red);
	this->collected = true;
}

bool PowerUp::isCollected() {
	return this->collected;
}

Vector2f PowerUp::getPosition() {
	return Vector2f(this->position.x + this->radius, this->position.y + this->radius);
}

float PowerUp::getRadius() {
	return this->radius;
}

void PowerUp::draw(RenderWindow* window) {
	this->shape.setPosition(Vector2f(this->position.x - this->radius, this->position.y + this->radius));
	if (!this->collected) {
		window->draw(this->shape);
	}
}

class Scoreboard {
public:
	Scoreboard(Vector2f position);
	void draw(RenderWindow* window);
	void update(int scoreRight, int scoreleft);
	void reset();
private:
	int leftScore;
	int rightScore;
	Text leftScoreText;
	Text rightScoreText;
	Vector2f positionLeft;
	Vector2f positionRight;
	Font font;
};

Scoreboard::Scoreboard(Vector2f position) {
	this->leftScore = 0;
	this->rightScore = 0;

	sf::Font fontLoader;
	if (!fontLoader.loadFromFile("arial.ttf"))
	{
		std::exit(-1);
	}

	this->font = fontLoader;

	this->leftScoreText = Text("0", this->font, 30);
	this->rightScoreText = Text("0", this->font, 30);
	this->leftScoreText.setPosition(Vector2f(position.x - 100.0f, position.y));
	this->rightScoreText.setPosition(Vector2f(position.x + 80.0f, position.y));
	this->leftScoreText.setFillColor(sf::Color::White);
	this->rightScoreText.setFillColor(sf::Color::White);
	this->leftScoreText.setStyle(sf::Text::Bold);
	this->rightScoreText.setStyle(sf::Text::Bold);
}

void Scoreboard::draw(RenderWindow* window) {
	window->draw(this->leftScoreText);
	window->draw(this->rightScoreText);
}

void Scoreboard::update(int scoreLeft, int scoreRight) {
	// update score ints
	this->leftScore += scoreLeft;
	this->rightScore += scoreRight;

	// change  text to reflect change
	this->leftScoreText.setString(to_string(this->leftScore));
	this->rightScoreText.setString(to_string(this->rightScore));
}

void Scoreboard::reset() {
	// update score ints
	this->leftScore = 0;
	this->rightScore = 0;

	// change  text to reflect change
	this->leftScoreText.setString(to_string(this->leftScore));
	this->rightScoreText.setString(to_string(this->rightScore));
}

class Paddle {
public:
	// constructors, default pos is (0,0)
	Paddle();
	Paddle(Vector2f position);

	// accessors and mutators
	Vector2f getPosition();
	Vector2f getSize();
	void setAi(bool toSet);

	// render function
	void draw(RenderWindow* window);

	// update functions
	void setVelocityPlayer(float dt, bool down, bool up);
	void setVelocityAi(float dt, Vector2f bp);
	void updateDelegator(float dt, bool down, bool up, Vector2f bp);
private:
	float velocity_y;
	Vector2f position;
	RectangleShape shape;
	float height;
	float baseVelocity;
	bool ai;
};

Paddle::Paddle(Vector2f position) {
	// set up shape
	this->height = 70.0f;
	this->shape = RectangleShape(Vector2f(10.0f, this->height));

	// set up pos and velocity
	this->position = position;
	this->velocity_y = 0.0f;
	this->baseVelocity = 0.4f;

	// set up ai
	this->ai = false;
}

Paddle::Paddle() {
	Paddle::Paddle(Vector2f(0.0f, 0.0f));
}

void Paddle::setAi(bool toSet) {
	this->ai = toSet;
}

Vector2f Paddle::getPosition() {
	return this->position;
}

Vector2f Paddle::getSize() {
	return this->shape.getSize();
}

// delegates to player OR AI function and then updates
void Paddle::updateDelegator(float dt, bool down, bool up, Vector2f bp) { 
	if (this->ai) { // if this is an AI paddle
		setVelocityAi(dt, bp);
	}
	else { // this is a player paddle
		setVelocityPlayer(dt, down, up);
	}

	// update position based on velocity
	this->position.y += this->velocity_y * dt;

	// check y bounds (keep paddle on screen)
	if (this->position.y + this->height > WINDOW_HEIGHT) {
		this->position.y = WINDOW_HEIGHT - this->height;
	}
	else if (this->position.y < 0) {
		this->position.y = 0;
	}
}

void Paddle::setVelocityAi(float dt, Vector2f bp) {
	float distanceToBall = abs(this->position.x - bp.x);
	if (distanceToBall < WINDOW_WIDTH / 2.0f) {
		if (bp.y > this->position.y + this->height) {
			this->velocity_y = this->baseVelocity;
		}
		else if (bp.y < this->position.y) {
			this->velocity_y = -1 * this->baseVelocity;
		}
		else {
			this->velocity_y = 0.0f;
		}
	}
	else {
		this->velocity_y = 0.0f;
	}
}

void Paddle::setVelocityPlayer(float dt, bool down, bool up) {
	// set velocity on bools
	if (down && up || !(down || up)) {
		// no buttons or both buttons gives no net change
		this->velocity_y = 0.0f;
	}
	else if (down) {
		this->velocity_y = this->baseVelocity;
	}
	else if (up) {
		this->velocity_y = -1 * this->baseVelocity;
	}
}

void Paddle::draw(RenderWindow* window) {
	this->shape.setPosition(this->position);
	window->draw(this->shape);
}

class Ball {
public:
	Ball();
	Ball(Vector2f position);
	Ball(Vector2f position, Vector2f velocity);
	void draw(RenderWindow* window);
	void update(float dt);
	void bounce(Paddle p);
	Vector2f getPosition();
	void setPosition(Vector2f newPosition);
	float getRadius();
	void setRadius(float newrad);
	void randomizeStartVelocity();
	Vector2f getVelocity();
	void setVelocity(Vector2f velocity);
	int isOffScreen();
private:
	Vector2f velocity;
	float baseSpeed;
	Vector2f position;
	CircleShape shape;
	float radius;
	int colorCycleCount; // for flashing ball
	int offScreen;
};

Ball::Ball(Vector2f position) {
	// set up shape
	this->radius = 5;
	this->shape = CircleShape(this->radius);
	this->shape.setFillColor(Color::Blue);

	// set up position and velocity
	this->position = position;
	this->baseSpeed = 0.4f;
	this->randomizeStartVelocity();

	this->colorCycleCount = 10;
}
Ball::Ball(Vector2f position, Vector2f velocity) {
	// set up shape
	this->radius = 5;
	this->shape = CircleShape(this->radius);
	this->shape.setFillColor(Color::Blue);

	// set up position and velocity
	this->position = position;
	this->baseSpeed = 0.4f;
	this->velocity = velocity;

	this->colorCycleCount = 10;
}

Ball::Ball() {
	Ball::Ball(Vector2f(0.0f, 0.0f));
}

int Ball::isOffScreen() {
	return this->offScreen;
}

void Ball::setVelocity(Vector2f velocity) {
	this->velocity = velocity;
}

Vector2f Ball::getVelocity() {
	return this->velocity;
}

void Ball::randomizeStartVelocity() {
	float theta = rand() % 11 + 1; // rand 1-10
	theta = (theta / 10) * (PI / 4.0f); // rand 0-Pi/4
	float newX = cos(theta) * this->baseSpeed;
	float newY = sin(theta) * this->baseSpeed;

	int flipY = rand() % 2; // 50% chance to flip y
	if (flipY == 0) {
		newY *= -1;
	}
	int flipX = rand() % 2; // 50% chance to flip x (shoot at other player)
	if (flipX == 0) {
		newX *= -1;
	}

	// velocity always points to right
	this->velocity = Vector2f(newX, newY);
}

void Ball::setPosition(Vector2f newPosition) {
	this->position = newPosition;
}

void Ball::setRadius(float newrad) {
	if (newrad > 1) {
		this->radius = newrad;
	}
	this->shape.setRadius(this->radius);
}

/*
void Ball::bounce(Paddle p) {
	// flip x
	this->velocity.x *= -1.0f;
	// change angle
	float midP = p.getPosition().y + p.getSize().y / 2.0f; // midpoint of the paddle (y)
	float spread = abs(midP - this->position.y); // distance from midpoint to collision (y)
	float scaleFactor = spread / p.getSize().y + 1.0f; // ratio of distance to paddle height + 1
	this->velocity.y *= scaleFactor; // scale y by ratio
	this->velocity.x *= 1.0f / scaleFactor; // reduce x by ratio (to maintain overall speed)
	// accelerate
	this->velocity.x *= 1.1f;
	this->velocity.y *= 1.1f;
}
*/

void Ball::bounce(Paddle p) {

	float currentX = this->velocity.x; // current x and y components
	float currentY = this->velocity.y;
	float currentMagnitude = sqrt(currentX * currentX + currentY * currentY); // distance formula for magnitude of velocity
	currentMagnitude *= 1.1f; // 10% increase in speed each bounce

	float midP = p.getPosition().y + p.getSize().y / 2.0f; // midpoint of the paddle (y)
	float spread = abs(midP - this->position.y + this->radius); // distance from midpoint y to ball center y
	float ratio = spread / (p.getSize().y / 2.0f); // ratio of distance to total paddle height (between 0-1)
	float theta = ratio * 80.0f; // angle of exit based on ratio (min 0, max 80)
	if (theta > 80.0f) {
		theta = 80.0f;
	}
	theta = theta * (PI / 180.0f); // convert to radians

	if (currentY < 0) { // flip angle to reflect y
		theta *= -1.0f;
	}

	float newX = cos(theta) * currentMagnitude; // determine new x and y components with accelerated magnitude
	float newY = sin(theta) * currentMagnitude;

	if (currentX > 0) { // flip X (newX will always come out positive)
		newX *= -1;
	}

	this->velocity = Vector2f(newX, newY);
}

void Ball::update(float dt) {
	this->position.x += this->velocity.x * dt;
	this->position.y += this->velocity.y * dt;

	// kooky colors
	if (this->colorCycleCount > 3) {
		this->colorCycleCount = 0;
		this->shape.setFillColor(Color(rand(), rand(), rand()));
	}
	else {
		this->colorCycleCount++;
	}

	// check x bounds
	if (this->position.x > WINDOW_WIDTH) {
		this->position.x = WINDOW_WIDTH / 2;
		this->position.y = WINDOW_HEIGHT / 2;
		this->randomizeStartVelocity();	
		this->offScreen = 1;

	}
	else if (this->position.x + 2 * this->radius < 0) {
		this->position.x = WINDOW_WIDTH / 2;
		this->position.y = WINDOW_HEIGHT / 2;
		this->randomizeStartVelocity();
		this->offScreen = -1;
	}

	// check y bounds
	if (this->position.y > WINDOW_HEIGHT - 2 * this->radius) {
		this->position.y = WINDOW_HEIGHT - 2 * this->radius;
		this->velocity.y *= -1;
	}
	else if (this->position.y < 0) {
		this->position.y = 0;
		this->velocity.y *= -1;
	}

	this->offScreen = 0;
}

void Ball::draw(RenderWindow* window) {
	this->shape.setPosition(this->position);
	window->draw(this->shape);
}

Vector2f Ball::getPosition() {
	return Vector2f(this->position.x + this->radius, this->position.y + this->radius);
}

float Ball::getRadius() {
	return this->radius;
}

bool collisionLine(Vector2f bp, Vector2f pp) { // TODO: use line collision option?
	return true;
}

bool collisionCircle(Vector2f b1p, float b1r, Vector2f b2p, float b2r) {
	//compute distance between centers
	float dist = sqrt( (b1p.x - b2p.x) * (b1p.x - b2p.x) + (b1p.y - b2p.y) * (b1p.y - b2p.y) );
	float radiiSum = b1r + b2r;
	if (dist <= radiiSum) {
		return true;
	}
	return false;
}

bool collisionRectangle(Ball *ball, Paddle *paddle) { // checks if a ball and paddle collided
	Vector2f bp = ball->getPosition();
	float br = ball->getRadius();
	Vector2f pp = paddle->getPosition();
	Vector2f ps = paddle->getSize();

	float testX = bp.x;
	float testY = bp.y;
	if (bp.x < pp.x) {
		// ball left from left edge
		testX = pp.x;
	}
	else if (bp.x > pp.x + ps.x) {
		// ball right of right edge
		testX = pp.x + ps.x;
	}
	// else ball within x range

	if (bp.y < pp.y) {
		// ball up from top edge
		testY = pp.x;
	}
	else if (bp.y > pp.y + ps.y) {
		// ball down from bottom edge
		testY = pp.x + ps.x;
	}
	// else ball within y range

	// calculate pythagorean distance from ball to closest edge
	float pythagDist = sqrt((bp.x - testX) * (bp.x - testX) + (bp.y - testY) * (bp.y - testY));
	
	if (pythagDist <= br) { // pythag collision
		return true;
	}
	else {
		return false;
	}
}


int main()
{
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pong"); // create window
	window.setVerticalSyncEnabled(true); // vsync bc why not
	window.setKeyRepeatEnabled(false); // remove repeated key events

	// set up music and sfx
	sf::SoundBuffer sfx_impact_buffer;
	if (!sfx_impact_buffer.loadFromFile("impact.wav")) {
		exit(-1);
	}
	sf::Sound sfx_impact;
	sfx_impact.setBuffer(sfx_impact_buffer);

	Music music;
	if (!music.openFromFile("pongdraft02.wav")) {
		exit(-1);
	}
	music.setLoop(true);
	music.play();

	Clock clock; // init clock
	float dt_ms = 0;

	// key booleans
	bool upKeyPressed = false;
	bool downKeyPressed = false;
	bool wKeyPressed = false;
	bool sKeyPressed = false;
	
	// keep track of how many balls on screen
	int ball1OffScreen = 0;
	int ball2OffScreen = 0;
	int ball3OffScreen = 0;
	int activeBalls = 0;

	// menu setup
	sf::Font fontLoader;
	if (!fontLoader.loadFromFile("arial.ttf"))
	{
		std::exit(-1);
	}
	Text menuText;
	menuText.setFont(fontLoader);
	menuText.setString("[1] Play vs A.I.\n[2] Play vs Human\n[3] Demo mode\n[4] Exit");
	menuText.setFillColor(Color::White);
	menuText.setPosition(Vector2f(WINDOW_WIDTH / 2.0f - 100.0f, WINDOW_HEIGHT / 2.0f - 80.0f));

	// board setup
	sf::Texture bg_texture;
	if (!bg_texture.loadFromFile("spacebg2.png"))
	{
		exit(-1);
	}
	Sprite background = Sprite(bg_texture);
	background.setPosition(0.0f, 0.0f);
	RectangleShape midLine(Vector2f(5.0f, WINDOW_HEIGHT));
	midLine.setPosition(Vector2f(WINDOW_WIDTH / 2 - 2.5, 0));
	midLine.setFillColor(Color(255, 255, 255, 255));

	// initialize game objects
	Scoreboard scoreboard(Vector2f(WINDOW_WIDTH / 2, 20.0f));
	Ball ball1(Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));
	Ball ball2(Vector2f(-1.0f, -1.0f), Vector2f(0.0f, 0.0f));
	Ball ball3(Vector2f(-1.0f, -1.0f), Vector2f(0.0f, 0.0f));

	Ball balls[3] = {ball1, ball2, ball3};
	activeBalls = 1; // one ball active at start of game

	Paddle paddleRight(Vector2f(WINDOW_WIDTH - 15.0f, WINDOW_HEIGHT / 2.0f - 35.0f)); // place paddles
	Paddle paddleLeft(Vector2f(15.0, WINDOW_HEIGHT / 2.0f - 35.0f)); 
	paddleLeft.setAi(true); // TEMP ai trigger
	paddleRight.setAi(true);

	PowerUp pu1(0, Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.8f)); // place powerups
	PowerUp pu2(0, Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.2f));
	
	while (window.isOpen()) // overall game loop
	{
		// Timing
		dt_ms = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				window.close();
			}
			else if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Up) {
					upKeyPressed = true;
				}
				else if (event.key.code == Keyboard::Down) {
					downKeyPressed = true;
				}
				else if (event.key.code == Keyboard::W) {
					wKeyPressed = true;
				}
				else if (event.key.code == Keyboard::S) {
					sKeyPressed = true;
				}
			}
			else if (event.type == Event::KeyReleased) {
				if (event.key.code == Keyboard::Up) {
					upKeyPressed = false;
				}
				else if (event.key.code == Keyboard::Down) {
					downKeyPressed = false;
				} 
				else if (event.key.code == Keyboard::W) {
					wKeyPressed = false;
				}
				else if (event.key.code == Keyboard::S) {
					sKeyPressed = false;
				}
			}
		}

		for (int i = 0; i < activeBalls; i++) { // update the movement of the balls
			balls[i].update(dt_ms);
		}
	
		// update movements of the paddles
		paddleRight.updateDelegator(dt_ms, downKeyPressed, upKeyPressed, balls[0].getPosition()); // player controls with (up) (down)
		paddleLeft.updateDelegator(dt_ms, sKeyPressed, wKeyPressed, balls[0].getPosition()); // player controls with (w) (s)

		// collision check for balls
		for (int i = 0; i < activeBalls; i++) {
			if (collisionRectangle(&balls[i], &paddleRight)) {
				balls[i].bounce(paddleRight);
				balls[i].setPosition(Vector2f(paddleRight.getPosition().x - 2 * balls[i].getRadius() - 1.0f, balls[i].getPosition().y));
				sfx_impact.play();
			}
			else if (collisionRectangle(&balls[i], &paddleLeft)) {
				balls[i].bounce(paddleLeft);
				balls[i].setPosition(Vector2f(paddleLeft.getPosition().x + paddleLeft.getSize().x + 1.0f, balls[i].getPosition().y));
				sfx_impact.play();
			}
		}

		//scoring
		for (int i = 0; i < activeBalls; i++) {
			if (balls[i].isOffScreen() < 0) {
				scoreboard.update(0, 1);
			}
			else if (balls[i].isOffScreen() > 0) {
				scoreboard.update(1, 0);
			}
		}
		
		// draw board functions
		window.clear(Color(0, 0, 0, 255)); // clear to black
		window.draw(background); // draw static board objects
		window.draw(midLine);
		// window.draw(menuText);

		// draw updated game objects 
		scoreboard.draw(&window); 
		for (int i = 0; i < activeBalls; i++) {
			balls[i].draw(&window);
		}
		paddleRight.draw(&window);
		paddleLeft.draw(&window);

		pu1.draw(&window);
		pu2.draw(&window);
		
		window.display(); // show the new screen
	}

	music.stop(); // cut music on exit
	return 0;
}
