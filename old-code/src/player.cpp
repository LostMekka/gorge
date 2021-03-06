#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <forward_list>

#include "helper.hpp"
#include "object.hpp"
#include "walls.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "badguy.hpp"


using namespace std;




Laser::Laser(Vec2 pos) {
	init("media/laser.png");
	setPosition(pos);
	vel = Vec2(0, -16);
	alive = true;
	playSound("media/laser.wav", pos);
}


bool Laser::update() {
	move(vel);
	updateCollisionPoly();

	float distance = walls.checkCollision(poly);
	if (distance > 0) {
		makeParticle<Hit>(getPosition());
		return false;
	}

	if (getPosition().y < -16) return false;
	return alive;
}


const Poly& Laser::getCollisionModel() const {
	static const Poly model = {
		Vec2(1, 5),
		Vec2(1, -3),
		Vec2(-1, -3),
		Vec2(-1, 5),
	};
	return model;
}






const Poly& Player::getCollisionModel() const {
	static const Poly model = {
		Vec2(4, 4),
		Vec2(4, 0),
		Vec2(1, -3),
		Vec2(-1, -3),
		Vec2(-4, 0),
		Vec2(-4, 4),
	};
	return model;
}


void Player::init() {
	Object::init("media/ship.png");
	setPosition(400, 500);
	score = 0;
	tick = 0;
}



void Player::takeHit() {
	printf("%d\n", score);
//	window.close();
}



bool Player::checkCollisionWithBullets() {
	Vec2 normal;
	for (auto& bullet : bullets) {
		float distance = checkCollision(*bullet, &normal);
		if (distance > 0) {
			makeParticle<Hit>(bullet->getPosition());
			bullet->die();
			makeParticle<Explosion>(getPosition());
			move(normal * (distance + 5.0f));
			takeHit();
			return false;
		}
	}
	return true;
}


bool Player::checkCollisionWithBadGuys() {
	Vec2 normal, pos;
	for (auto& guy : badGuys) {
		float distance = checkCollision(*guy, &normal, &pos);
		if (distance > 0) {
			makeParticle<Hit>(pos);
			guy->takeHit(1);
			makeParticle<Explosion>(getPosition());
			move(normal * (distance + 15.0f));
			takeHit();
			return false;
		}
	}
	return true;
}



bool Player::update() {

	float joy_x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
	float joy_y = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
	Vec2 mov;
	mov.x = (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) or joy_x > 50) -
			(sf::Keyboard::isKeyPressed(sf::Keyboard::Left) or joy_x < -50);
	mov.y = (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) or joy_y > 50) -
			(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) or joy_y < -50);
	Vec2 pos = getPosition();
	pos += mov * 3.0f;
	pos.x = std::min(pos.x, 784.0f);
	pos.x = std::max(pos.x, 16.0f);
	pos.y = std::min(pos.y, 584.0f);
	pos.y = std::max(pos.y, 16.0f);
	setPosition(pos);
	updateCollisionPoly();
	Vec2 normal;
	float distance = walls.checkCollision(poly, &normal);
	if (distance > 0) {
		makeParticle<Explosion>(pos);
		move(normal * -(distance + 25.0f));
		pos = getPosition();
		takeHit();
	}


	bool shoot = sf::Joystick::isButtonPressed(0, 0) |
				sf::Keyboard::isKeyPressed(sf::Keyboard::X);

	if (shoot) {
		if (shootDelay == 0) {
			lasers.emplace_front(std::unique_ptr<Laser>(new Laser(pos + Vec2(0, -10))));
			shootDelay = 15;
		}
		else shootDelay--;
	}
	else if (shootDelay > 0) shootDelay--;


	tick++;
	tick %= 8;
	setFrame(tick > 3);

	checkCollisionWithBullets();
	checkCollisionWithBadGuys();

	return true;
}
