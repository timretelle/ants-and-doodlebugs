#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
using namespace std;


const int X_SIZE = 20;
const int Y_SIZE = 20;
const float SQUARE_PIXELS = 20.0;

class Index_out_of_bounds {};

enum Tile_type { EMPTY, GREEN_BLOB, RED_BLOB, NUMBER_OF_TILES };
enum Direction { UP, DOWN, LEFT, RIGHT, NUMBER_OF_DIRECTIONS };
class World;

class Tile
{
public:
	Tile() : x(0), y(0), shape() {  }
	Tile(int x = 0, int y = 0, float radius = SQUARE_PIXELS, size_t point_count = 30) : x(x), y(y), shape(radius, point_count) { }
	virtual void display(sf::RenderWindow& window) = 0;
	static void tile_swap(Tile*& pTile1, Tile*& pTile2);
	virtual Tile_type who() = 0;
	virtual void turn(World& w) {};
protected:
	int x; 
	int y;
	sf::CircleShape shape;
	void fix_shape_position();
};

class Blob : public Tile
{
public:
	Blob(int x = 0, int y = 0) : Tile(x, y, (SQUARE_PIXELS - 2) / 2, 30)
	{
		shape.setPosition(sf::Vector2f(x * SQUARE_PIXELS, y * SQUARE_PIXELS));
	}
	void display(sf::RenderWindow& window) = 0;
protected:
	bool breeding;//whether or not the blob is breeding on this iteration
	int steps_alive = 0;
};

class Green_blob : public Blob
{
public:
	Green_blob(int x = 0, int y = 0) : Blob(x, y)
	{
		shape.setFillColor(sf::Color::Green);
	}
	void display(sf::RenderWindow& window)
	{
		window.draw(shape);
	}
	virtual Tile_type who() {return GREEN_BLOB;}
	virtual void turn(World& w);
};

class Red_blob : public Blob
{
public:
	Red_blob(int x = 0, int y = 0) : Blob(x, y)
	{
		shape.setFillColor(sf::Color::Red);
	}
	void display(sf::RenderWindow& window)
	{
		window.draw(shape);
	}
	virtual Tile_type who() { return RED_BLOB; }
	virtual void turn(World& w);
private:
	bool ate;
	int steps_since_food = 0;
};

class Empty : public Tile
{
public:
	//Empty() : Tile(0, 0) { cout << "Calling default Empty constructor." << endl; }
	Empty(int x = 0, int y = 0) : Tile(x, y, (SQUARE_PIXELS - 2)/2, 4) 
	{ 
		shape.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
		//tell the shape where it lives START HERE
		shape.setPosition(sf::Vector2f(x * SQUARE_PIXELS, y * SQUARE_PIXELS));
	}
	void display(sf::RenderWindow& window)
	{
		//window.draw(shape);
	}
	virtual Tile_type who() { return EMPTY; }
	
};

class World
{
public:
	World(int x = X_SIZE, int y = Y_SIZE, int green_blobs = 100, int red_blobs = 5);
	~World();
	void display(sf::RenderWindow& window);
	void run_simulation();
	void turn();
	vector<Tile*>& operator[](int index);
	const vector<Tile*>& operator[](int index) const;

private: 
	vector<vector<Tile*>> grid;
};


int main()
{	
	srand(time(0));
	World world;
	world.run_simulation();
	
}



World::World(int x, int y, int green_blobs, int red_blobs)
{
	vector<Tile*> column(y);
	for (int i = 0; i < x; i++)
	{
		grid.push_back(column);
	}

	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			if  (green_blobs > 0)
			{
				grid[i][j] = new Green_blob(i, j);
				green_blobs--;
			}
			else if(red_blobs > 0)
			{
				grid[i][j] = new Red_blob(i, j);
				red_blobs--;
			}
			else
			{
				grid[i][j] = new Empty(i, j);
			}
		}
	}

	int x1, x2;
	int y1, y2;
	for (int i = 0; i < 100000; i++)
	{
		x1 = rand() % x;
		x2 = rand() % x;
		y1 = rand() % y;
		y2 = rand() % y;
		Tile::tile_swap(grid[x1][y1], grid[x2][y2]);
	}
}

