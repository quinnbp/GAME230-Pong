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

class Scoreboard {
public:
	Scoreboard(Vector2f posLeft, Vector2f posRight);
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

Scoreboard::Scoreboard(Vector2f posLeft, Vector2f posRight) {
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
	this->leftScoreText.setPosition(posLeft);
	this->rightScoreText.setPosition(posRight);
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
	this->baseVelocity = 0.5f;

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
	// position of center of the paddle (y)

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
	Ball(Vector2f position, Vector2f velocity);
	void draw(RenderWindow* window);
	int update(float dt);
	void bounce();
	Vector2f getPosition();
	void setPosition(Vector2f newPosition);
	float getRadius();
	void setRadius(float newrad);
private:
	Vector2f velocity;
	Vector2f basevelocity;
	Vector2f position;
	CircleShape shape;
	float radius;
	int colorCycleCount; // for flashing ball
};

Ball::Ball(Vector2f position, Vector2f velocity) {
	// set up shape
	this->radius = 5;
	this->shape = CircleShape(this->radius);
	this->shape.setFillColor(Color::Blue);

	// set up position and velocity
	this->position = position;
	this->basevelocity = velocity;
	this->velocity = this->basevelocity;

	this->colorCycleCount = 10;
}

Ball::Ball() {
	Ball::Ball(Vector2f(0.0f, 0.0f), Vector2f(0.0f, 0.0f));
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

void Ball::bounce() {
	this->velocity.x *= -1.1f;
}

int Ball::update(float dt) {
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
		this->velocity = this->basevelocity;
		return 1;

	}
	else if (this->position.x + 2 * this->radius < 0) {
		this->position.x = WINDOW_WIDTH / 2;
		this->position.y = WINDOW_HEIGHT / 2;
		this->velocity = this->basevelocity;
		return -1;
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

	return 0;
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

bool collisionRectangle(Ball ball, Paddle paddle) { // checks if a ball and paddle collided
	Vector2f bp = ball.getPosition();
	float br = ball.getRadius();
	Vector2f pp = paddle.getPosition();
	Vector2f ps = paddle.getSize();

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

	// music and sfx
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
	int offScreen = 0;

	// board setup
	RectangleShape midLine(Vector2f(5.0f, WINDOW_HEIGHT));
	midLine.setPosition(Vector2f(WINDOW_WIDTH / 2, 0));
	midLine.setFillColor(Color(255, 255, 255, 255));

	// initialize game objects
	Scoreboard scoreboard(Vector2f(WINDOW_WIDTH / 2 - 100.0f, 20.0f), Vector2f(WINDOW_WIDTH / 2 + 100.0f, 20.0f));
	Ball ball(Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f), Vector2f(0.3f, 0.2f));
	Paddle paddleRight(Vector2f(WINDOW_WIDTH - 15.0f, WINDOW_HEIGHT / 2.0f));
	Paddle paddleLeft(Vector2f(15.0, WINDOW_HEIGHT / 2.0f));
	paddleLeft.setAi(true);
	
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
			}
			else if (event.type == Event::KeyReleased) {
				if (event.key.code == Keyboard::Up) {
					upKeyPressed = false;
				}
				else if (event.key.code == Keyboard::Down) {
					downKeyPressed = false;
				}
			}
		}

		// update functions
		if (collisionRectangle(ball, paddleRight)) {
			ball.bounce();
			ball.setPosition(Vector2f(paddleRight.getPosition().x - ball.getRadius() - 1.0f, ball.getPosition().y));
			sfx_impact.play();
		}
		if (collisionRectangle(ball, paddleLeft)) {
			ball.bounce();
			ball.setPosition(Vector2f(paddleLeft.getPosition().x + paddleLeft.getSize().x + ball.getRadius() + 1.0f, ball.getPosition().y));
			sfx_impact.play();	
		}

		offScreen = ball.update(dt_ms); // update the movement of the ball
		paddleRight.updateDelegator(dt_ms, downKeyPressed, upKeyPressed, ball.getPosition());
		paddleLeft.updateDelegator(dt_ms, downKeyPressed, upKeyPressed, ball.getPosition());
		
		// scoring
		if (offScreen < 0) {
			scoreboard.update(0, 1);
		}
		else if (offScreen > 0) {
			scoreboard.update(1, 0);
		}

		// draw functions
		window.clear(Color(0, 0, 0, 255)); // clear to black (no epilepsy warnings)
		window.draw(midLine);
		scoreboard.draw(&window);
		ball.draw(&window); // draw updated game objects
		paddleRight.draw(&window);
		paddleLeft.draw(&window);
		
		window.display(); // draw the new screen
	}

	music.stop();
	return 0;
}
