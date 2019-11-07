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
	void collect(bool state);
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

void PowerUp::collect(bool state) {
	this->collected = state;
}

bool PowerUp::isCollected() {
	return this->collected;
}

Vector2f PowerUp::getPosition() {
	return this->position;
}

float PowerUp::getRadius() {
	return this->radius;
}

void PowerUp::draw(RenderWindow* window) {
	// correct for top-left shape origin
	this->shape.setPosition(Vector2f(this->position.x - this->radius, this->position.y - this->radius));
	window->draw(this->shape);
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
	// constructor
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
	Ball(Vector2f position);
	Ball(Vector2f position, Vector2f velocity);
	void draw(RenderWindow* window);
	void update(float dt);
	void bounce(Paddle p);
	void bounceSimple();
	Vector2f getPosition();
	void setPosition(Vector2f newPosition);
	float getRadius();
	void setRadius(float newrad);
	void randomizeStartVelocity();
	Vector2f getVelocity();
	void setVelocity(Vector2f velocity);
	int isOffScreen();
	bool isActive();
	void setActive(bool state);
private:
	Vector2f velocity;
	float baseSpeed;
	Vector2f position;
	CircleShape shape;
	float radius;
	int colorCycleCount; // for flashing ball
	int offScreen;
	bool active;
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
	this->active = false;
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
	this->active = false;
}

bool Ball::isActive() {
	return this->active;
}

void Ball::setActive(bool state) {
	this->active = state;
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

	this->velocity = Vector2f(newX, newY);
}

void Ball::setPosition(Vector2f newPosition) {
	this->position = newPosition;
}

void Ball::setRadius(float newrad) {
	if (newrad >= 1) {
		this->radius = newrad;
	}
	this->shape.setRadius(this->radius);
}

void Ball::bounce(Paddle p) {
	// calculate current magnitude and accelerate
	float currentX = this->velocity.x; // current x and y components
	float currentY = this->velocity.y;
	float currentMagnitude = sqrt(currentX * currentX + currentY * currentY); // distance formula for magnitude of velocity
	currentMagnitude *= 1.1f; // 10% increase in speed each bounce

	// calculate new angle
	float midP = p.getPosition().y + p.getSize().y / 2.0f; // midpoint of the paddle (y)
	float spread = abs(midP - this->position.y); // distance from midpoint y to ball center y
	float ratio = spread / (p.getSize().y / 2.0f); // ratio of distance to total paddle height (between 0-1)
	float theta = ratio * 75.0f; // angle of exit based on ratio (min 0, max 80)
	if (theta > 75.0f) {
		theta = 75.0f;
	}
	theta = theta * (PI / 180.0f); // convert to radians
	if (currentY < 0) { // flip angle to reflect y
		theta *= -1.0f;
	}

	// calculate new x and y comps
	float newX = cos(theta) * currentMagnitude; // determine new x and y components with accelerated magnitude
	float newY = sin(theta) * currentMagnitude;
	if (currentX > 0) { // flip X (newX will always come out positive)
		newX *= -1;
	}

	// set new velocity
	this->velocity = Vector2f(newX, newY);
}

void Ball::bounceSimple() { // no angle change calcs
	this->velocity = Vector2f(-1.0f * this->velocity.x, this->velocity.y);
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
	if (this->position.x + this->radius > WINDOW_WIDTH) {
		this->offScreen = 1;
	}
	else if (this->position.x - this->radius < 0) {
		this->offScreen = -1;
	}
	else {
		this->offScreen = 0;
	}

	// check y bounds
	if (this->position.y + this->radius > WINDOW_HEIGHT) { // if below window
		this->position.y = WINDOW_HEIGHT - this->radius; // set to window bottom and flip
		this->velocity.y *= -1;
	}
	else if (this->position.y - this->radius < 0) { // if above window
		this->position.y = this->radius; // set to window top and flip
		this->velocity.y *= -1;
	}
}

void Ball::draw(RenderWindow* window) {
	// correct for SHAPE POSITION top-left origin
	this->shape.setPosition(Vector2f(this->position.x - this->radius, this->position.y - this->radius));
	window->draw(this->shape);
}

Vector2f Ball::getPosition() {
	return Vector2f(this->position.x, this->position.y);
}

float Ball::getRadius() {
	return this->radius;
}

bool collisionCircle(Vector2f b1p, float b1r, Vector2f b2p, float b2r) {
	// b1p and b2p are the CENTER positions of the two circles
	//compute distance between centers
	float dist = sqrt( (b1p.x - b2p.x) * (b1p.x - b2p.x) + (b1p.y - b2p.y) * (b1p.y - b2p.y) );
	float radiiSum = b1r + b2r;
	if (dist <= radiiSum) {
		return true;
	}
	return false;
}