World::~World()
{
	for (int i = 0; i < grid.size(); i++)
	{
		for (int j = 0; j < grid[i].size(); j++)
		{
			delete grid[i][j];
		}
	}
}

void World::display(sf::RenderWindow& window)
{
	for (int i = 0; i < grid.size(); i++)
	{
		for (int j = 0; j < grid[i].size(); j++)
		{
			grid[i][j]->display(window);
		}
	}
}

void World::turn()
{
	vector<Tile*> red_blobs;
	for (int i = 0; i < grid.size(); i++)
	{
		for (int j = 0; j < grid[i].size(); j++)
		{
			if (grid[i][j]->who() == RED_BLOB)
			{
				red_blobs.push_back(grid[i][j]);
			}
		}
	}

	for (auto red_blob : red_blobs)
	{
		red_blob->turn(*this);
	}

	vector<Tile*> green_blobs;
	for (int i = 0; i < grid.size(); i++)
	{
		for (int j = 0; j < grid[i].size(); j++)
		{
			if (grid[i][j]->who() == GREEN_BLOB)
			{
				green_blobs.push_back(grid[i][j]);
			}
		}
	}

	for (auto green_blob : green_blobs)
	{
		green_blob->turn(*this);
	}


}

void World::run_simulation()
{
	sf::RenderWindow window(sf::VideoMode(400, 400), "World of blobs!");
	int count = 0;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			//cout << event.type << endl;
			if (event.type == sf::Event::KeyPressed){//do a time step whenever enter is pressed
				if(event.key.code == sf::Keyboard::Enter){
					turn();			
					count = 0;
				}
			}
		}
		sf::sleep(sf::milliseconds(50));//make sure it doesn't iterate multiple times on one press unless they hold it down intentionally
		window.clear();
		count++;
		
		display(window);
		//shape.move(sf::Vector2f(rand()%3 -1, rand()%3 - 1));
		//shape.rotate(1);
		//window.draw(shape);		
		window.display();

	}

}

void Tile::tile_swap(Tile*& pTile1, Tile*& pTile2)
{
	swap(pTile1->x, pTile2->x);
	swap(pTile1->y, pTile2->y);
	swap(pTile1, pTile2);
	pTile1->fix_shape_position();
	pTile2->fix_shape_position();
}

void Tile::fix_shape_position()
{
	shape.setPosition(sf::Vector2f(x * SQUARE_PIXELS, y * SQUARE_PIXELS));
}

