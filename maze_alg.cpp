#include <array>
#include <iostream>
#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>

#include <chrono>
#include <time.h>

int h = 20;
int w = 20;




/*
________

N = 1;
E = 2;
S = 4;
W = 8;
________

*/

auto rng = std::default_random_engine {};



std::random_device rd;
std::mt19937 g(rd());
 
// Helper functions
int dir_to_y(int dir){
	switch(dir){
		case 1: return -1;
		case 4: return 1;
	}
	return 0;
}

int dir_to_x(int dir){
	switch(dir){
		case 8: return -1;
		case 2: return 1;
	}
	return 0;
}

int opp(int dir){
	switch(dir){
		case 1: return 4;
		case 2: return 8;
		case 4: return 1;
		case 8: return 2;
	}
}

 
void carve_pass_from(int maze[20][20], int x, int y){
	// Shuffle array of directoins
	std::array<int, 4> directions = {1, 2 , 4, 8};
	std::shuffle(directions.begin(), directions.end(), rng);

	// for every direction
	for(size_t i = 0; i < 4; ++i){
		// set new coodrinates
		int ny = y + dir_to_y(directions[i]);
		int nx = x + dir_to_x(directions[i]);

		// if new coord lie in bounds and point at empty cell
		if(ny < h && nx < w && ny >= 0 && nx >= 0){
		
			if(maze[ny][nx] == 0){

				// bitwise OR to set corresponding bits maze[x][y] |= direction

				maze[y][x] |= directions[i];

				// maze[new_x][new_y] |= opposite of direction
				maze[ny][nx] |= opp(directions[i]);
				// recur
				carve_pass_from(maze, nx, ny);	
			}
		}
	}
}

int maze[20][20] = {0};
std::vector<sf::RectangleShape> paths;
std::array<int, 4> directions = {1, 2 , 4, 8};

std::map<int, sf::Vector2i> dist;

int carve_exit(int x, int y, int l){
	
	int length = l;
	// Visited
	maze[y][x] |= 32;
	
	for(int i = 0; i < directions.size(); ++i){
		if((maze[y][x] & directions[i])){
			int ny = y + dir_to_y(directions[i]);
			int nx = x + dir_to_x(directions[i]);
			if(ny < h && nx < w && ny >= 0 && nx >= 0){
				if(!(maze[ny][nx] & 32)){
					++length;
					carve_exit(nx, ny, length);			
				}
			}
		}
	}
	sf::Vector2i pos(y, x);
	dist[length] = pos;
	return length;
}

std::vector<sf::RectangleShape> cells;

void SetCells(){
	
	sf::RectangleShape cell(sf::Vector2f(20, 20));
	cell.setFillColor(sf::Color::White);
	cell.setOutlineColor(sf::Color::Black);
	cell.setOutlineThickness(2);
	
	for(int x = 0; x < w; ++x){
		for(int y = 0; y < h; ++y){
			cell.setPosition(x*20, y*20);
			cells.push_back(cell);
		}
	}
}

void RenderMaze(){
	
	float size = 34;
	
	sf::RectangleShape NS_path(sf::Vector2f(size , 2));
	NS_path.setFillColor(sf::Color::Black);
	sf::RectangleShape WE_path(sf::Vector2f(2, size ));
	WE_path.setFillColor(sf::Color::Black);
	
	for(int y = 0; y < 20; ++y){
		for(int x = 0; x < 20; ++x){
			if(!(maze[y][x] & 1)){
				NS_path.setPosition(x*size + 10 , y*size + 10 );
				paths.push_back(NS_path);
			}
			if(!(maze[y][x] & 2)){
				WE_path.setPosition(x*size + 10 + size, y*size + 10 );
				paths.push_back(WE_path);
			}
			if(!(maze[y][x] & 4)){
				NS_path.setPosition(x*size + 10 , y*size + size + 10);
				paths.push_back(NS_path);
			}
			if(!(maze[y][x] & 8)){
				WE_path.setPosition(x*size + 10, y*size + 10);
				paths.push_back(WE_path);
			}
		}
	}
}

sf::Vector2f vel(0, 0);

bool move_ch = true;

bool MoveChar(sf::RectangleShape& ch){
	bool no_col = false;
	sf::RectangleShape ch_copy = ch;
	ch_copy.move(vel);
	for(size_t i = 0; i < paths.size(); ++i){
		if(ch_copy.getGlobalBounds().intersects(paths[i].getGlobalBounds())){
			no_col = false;
		}
	}
	if(no_col){
		ch.move(vel);
		move_ch = false;
	}
}

int main(){
	sf::RenderWindow window (sf::VideoMode(700, 700), "Mazes!");
	window.setVerticalSyncEnabled(true);
	
	std::mt19937 mt;
	mt.seed(std::time(0));
	std::uniform_int_distribution<int> distrib(0, 20);

	bool move = false;
	carve_pass_from(maze, 0, 0);
	
	sf::RectangleShape ch(sf::Vector2f(10, 10));
	ch.setFillColor(sf::Color::Green);
	ch.setPosition(3, 3);
	RenderMaze();

	carve_exit(0, 0, 0);

	int maxl = -1;
	sf::Vector2i exit;
	for(auto it = dist.begin(); it != dist.end(); ++it){
		
		if(it->first >= maxl && it->second.x == 19 || it->second.y == 19){
			maxl = it->first;
			exit = it->second;
		}
	}
	std::cout << maxl << std::endl;
	std::cout << exit.x << " " << exit.y << std::endl;
	ch.setPosition(exit.x*34-5, exit.y*34+5);
	while(window.isOpen()){
		sf::Event event;
		window.clear(sf::Color::White);
		while(window.pollEvent(event)){
			if(event.type == sf::Event::Closed){
				window.close();
			}
		}
		if(move_ch){
			MoveChar(ch);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
			
			for(int y = 0; y < 20; ++y){
				for(int x = 0; x < 20; ++x){
					maze[y][x] = 0;
				}
			}
			
			paths.clear();
			int x_par = distrib(mt);
			int y_par = distrib(mt);

			carve_pass_from(maze, x_par, y_par);
			RenderMaze();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){move_ch = true; vel.y = -2; vel.x = 0;}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){move_ch = true; vel.y = 0; vel.x = -2;}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){move_ch = true; vel.y = 2; vel.x = 0;}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){move_ch = true; vel.y = 0; vel.x = 2;}


		for(size_t i = 0; i < paths.size(); ++i){
			
			window.draw(paths[i]);
		}
		window.draw(ch);
		window.display();
	}
}

