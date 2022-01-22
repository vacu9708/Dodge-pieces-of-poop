#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <list>
#include <windows.h> // for Sleep();
#include <thread>
#include <random>
#include <string>

using namespace std;
using namespace sf;

double random_real_number(double min, double max) {
	random_device seed; // Generate a random seed
	mt19937_64 mersenne(seed());
	uniform_real_distribution<> range(min, max);
	return range(mersenne);
}


int score = 1, max_score = 1;
class A_piece_of_poop {
	Texture poop_texture;
	Texture squashed_poop;
	SoundBuffer soundbuffer;
	Sound squashing_sound;
public:
	bool falling = false;
	Sprite poop_sprite;
	A_piece_of_poop() {
		soundbuffer.loadFromFile("resources/squashing sound.wav");
		poop_texture.loadFromFile("resources/poop.png");
		squashed_poop.loadFromFile("resources/squashed poop.png");
		poop_sprite.setTexture(poop_texture);
		poop_sprite.scale(0.15f, 0.15f);
	}

	void form_poop() {
		poop_sprite.setTexture(poop_texture);
	}

	void update_a_piece_of_poop(Sprite& player) {
		double coefficient = (double)score / (100 + (double)score);
		float speed = random_real_number(1, 1 + coefficient * 2);
		while (true) {
			if (score <= 0) {
				falling = false;
				return;
			}
			poop_sprite.move(0, speed);
			if (poop_sprite.getGlobalBounds().intersects(player.getGlobalBounds()) == true) { // If hit by poop
				score -= 100;
				break;
			}
			if (poop_sprite.getPosition().y > 800 - 50) { // If poop dodged
				score++;
				if (score > max_score)
					max_score = score;
				break;
			}
			Sleep(1);
		}
		poop_sprite.setTexture(squashed_poop);
		squashing_sound.setBuffer(soundbuffer);
		squashing_sound.play();
		while (squashing_sound.getStatus() == Sound::Playing);
		falling = false;
	}

	void draw_a_piece_a_poop(RenderWindow& window) {
		if (falling == true)
			window.draw(poop_sprite);
	}
};

class Poop_set {
	list<A_piece_of_poop> pieces_of_poop;
public:
	Poop_set(short how_many) {
		for (int i = 0; i < how_many; i++)
			pieces_of_poop.push_back(A_piece_of_poop());
	}

	//void initializer(int how_many) {
	//	for (A_piece_of_poop& i : pieces_of_poop) {
	//		for (int i = 0; i < how_many; i++)
	//			pieces_of_poop.push_back(A_piece_of_poop());
	//		//i.poop_sprite.setPosition((int)random_real_number(0, 500), 50);
	//		//i.falling = false;
	//	}
	//}

	void drop_poop(Sprite& player) {
		list<A_piece_of_poop>::iterator i = pieces_of_poop.begin();
		short falling_interval = 500;
		while (true) {
			if (i->falling == false) {
				i->poop_sprite.setPosition((int)random_real_number(0, 550), 50);
				i->form_poop();
				thread thread1(&A_piece_of_poop::update_a_piece_of_poop, i, ref(player));
				thread1.detach();
				i->falling = true;
			}

			Sleep(falling_interval);
			if (falling_interval > 150) // Decrease falling interval
				falling_interval -= 5;

			while (i->falling == true) { // Find a piece of poop not falling that can be dropped
				i++;
				if (i == pieces_of_poop.end())
					i = pieces_of_poop.begin();
			}
			if (score <= 0) {
				for (A_piece_of_poop& i : pieces_of_poop)
					i.falling = false;
				return;
			}
		}
	}

	void draw_pieces_of_poop(RenderWindow& window) {
		for (A_piece_of_poop& i : pieces_of_poop)
			if (i.falling == true)
				i.draw_a_piece_a_poop(window);
	}
};

void display_score_board(Text& scoreboard, RenderWindow& window) {
	scoreboard.setString("Score : " + to_string(score));
	window.draw(scoreboard);
}

bool left_key_pressed, right_key_pressed, jumping; // Without this, multiple threads are executed at the same time which leads to the acceleration of player
void move_left(Sprite& player) {
	while (Keyboard::isKeyPressed(Keyboard::Left) == true && player.getPosition().x > 0) {
		player.move(-1, 0);
		Sleep(1);
	}
	left_key_pressed = false;
}