bool collisionRectangle(Ball *ball, Paddle *paddle) { // checks if a ball and paddle collided
	// bp is the CENTER position of the circle, pp is the TOP LEFT of the paddle
	Vector2f bp = ball->getPosition();
	float br = ball->getRadius();
	Vector2f pp = paddle->getPosition();
	Vector2f ps = paddle->getSize();


	float testX = bp.x;
	float testY = bp.y;
	if (bp.x < pp.x) {
		// ball center left from left edge
		testX = pp.x;
	}
	else if (bp.x > pp.x + ps.x) {
		// ball center right of right edge
		testX = pp.x + ps.x;
	}
	// else ball center within x range

	if (bp.y < pp.y) {
		// ball center up from top edge
		testY = pp.y;
	}
	else if (bp.y > pp.y + ps.y) {
		// ball center down from bottom edge
		testY = pp.y + ps.y;
	}
	// else ball center within y range

	// calculate pythagorean distance from ball to closest edge
	float distX = bp.x - testX;
	float distY = bp.y - testY;
	float pythagDist = sqrt((distX * distX) + (distY * distY));
	
	if (pythagDist <= br) { // pythag collision
		return true;
	}
	else {
		return false;
	}
}


int main() {
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pong"); // create window
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false); // remove repeated key events

	// set up music and sfx
	sf::SoundBuffer sfx_impact_buffer;
	if (!sfx_impact_buffer.loadFromFile("impact.wav")) {
		exit(-1);
	}
	sf::Sound sfx_impact;
	sfx_impact.setBuffer(sfx_impact_buffer);

	sf::SoundBuffer sfx_powerup_buffer;
	if (!sfx_powerup_buffer.loadFromFile("powerup.wav")) {
		exit(-1);
	}
	sf::Sound sfx_powerup;
	sfx_powerup.setBuffer(sfx_powerup_buffer);

	Music music;
	if (!music.openFromFile("pongdraft02.wav")) {
		exit(-1);
	}
	music.setLoop(true);
	music.play();

	Clock clock; // init clock
	float dt_ms = 0;

	// game and menu booleans
	bool gameOver = false;
	bool menuChosen = false;

	// key booleans
	bool upKeyPressed = false;
	bool downKeyPressed = false;
	bool wKeyPressed = false;
	bool sKeyPressed = false;

	// menu setup
	sf::Font spacefontloader;
	if (!spacefontloader.loadFromFile("spacefont.otf"))
	{
		std::exit(-1);
	}
	sf::Font fontLoader;
	if (!fontLoader.loadFromFile("arial.ttf"))
	{
		std::exit(-1);
	}
	
	Text titleText;
	titleText.setFont(spacefontloader);
	titleText.setString("SPACE PONG");
	titleText.setFillColor(Color::White);
	titleText.setCharacterSize(60);
	titleText.setPosition(Vector2f(WINDOW_WIDTH / 2.0f - 275.0f, WINDOW_HEIGHT / 2.0f - 200.0f));
	
	Text titleTextShadow = titleText;
	titleTextShadow.setFillColor(Color::Red);
	titleTextShadow.setPosition(Vector2f(WINDOW_WIDTH / 2.0f - 275.0f - 3.0f, WINDOW_HEIGHT / 2.0f - 200.0f - 3.0f));
	
	Text menuText;
	menuText.setFont(fontLoader);
	menuText.setString("1  Play vs AI\n2  Play vs Human\n3  Demo mode\n4  Exit");
	menuText.setFillColor(Color::White);
	menuText.setPosition(Vector2f(WINDOW_WIDTH / 2.0f - 100.0f, WINDOW_HEIGHT / 2.0f - 80.0f));

	Text menuTextShadow = menuText;
	menuTextShadow.setFillColor(Color::Red);
	menuTextShadow.setPosition(Vector2f(WINDOW_WIDTH / 2.0f - 100.0f - 2.0f, WINDOW_HEIGHT / 2.0f - 80.0f - 2.0f));

	
	sf::Texture bg2_texture;
	if (!bg2_texture.loadFromFile("spacebg.png"))
	{
		exit(-1);
	}
	Sprite backgroundMenu = Sprite(bg2_texture);
	backgroundMenu.setPosition(0.0f, 0.0f);

	// board setup
	sf::Texture bg_texture;
	if (!bg_texture.loadFromFile("spacebg2.png"))
	{
		exit(-1);
	}
	Sprite backgroundGame = Sprite(bg_texture);
	backgroundGame.setPosition(0.0f, 0.0f);
	RectangleShape midLine(Vector2f(5.0f, WINDOW_HEIGHT));
	midLine.setPosition(Vector2f(WINDOW_WIDTH / 2 - 2.5, 0));
	midLine.setFillColor(Color(255, 255, 255, 255));

	// initialize game objects
	Scoreboard scoreboard(Vector2f(WINDOW_WIDTH / 2, 20.0f));
	
	Ball ball1(Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f)); // primary ball
	ball1.setActive(true);

	Ball ball2(Vector2f(-100.0f, 0), Vector2f(0.0f, 0.0f)); // extra balls created by powerups
	Ball ball3(Vector2f(-100.0f, 0), Vector2f(0.0f, 0.0f));	// set pos and velocity to keep out of way

	Ball balls[3] = {ball1, ball2, ball3}; // add all to array

	Paddle paddleRight(Vector2f(WINDOW_WIDTH - 15.0f, WINDOW_HEIGHT / 2.0f - 35.0f)); // set up left and right paddles
	Paddle paddleLeft(Vector2f(15.0, WINDOW_HEIGHT / 2.0f - 35.0f));				  // start in middle
	
	paddleLeft.setAi(true); // TODO: make the AI trigger an option
	//paddleRight.setAi(true);

	PowerUp pu1(0, Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.8f)); // place powerups
	PowerUp pu2(0, Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.2f));
	
	while (window.isOpen()) // GAME LOOP
	{
		// frame timing for velocity and position modifications
		dt_ms = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		
		// keep track of keyboard and click events
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) {
				music.stop(); // cut music on exit
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
		
		if (menuChosen) {
			// update movements of the paddles
			paddleRight.updateDelegator(dt_ms, downKeyPressed, upKeyPressed, balls[0].getPosition()); // player controls with (up) (down)
			paddleLeft.updateDelegator(dt_ms, sKeyPressed, wKeyPressed, balls[0].getPosition()); // player controls with (w) (s)

			//update ball behavior for each ball in array
			float ballsOnScreen = 0;
			for (int i = 0; i < 3; i++) {
				balls[i].update(dt_ms); // upate ball position (will set offscreen if offscreen)

				// check paddle collisions
				if (collisionRectangle(&balls[i], &paddleRight) && balls[i].isActive()) {
					balls[i].bounce(paddleRight);
					balls[i].setPosition(Vector2f(paddleRight.getPosition().x - balls[i].getRadius() - 1.0f, balls[i].getPosition().y));
					sfx_impact.play();
				}
				else if (collisionRectangle(&balls[i], &paddleLeft) && balls[i].isActive()) {
					balls[i].bounce(paddleLeft);
					balls[i].setPosition(Vector2f(paddleLeft.getPosition().x + paddleLeft.getSize().x +
						balls[i].getRadius() + 1.0f, balls[i].getPosition().y));
					sfx_impact.play();
				}

				// check if ball hit powerup
				if (!pu1.isCollected()) { // if pu1 still active
					if (collisionCircle(balls[i].getPosition(), balls[i].getRadius(), pu1.getPosition(), pu1.getRadius())) {
						pu1.collect(true); // remove on collision, create multiball
						balls[1].setActive(true);
						balls[1].setPosition(balls[i].getPosition());
						balls[1].setVelocity(Vector2f(balls[i].getVelocity().x, -1.0f * balls[i].getVelocity().y));
						sfx_powerup.play();
					}
				}
				if (!pu2.isCollected()) { // if pu2 active
					if (collisionCircle(balls[i].getPosition(), balls[i].getRadius(), pu2.getPosition(), pu2.getRadius())) {
						pu2.collect(true); // remove on collision, create multiball
						balls[2].setActive(true);
						balls[2].setPosition(balls[i].getPosition());
						balls[2].setVelocity(Vector2f(balls[i].getVelocity().x, -1.0f * balls[i].getVelocity().y));
						sfx_powerup.play();
					}
				}

				// keep track of how many balls on screen, scores
				if (balls[i].isOffScreen() != 0 && balls[i].isActive()) { // if active ball off screen
					balls[i].setActive(false);
					balls[i].setVelocity(Vector2f(0.0f, 0.0f));
					if (balls[i].isOffScreen() < 0) {
						// off the left side
						scoreboard.update(0, 1);
					}
					else { // off right side
						scoreboard.update(1, 0);
					}
				}
				else if (balls[i].isOffScreen() == 0 && balls[i].isActive()) { // active ball on screen
					ballsOnScreen++;
				}
			}

			// if no balls on screen, move main ball to center and start it
			if (ballsOnScreen == 0) {
				balls[0].setPosition(Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));
				balls[0].randomizeStartVelocity();
				balls[0].setActive(true);
				pu1.collect(false);
				pu2.collect(false);
			}

			// clear to black
			window.clear(Color(0, 0, 0, 255));
			// draw static board objects
			window.draw(backgroundGame);
			window.draw(midLine);

			// draw updated game objects 
			scoreboard.draw(&window);
			paddleRight.draw(&window);
			paddleLeft.draw(&window);

			// draw all currently active balls
			for (int i = 0; i < 3; i++) {
				if (balls[i].isActive()) {
					balls[i].draw(&window);
				}
			}

			// draw all uncollected powerups
			if (!pu1.isCollected()) {
				pu1.draw(&window);
			}
			if (!pu2.isCollected()) {
				pu2.draw(&window);
			}
		}
		else {
			window.clear(Color(0, 0, 0, 255));
			window.draw(backgroundMenu);
			window.draw(midLine);
			window.draw(titleTextShadow);
			window.draw(titleText);
			window.draw(menuTextShadow);
			window.draw(menuText);
		}

		window.display();
	}
	return 0;
}