void Red_blob::turn(World& w)
{
	ate = false;
	breeding = false;
	steps_alive++;
	steps_since_food++;
	if(steps_since_food == 4){
		w[x][y] = new Empty(x, y);//empty ourself
		delete this;
		return;
	}
	breeding = (steps_alive % 8 == 0);
	if(y > 0){//make sure we don't check above us if we are on the top
		if(w[x][y-1]->who() == GREEN_BLOB){
			ate = true;
			w[x][y-1] = new Empty(x, y-1);//replace the ant with an empty tile
			Tile::tile_swap(w[x][y], w[x][y-1]);//move where the ant was
			steps_since_food = 0;
			//delete w[x][y-1];//kill the ant
		}
	}
	if(y < Y_SIZE-1){//make sure we don't check below us if we are on the bottom
		if(w[x][y+1]->who() == GREEN_BLOB){
			ate = true;
			w[x][y+1] = new Empty(x, y+1);//replace the ant with an empty tile
			Tile::tile_swap(w[x][y], w[x][y+1]);//move where the ant was
			steps_since_food = 0;
			//delete w[x][y-1];//kill the ant
		}
	}
	if(x > 0){//make sure we don't check to the left if we are against the left edge
		if(w[x-1][y]->who() == GREEN_BLOB){
			ate = true;
			w[x-1][y] = new Empty(x-1, y);//replace the ant with an empty tile
			Tile::tile_swap(w[x][y], w[x-1][y]);//move where the ant was
			steps_since_food = 0;
			//delete w[x][y-1];//kill the ant
		}
	}
	if(x < X_SIZE-1){//make sure we don't check to the right if we are against the right edge
		if(w[x+1][y]->who() == GREEN_BLOB){
			ate = true;
			w[x+1][y] = new Empty(x+1, y);//replace the ant with an empty tile
			Tile::tile_swap(w[x][y], w[x+1][y]);//move where the ant was
			steps_since_food = 0;
			//delete w[x][y-1];//kill the ant
		}
	}
	//only move randomly if we didn't find any food
	switch (rand() % NUMBER_OF_DIRECTIONS)
	{
	case UP:
		if (y > 0)
		{
			if (w[x][y - 1]->who() == EMPTY)
			{
				if(!breeding && !ate)
					Tile::tile_swap(w[x][y], w[x][y-1]);
				else if(breeding)
					w[x][y - 1] = new Red_blob(x, y-1);
			}
		}
		break;
	case DOWN:
		if (y < Y_SIZE-1)
		{
			if (w[x][y + 1]->who() == EMPTY)
			{
				if(!breeding && !ate)
					Tile::tile_swap(w[x][y], w[x][y+1]);
				else if(breeding)
					w[x][y+1] = new Red_blob(x, y+1);
			}
		}
		break;
	case LEFT:
		if (x > 0)
		{
			//go left
			if (w[x - 1][y]->who() == EMPTY)
			{
				if(!breeding && !ate)
					Tile::tile_swap(w[x][y], w[x-1][y]);
				else if(breeding)
					w[x-1][y] = new Red_blob(x-1, y);
			}
		}
		break;
	case RIGHT:
		if (x < X_SIZE-1)
		{
			if (w[x + 1][y]->who() == EMPTY)
			{
				if(!breeding && !ate)
					Tile::tile_swap(w[x][y], w[x+1][y]);
				else if(breeding)
					w[x+1][y] = new Red_blob(x+1, y);
			}
		}
		break;
	}
}
void Green_blob::turn(World& w)
{
	breeding = false;
	steps_alive++;
	breeding = (steps_alive % 3 == 0);
	switch (rand() % NUMBER_OF_DIRECTIONS)
	{
	case UP:
		if (y > 0)
		{
			if (w[x][y - 1]->who() == EMPTY)
			{
				if(!breeding)
					Tile::tile_swap(w[x][y], w[x][y-1]);
				else
					w[x][y - 1] = new Green_blob(x, y-1);
			}
		}
		break;
	case DOWN:
		if (y < Y_SIZE-1)
		{
			if (w[x][y + 1]->who() == EMPTY)
			{
				if(!breeding)
					Tile::tile_swap(w[x][y], w[x][y+1]);
				else
					w[x][y+1] = new Green_blob(x, y+1);
			}
		}
		break;
	case LEFT:
		if (x > 0)
		{
			//go left
			if (w[x - 1][y]->who() == EMPTY)
			{
				if(!breeding)
					Tile::tile_swap(w[x][y], w[x-1][y]);
				else
					w[x-1][y] = new Green_blob(x-1, y);
			}
		}
		break;
	case RIGHT:
		if (x < X_SIZE-1)
		{
			if (w[x + 1][y]->who() == EMPTY)
			{
				if(!breeding)
					Tile::tile_swap(w[x][y], w[x+1][y]);
				else
					w[x+1][y] = new Green_blob(x+1, y);
			}
		}
		break;
	}

}

vector<Tile*>& World::operator[](int index)
{
	if (index >= grid.size())
	{
		throw Index_out_of_bounds();
	}
	return grid[index];
}
const vector<Tile*>& World::operator[](int index) const
{
	if (index >= grid.size())
	{
		throw Index_out_of_bounds();
	}
	return grid[index];
}