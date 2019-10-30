#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

#include<vector>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 512;
const int WINDOW_HEIGHT = 512;

class Paddle {
public:
	Paddle();
	Paddle(Vector2f position, Vector2f velocity);
	void draw(RenderWindow* window);
	void update(float dt);
private:
	Vector2f velocity;
	Vector2f position;
};

class Ball {
public:
	Ball();
	Ball(Vector2f position, Vector2f velocity);
	void draw(RenderWindow* window);
	void update(float dt);
private:
	Vector2f velocity;
	Vector2f position;
	CircleShape shape;
	int radius;
};

Ball::Ball(Vector2f position, Vector2f velocity) {
	// set up shape
	this->radius = 10;
	this->shape = CircleShape(this->radius);
	this->shape.setFillColor(Color::Blue);

	// set up position and velocity
	this->position = position;
	this->velocity = velocity;
}

Ball::Ball() {
	Ball::Ball(Vector2f(0.0f, 0.0f), Vector2f(0.0f, 0.0f));
}

void Ball::update(float dt) {
	this->position.x += this->velocity.x * dt;
	this->position.y += this->velocity.y * dt;

	// check x bounds
	if (this->position.x > WINDOW_WIDTH - 2 * this->radius) {
		this->position.x = WINDOW_WIDTH - 2 * this->radius;
		this->velocity.x *= -1;
	} 
	else if (this->position.x < 0) {
		this->position.x = 0;
		this->velocity.x *= -1;
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
}

void Ball::draw(RenderWindow* window) {
	this->shape.setPosition(this->position);
	window->draw(this->shape);
}


int main()
{
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pong"); // create window
	window.setVerticalSyncEnabled(true); // vsync bc why not

	Clock clock; // init clock
	float dt_ms = 0;

	Ball ball(Vector2f(100.0f, 0.0f), Vector2f(1.0f, 1.0f)); // initialize game objects

	while (window.isOpen()) // overall game loop
	{
		// Timing
		dt_ms = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) // check if window was closer this cycle
				window.close();
		}

		// update functions
		ball.update(dt_ms); // update the movement of the ball

		// draw functions
		window.clear(); // clear to black (no epilepsy warnings)
		ball.draw(&window); // draw updated game objects
		window.display(); // draw the new screen
	}

	return 0;
}