void move_right(Sprite& player) {
	while (Keyboard::isKeyPressed(Keyboard::Right) == true && player.getPosition().x < 600 - 40) {
		player.move(1, 0);
		Sleep(1);
	}
	right_key_pressed = false;
}

void jump(Sprite& player) {
	while (Keyboard::isKeyPressed(Keyboard::Space) == true && player.getPosition().y > 500) {
		player.move(0, -1);
		Sleep(1);
	}
	while (player.getPosition().y < 800 - 80) {
		player.move(0, 1);
		Sleep(1);
	}
	jumping = false;
}

Event event;
inline void events(Sprite& player, RenderWindow& window) {
	while (window.pollEvent(event)) {
		switch (event.type) {
		case Event::Closed:
			window.clear();
			break;
		case Event::KeyPressed:
			if (Keyboard::isKeyPressed(Keyboard::Left) == true && left_key_pressed == false) {
				left_key_pressed = true;
				thread thread1(move_left, ref(player));
				thread1.detach();
			}
			else if (Keyboard::isKeyPressed(Keyboard::Right) == true && right_key_pressed == false) {
				right_key_pressed = true;
				thread thread1(move_right, ref(player));
				thread1.detach();
			}
			else if (Keyboard::isKeyPressed(Keyboard::Space) == true && jumping == false) {
				jumping = true;
				thread thread1(jump, ref(player));
				thread1.detach();
			}
			break;
		}
	}
}

int main() {
	RenderWindow window(VideoMode(600, 800), "Dodge poop"); // Window
	window.setFramerateLimit(60);
	window.clear(Color::Black);

	Font font; // Set text
	font.loadFromFile("resources/AGENCYR.ttf");
	Text scoreboard;
	scoreboard.setFont(font);

	scoreboard.setFillColor(Color::Green); // Use the scoreboard to display letters temporarily
	scoreboard.setCharacterSize(55);
	scoreboard.setPosition(115, 333);
	scoreboard.setString("Press Enter to start");
	window.draw(scoreboard);
	window.display();

	while (window.isOpen()) { // Wait until Enter is pressed to start
		if (window.pollEvent(event))
			if (Keyboard::isKeyPressed(Keyboard::Enter) == true)
				break;
	}

	scoreboard.setFillColor(Color::Black); // To display score
	scoreboard.setCharacterSize(45);
	scoreboard.setPosition(8, 0);
	scoreboard.setStyle(Text::Bold);

	Texture stickman_texture; // Player
	stickman_texture.loadFromFile("resources/stickman.png");
	Sprite player;
	player.setTexture(stickman_texture);
	player.setPosition(300 - 20, 800 - 80);
	player.scale(0.2f, 0.2f);

	RectangleShape sky(Vector2f(600, 100)); // Sky
	sky.setFillColor(Color::Cyan);

	//unique_ptr<Poop_set> poop_set(new Poop_set(33));
	//Poop_set* poop_set = new Poop_set(33);
	Poop_set poop_set(66);
	thread thread1(&Poop_set::drop_poop, &poop_set, ref(player));
	thread1.detach();

	while (window.isOpen()) {
		window.draw(player);
		window.draw(sky);
		display_score_board(scoreboard, window);
	
		poop_set.draw_pieces_of_poop(window);
		events(player, window);

		window.display();
		window.clear(Color::White);

		if (score <= 0) {
			scoreboard.setFillColor(Color::Red); // GAME OVER
			scoreboard.setCharacterSize(55);
			scoreboard.setPosition(0, 200);
			scoreboard.setString("                GAME OVER \n         Press Enter to retry\n         Max score : " + to_string(max_score));
			window.draw(scoreboard);

			window.display();
			window.clear(Color::Black);

			while (window.isOpen()) { // Wait until Enter is pressed to start
				if (window.pollEvent(event))
					if (Keyboard::isKeyPressed(Keyboard::Enter) == true) { // When Enter is pressed
						max_score = 1, score = 1;
						scoreboard.setFillColor(Color::Black);
						scoreboard.setCharacterSize(45);
						scoreboard.setPosition(8, 0);
						thread thread1(&Poop_set::drop_poop, &poop_set, ref(player));
						thread1.detach();
						break;
					}
			}
		}
	}
}
